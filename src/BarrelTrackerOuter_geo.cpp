// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong

/** \addtogroup Trackers Trackers
 * \brief Type: **BarrelTrackerWithFrame**.
 * \author W. Armstrong
 *
 * \ingroup trackers
 *
 * @{
 */
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "DD4hep/Shapes.h"
#include "DDRec/DetectorData.h"
#include "DDRec/Surface.h"
#include "XML/Layering.h"
#include "XML/Utilities.h"
#include <array>
#include "DD4hepDetectorHelper.h"
#include <cmath>


#include "TVector3.h"
//#include "TGDMLParse.h"
#include "TGDMLParseBiggerFiles.h"
//#include "CADPlugins.h"
#include "FileLoaderHelper.h"

using namespace std;
using namespace dd4hep;
using namespace dd4hep::rec;
using namespace dd4hep::detail;

//typedef Object::Vertex_t Vertex;

/* THIS BIT IS WORK IN PROGRESS */
struct TriangularPrism // : public TessellatedSolid //Solid_type<TGeoTessellated> 
{
  
  TessellatedSolid::Vertex normal;
  TessellatedSolid::Vertex extrusionVector;
  
  TessellatedSolid::Vertex getNormal() {return normal;}
  TessellatedSolid::Vertex compute_normal(vector<TessellatedSolid::Vertex> vertices)
  {
    //figure out the castings here
    TessellatedSolid::Vertex vec1 = vertices.at(1) - vertices.at(2);
    TessellatedSolid::Vertex vec2 = vertices.at(0) - vertices.at(2);
    
    //take the cross product
    //note that cross is stored in the struct vertex so I needed to access it the static way
    TessellatedSolid::Vertex normal_return = TessellatedSolid::Vertex::Cross(vec1, vec2);
    if(!normal_return.IsNormalized()) {
      normal_return.Normalize();
    }
    return normal_return;
  }

  TessellatedSolid return_TriangularPrism(vector<TessellatedSolid::Vertex> vertices, double extrusion_length) {
    
    if(vertices.size() > 3) {
      printout(ERROR, "BarrelTrackerOuter", "Trying to construct triangular prism with more or less than 3 vertices");
      throw runtime_error("Triangular prisim construction failed.");
    }
    else {
      normal = compute_normal(vertices);
      extrusionVector = extrusion_length * normal;
      vector<TessellatedSolid::Vertex> extruded_vertices;
      for(auto& element : vertices) 
      {
        extruded_vertices.push_back(element + extrusionVector);
      }
      //vector<TessellatedSolid::Vertex> all_vertices(vertices.size() + extruded_vertices.size());
      //merge(vertices.begin(), vertices.end(), extruded_vertices.begin(), extruded_vertices.end(), 
          //all_vertices.begin()); 
      TessellatedSolid prism("prism", 6);
      //Now add the facets:
      //Top and bottom
      prism.addFacet(vertices.at(0), vertices.at(1), vertices.at(2));
      //note the reversal of order to keep the normals well
      prism.addFacet(extruded_vertices.at(2), extruded_vertices.at(1), extruded_vertices.at(0));

      //sides
      for(unsigned long i = 0; i < vertices.size(); i++) 
      {
        int next_i = (i + 1) % vertices.size();
        prism.addFacet(extruded_vertices.at(i), extruded_vertices.at(next_i), vertices.at(next_i), vertices.at(i)); 
      }
      //finally, correct the normals and close the mesh if not closed
      
      //this.CloseShape(true, true, true); //otherwise you get an infinite bounding box
      //this.CheckClosure(true, true); //fix any flipped orientation in facets, the second 'true' is for verbose
      //these functions should be passed outside this object?
      return prism;
    }
    
  }
};

/** Barrel Tracker imported from GDML file
 *
 *
 * The shapes are created using createShape which can be one of many basic geomtries.
 * See the examples Check_shape_*.xml in
 * [dd4hep's examples/ClientTests/compact](https://github.com/AIDASoft/DD4hep/tree/master/examples/ClientTests/compact)
 * directory.
 *
 *
 * 
 *
 * \ingroup trackers
 *
 * \code
 * \endcode
 *
 *
 * @author Whitney Armstrong, Tuna Tasali
 */




static Ref_t create_BarrelTrackerOuter(Detector& description, xml_h e, SensitiveDetector sens) {

  typedef vector<PlacedVolume> Placements;
  xml_det_t x_det = e;
  //Material air    = description.air();
  Material vacuum = description.vacuum();
  //Material silicon = description.material();
  int det_id      = x_det.id();
  string det_name = x_det.nameStr();
  DetElement sdet(det_name, det_id);

  map<string, Volume> volumes;
  map<string, Placements> sensitives;
  map<string, std::vector<VolPlane>> volplane_surfaces;
  map<string, std::array<double, 2>> module_thicknesses;

  PlacedVolume pv;

  // Set detector type flag
  dd4hep::xml::setDetectorTypeFlag(x_det, sdet);
  auto& params = DD4hepDetectorHelper::ensureExtension<dd4hep::rec::VariantParameters>(sdet);

  // Add the volume boundary material if configured
  for (xml_coll_t bmat(x_det, _Unicode(boundary_material)); bmat; ++bmat) {
    xml_comp_t x_boundary_material = bmat;
    DD4hepDetectorHelper::xmlToProtoSurfaceMaterial(x_boundary_material, params,
                                                    "boundary_material");
  }

  // dd4hep::xml::Dimension dimensions(x_det.dimensions());
  // Tube topVolumeShape(dimensions.rmin(), dimensions.rmax(), dimensions.length() * 0.5);
  // Volume assembly(det_name,topVolumeShape,air);
  Assembly assembly(det_name);

  //you can pick between these two options
  sens.setType("tracker");
  //sens.setType("calorimeter");
  //
  //

  // Loop over the suports
  for (xml_coll_t su(x_det, _U(support)); su; ++su) {
    xml_comp_t x_support     = su;
    double support_thickness = getAttrOrDefault(x_support, _U(thickness), 2.0 * mm);
    double support_length    = getAttrOrDefault(x_support, _U(length), 2.0 * mm);
    double support_rmin      = getAttrOrDefault(x_support, _U(rmin), 2.0 * mm);
    double support_zstart    = getAttrOrDefault(x_support, _U(zstart), 2.0 * mm);
    std::string support_name =
        getAttrOrDefault<std::string>(x_support, _Unicode(name), "support_tube");
    std::string support_vis = getAttrOrDefault<std::string>(x_support, _Unicode(vis), "AnlRed");
    xml_dim_t pos(x_support.child(_U(position), false));
    xml_dim_t rot(x_support.child(_U(rotation), false));
    Solid support_solid;
    if (x_support.hasChild(_U(shape))) {
      xml_comp_t shape(x_support.child(_U(shape)));
      string shape_type = shape.typeStr();
      support_solid     = xml::createShape(description, shape_type, shape);
    } else {
      support_solid = Tube(support_rmin, support_rmin + support_thickness, support_length / 2);
    }
    Transform3D tr =
        Transform3D(Rotation3D(), Position(0, 0, (support_zstart + support_length / 2)));
    if (pos.ptr() && rot.ptr()) {
      Rotation3D rot3D(RotationZYX(rot.z(0), rot.y(0), rot.x(0)));
      Position pos3D(pos.x(0), pos.y(0), pos.z(0));
      tr = Transform3D(rot3D, pos3D);
    } else if (pos.ptr()) {
      tr = Transform3D(Rotation3D(), Position(pos.x(0), pos.y(0), pos.z(0)));
    } else if (rot.ptr()) {
      Rotation3D rot3D(RotationZYX(rot.z(0), rot.y(0), rot.x(0)));
      tr = Transform3D(rot3D, Position());
    }
    Material support_mat = description.material(x_support.materialStr());
    Volume support_vol(support_name, support_solid, support_mat);
    support_vol.setVisAttributes(description.visAttributes(support_vis));
    pv = assembly.placeVolume(support_vol, tr);
    // pv = assembly.placeVolume(support_vol, Position(0, 0, support_zstart + support_length / 2));
  }



  // loop over the modules
  // to parse GDML files
  TGDMLParseBiggerFiles* parser = new TGDMLParseBiggerFiles();
  for (xml_coll_t mi(x_det, _U(module)); mi; ++mi) {
    xml_comp_t x_mod = mi;
    string m_nam     = x_mod.nameStr();

    if (volumes.find(m_nam) != volumes.end()) {
      printout(ERROR, "BarrelTrackerOuter",
               string((string("Module with named ") + m_nam + string(" already exists."))).c_str());
      throw runtime_error("Logics error in building modules.");
    }

    int ncomponents        = 0;
    int sensor_number      = 1;
    double total_thickness = 0;

    // Compute module total thickness from components
    xml_coll_t ci(x_mod, _U(module_component));
    for (ci.reset(), total_thickness = 0.0; ci; ++ci) {
      total_thickness += xml_comp_t(ci).thickness();
    }
    Assembly m_vol(m_nam);
    volumes[m_nam] = m_vol;
    m_vol.setVisAttributes(description.visAttributes(x_mod.visStr()));

    double thickness_so_far = 0.0;
    double thickness_sum    = -total_thickness / 2.0;
    for (xml_coll_t mci(x_mod, _U(module_component)); mci; ++mci, ++ncomponents) {
      xml_comp_t x_comp  = mci;
      xml_comp_t x_pos   = x_comp.position(false);
      xml_comp_t x_rot   = x_comp.rotation(false);
      const string c_nam = _toString(ncomponents, "component%d");
      const string c_nam_mesh = _toString(ncomponents, "component%d_MESH");

      //new code that consturcts them from GDML files
      //import the GDML file from the "file" attribute of the module_component
      std::string gdml_file =
          getAttrOrDefault<std::string>(x_comp, _Unicode(file), " ");
      printout(WARNING, "BarrelTrackerOuter", gdml_file);

      Volume c_vol(c_nam);
      printout(WARNING, "BarrelTrackerOuter", "Parsing a large GDML file may lead to segfault or heap overflow.");
      //STANDARD WHEN IMPORTING CAD MODELS: 
      //-> STAVE LONG AXIS MUST BE Z
      //-> STAVE FACES ALONG THE X AXIS
      c_vol = parser->GDMLReadFile(gdml_file.c_str());
      //check the validity of the volume
      if (!c_vol.isValid()) {
        printout(WARNING, "BarrelTrackerOuter", "%s", gdml_file.c_str());
        printout(WARNING, "BarrelTrackerOuter", "c_vol invalid, GDML parser failed!");
        std::_Exit(EXIT_FAILURE);
      }
      c_vol.import();
      c_vol.setMaterial(description.material(x_comp.materialStr()));
      //risky bit, might quickly fill up memory if too many solids are imported
      TessellatedSolid c_sol(c_vol.solid());
      //note: c_sol gets casted automatically to parent class TGeoTessellated by the copy constructor. IDK why?
      c_sol->CloseShape(true, true, true); //otherwise you get an infinite bounding box
      c_sol->CheckClosure(true, true); //fix any flipped orientation in facets, the second 'true' is for verbose
      c_vol.setSolid(c_sol);
      c_vol.setRegion(description, x_comp.regionStr());
      c_vol.setLimitSet(description, x_comp.limitsStr());
      //for former testing purposes
      //c_vol.setSolid(Box(12 * mm, 12 * mm, 120 * mm));
      
      /*
      //newer code, now can import .stl files directly
      //Still not ready :(
      //some code from: https://github.com/AIDASoft/DD4hep/blob/master/DDCAD/src/plugins/CADPlugins.cpp
      //create the component volume
      Volume c_vol(c_nam);
      //std::string stl_file =
          //getAttrOrDefault<std::string>(x_comp, _Unicode(file), " ");
      //double unit = mm; 
      TessellatedSolid c_sol = create_CAD_Shape(description, x_comp);
      //finally, refine the mesh
      c_sol->CloseShape(true, true, true); //otherwise you get an infinite bounding box
      c_sol->CheckClosure(true, true); //fix any flipped orientation in facets, the second 'true' is for verbose
      c_vol.setSolid(c_sol);*/
     
      
      // Utility variable for the relative z-offset based off the previous components
      const double zoff = thickness_sum + x_comp.thickness() / 2.0;
      //const double zoff = 0; //this value might cause issues, keep an eye out

      //now, the code branches in the following way:
      // if the volume is sensitive, build the tesselated solid into thickened pixels of extruded polyogons
      // and place those under m_vol
      // else, just place c_vol under m_vol
      if (x_comp.isSensitive()) { // sensitive volume, create the pixels
        printout(WARNING, "BarrelTrackingOuter", "SENSITIVE DETECTOR FOUND");
        //Volume sc_vol(c_nam);

        //loop over the facets of c_vol to define volumes and set them as sensitive
        //note these facets won't be actual pixels, but rather just bits of the mesh
        
        for(int facet_index = 0; facet_index < c_sol->GetNfacets(); facet_index++) {
          //printout(WARNING, "BarrelTrackingOuter", "SENSITIVE DETECTOR FACET FOUND");
          Volume sc_vol_tmp(c_nam + "_" + to_string(facet_index));
          TessellatedSolid::Facet current_facet = c_sol->GetFacet(facet_index); 
          //compute the normal to the facet
          TessellatedSolid::Vertex xhat(1., 0., 0.);
          TessellatedSolid::Vertex yhat(0., 1., 0.);
          TessellatedSolid::Vertex zhat(0., 0., 1.);
          //construct a Triangular prisim from the facet
          vector<TessellatedSolid::Vertex> vertices;
          //indices of the vertices
          int iv0 = -1;
          int iv1 = -1;
          int iv2 = -1;
          //int iv3 = -1;
          double extrusion_length = x_comp.thickness(); //thickness will control this
          if(current_facet.GetNvert() == 3) //triangular facet
          {
            //printout(WARNING, "BarrelTrackingOuter", "3333333SENSITIVE DETECTOR FACET FOUND");
            //get the vertex indices
            iv0 = current_facet.GetVertexIndex(0);
            iv1 = current_facet.GetVertexIndex(1);
            iv2 = current_facet.GetVertexIndex(2);
            //add the vertices to a vector
            vertices.push_back(c_sol->GetVertex(iv0));
            vertices.push_back(c_sol->GetVertex(iv1));
            vertices.push_back(c_sol->GetVertex(iv2));
            //construct a triangular prisim from the vertices
            struct TriangularPrism extruded_facet_access;
            TessellatedSolid extruded_facet = extruded_facet_access.return_TriangularPrism(vertices, extrusion_length);
            extruded_facet->CloseShape(true, true, true); //otherwise you get an infinite bounding box
            extruded_facet->CheckClosure(true, true); //fix any flipped orientation in facets, the second 'true' is for verbose
            
            //if the facet normal is not more than 90 deg off the x axis vector then keep it
            //note the standard in importing the cad models I defined when importing them
            double costheta = TessellatedSolid::Vertex::Dot(zhat, extruded_facet_access.getNormal());
            if (abs(costheta) >= 1/(sqrt(2))) //currently programmed for 45 deg max angle
            {
              //now define a facet volume and place it under sc_vol
              Volume sc_vol_facet("facet" + to_string(sensor_number));
              sc_vol_facet.setSolid(extruded_facet); //note: the dereferenced pointer is also a pointer
              sc_vol_facet.setMaterial(description.material(x_comp.materialStr()));
              sc_vol_facet.setRegion(description, x_comp.regionStr());
              sc_vol_facet.setLimitSet(description, x_comp.limitsStr());
              //now place the volume
              RotationX c_rot(M_PI/2);
              pv = m_vol.placeVolume(sc_vol_facet, Transform3D(c_rot, Position(0, 0, zoff)));
              sc_vol_facet.setVisAttributes(description, x_comp.visStr());
              pv.addPhysVolID("sensor", sensor_number);

              sensor_number = sensor_number + 1;
              sc_vol_facet.setSensitiveDetector(sens);
              sensitives[m_nam].push_back(pv);
              

              //SURFACE WORK
              //module_thicknesses[m_nam] = {extrusion_length,
                                          //0};
              module_thicknesses[m_nam] = {thickness_so_far + x_comp.thickness() / 2.0,
                                           total_thickness - thickness_so_far - x_comp.thickness() / 2.0};                   
              // -------- create a measurement plane for the tracking surface attched to the sensitive volume -----
              Vector3D u(-1., 0., 0.);
              Vector3D v(0., -1., 0.);
              Vector3D n(0., 0., 1.);
              //    Vector3D o( 0. , 0. , 0. ) ;
              // compute the inner and outer thicknesses that need to be assigned to the tracking surface
              // depending on whether the support is above or below the sensor
              double inner_thickness = module_thicknesses[m_nam][0];
              double outer_thickness = module_thicknesses[m_nam][1];
              SurfaceType type(SurfaceType::Sensitive);
              VolPlane surf(c_vol, type, inner_thickness, outer_thickness, u, v, n); //,o ) ;
              volplane_surfaces[m_nam].push_back(surf);
              printout(WARNING, "BarrelTrackingOuter", "AHAHAHAHAHHAHAHAHAHA");
            }
            else //for debugging
              printout(WARNING, "BarrelTrackingOuter", "Facet is not facing right direction, skipping");
            
            


            //release the facet extrusion*/
            //delete extruded_facet;
          }
          else if (current_facet.GetNvert() == 4) //quadrilateral facet
          {
            //yet to be implemented with quad prisim class
            printout(WARNING, "BarrelTrackingOuter", "Quadrilateral facets are not yet supported. Please revise your mesh.");
          }
          else throw runtime_error("Facet not triangular or quadrilateral.");


        }
        
      }
      else { // not a sensitive volume
        //place the volume
        if (x_pos && x_rot) {
          Position c_pos(x_pos.x(0), x_pos.y(0), x_pos.z(0) + zoff);
          RotationZYX c_rot(x_rot.z(0), x_rot.y(0), x_rot.x(0));
          pv = m_vol.placeVolume(c_vol, Transform3D(c_rot, c_pos));
        } else if (x_rot) {
          Position c_pos(0, 0, zoff);
          pv = m_vol.placeVolume(c_vol,
                                Transform3D(RotationZYX(x_rot.z(0), x_rot.y(0), x_rot.x(0)), c_pos));
        } else if (x_pos) {
          pv = m_vol.placeVolume(c_vol, Position(x_pos.x(0), x_pos.y(0), x_pos.z(0) + zoff));
        } else {
          //the c_rot is a temporary adjustment I added
          RotationX c_rot(M_PI/2);
          pv = m_vol.placeVolume(c_vol, Transform3D(c_rot, Position(0, 0, zoff)));
        
        }
        c_vol.setVisAttributes(description, x_comp.visStr());
      }
      thickness_sum += x_comp.thickness();
      thickness_so_far += x_comp.thickness();
      // apply relative offsets in z-position used to stack components side-by-side
      if (x_pos) {
        thickness_sum += x_pos.z(0);
        thickness_so_far += x_pos.z(0);
      }
    }
    
  }
  delete parser;

  // now build the layers
  for (xml_coll_t li(x_det, _U(layer)); li; ++li) {
    xml_comp_t x_layer  = li;
    xml_comp_t x_barrel = x_layer.child(_U(barrel_envelope));
    xml_comp_t x_layout = x_layer.child(_U(rphi_layout));
    xml_comp_t z_layout = x_layer.child(_U(z_layout)); // Get the <z_layout> element.
    int lay_id          = x_layer.id();
    string m_nam        = x_layer.moduleStr();
    string lay_nam      = det_name + _toString(x_layer.id(), "_layer%d");
    Tube lay_tub(x_barrel.inner_r(), x_barrel.outer_r(), x_barrel.z_length() / 2.0);
    Volume lay_vol(lay_nam, lay_tub, vacuum); // Create the layer envelope volume.
    Position lay_pos(0, 0, getAttrOrDefault(x_barrel, _U(z0), 0.));
    lay_vol.setVisAttributes(description.visAttributes(x_layer.visStr()));

    //Assembly lay_vol(lay_nam);
    //Position lay_pos(0, 0, getAttrOrDefault(x_barrel, _U(z0), 0.));

    double phi0     = x_layout.phi0();     // Starting phi of first module.
    double phi_tilt = x_layout.phi_tilt(); // Phi tilt of a module.
    double rc       = x_layout.rc();       // Radius of the module center.
    int nphi        = x_layout.nphi();     // Number of modules in phi.
    double rphi_dr  = x_layout.dr();       // The delta radius of every other module.
    double phi_incr = (M_PI * 2) / nphi;   // Phi increment for one module.
    double phic     = phi0;                // Phi of the module center.
    double z0       = z_layout.z0();       // Z position of first module in phi.
    double nz       = z_layout.nz();       // Number of modules to place in z.
    double z_dr     = z_layout.dr();       // Radial displacement parameter, of every other module.

    Volume module_env = volumes[m_nam];
    DetElement lay_elt(sdet, lay_nam, lay_id);
    Placements& sensVols = sensitives[m_nam];

    // the local coordinate systems of modules in dd4hep and acts differ
    // see http://acts.web.cern.ch/ACTS/latest/doc/group__DD4hepPlugins.html
    auto& layerParams =
        DD4hepDetectorHelper::ensureExtension<dd4hep::rec::VariantParameters>(lay_elt);

    for (xml_coll_t lmat(x_layer, _Unicode(layer_material)); lmat; ++lmat) {
      xml_comp_t x_layer_material = lmat;
      DD4hepDetectorHelper::xmlToProtoSurfaceMaterial(x_layer_material, layerParams,
                                                      "layer_material");
    }

    // Z increment for module placement along Z axis.
    // Adjust for z0 at center of module rather than
    // the end of cylindrical envelope.
    double z_incr = nz > 1 ? (2.0 * z0) / (nz - 1) : 0.0;
    // Starting z for module placement along Z axis.
    double module_z = -z0;
    int module      = 1;

    // Loop over the number of modules in phi.
    for (int ii = 0; ii < nphi; ii++) {
      double dx = z_dr * std::cos(phic + phi_tilt); // Delta x of module position.
      double dy = z_dr * std::sin(phic + phi_tilt); // Delta y of module position.
      double x  = rc * std::cos(phic);              // Basic x module position.
      double y  = rc * std::sin(phic);              // Basic y module position.

      // Loop over the number of modules in z.
      for (int j = 0; j < nz; j++) {
        string module_name = _toString(module, "module%d");
        DetElement mod_elt(lay_elt, module_name, module);

        Transform3D tr(RotationZYX(M_PI/2, ((M_PI / 2) - phic - phi_tilt), -M_PI/2), /*RotationZ(phic)*/
                        Position(x, y, module_z));

        pv = lay_vol.placeVolume(module_env, tr);
        pv.addPhysVolID("module", module);
        mod_elt.setPlacement(pv);
        for (size_t ic = 0; ic < sensVols.size(); ++ic) {
          PlacedVolume sens_pv = sensVols[ic];
          DetElement comp_de(mod_elt, std::string("de_") + sens_pv.volume().name(), module);
          comp_de.setPlacement(sens_pv);

          auto& comp_de_params =
              DD4hepDetectorHelper::ensureExtension<dd4hep::rec::VariantParameters>(comp_de);
          comp_de_params.set<string>("axis_definitions", "XYZ");
          //comp_de.setAttributes(description, sens_pv.volume(), x_layer.regionStr(), x_layer.limitsStr(),
                                 //xml_det_t(xmleles[m_nam]).visStr());
          //

          volSurfaceList(comp_de)->push_back(volplane_surfaces[m_nam][ic]);
        }

        /// Increase counters etc.
        module++;
        // Adjust the x and y coordinates of the module.
        x += dx;
        y += dy;
        // Flip sign of x and y adjustments.
        dx *= -1;
        dy *= -1;
        // Add z increment to get next z placement pos.
        module_z += z_incr;
      }
      phic += phi_incr; // Increment the phi placement of module.
      rc += rphi_dr;    // Increment the center radius according to dr parameter.
      rphi_dr *= -1;    // Flip sign of dr parameter.
      module_z = -z0;   // Reset the Z placement parameter for module.
    }
    // Create the PhysicalVolume for the layer.
    pv = assembly.placeVolume(lay_vol, lay_pos); // Place layer in mother
    pv.addPhysVolID("layer", lay_id);            // Set the layer ID.
    lay_elt.setAttributes(description, lay_vol, x_layer.regionStr(), x_layer.limitsStr(),
                          x_layer.visStr());
    lay_elt.setPlacement(pv);
  }
  
  //finally, place the world

  sdet.setAttributes(description, assembly, x_det.regionStr(), x_det.limitsStr(), x_det.visStr());
  assembly.setVisAttributes(description.invisible());
  pv = description.pickMotherVolume(sdet).placeVolume(assembly);
  pv.addPhysVolID("system", det_id); // Set the subdetector system ID.
  sdet.setPlacement(pv);
  printout(WARNING, "BarrelTrackerOuter", "DetElement instance \"sdet\" might be corrupted if the GDML design file is too big.");	
  
  return sdet;
}

//@}
// clang-format off
//Macros to access the XML files
DECLARE_DETELEMENT(epic_SiliconBarrel,    create_BarrelTrackerOuter)