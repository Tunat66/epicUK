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


#include "TVector3.h"
//#include "TGDMLParse.h"
#include "TGDMLParseBiggerFiles.h"
#include "FileLoaderHelper.h"

using namespace std;
using namespace dd4hep;
using namespace dd4hep::rec;
using namespace dd4hep::detail;

/** Barrel Tracker with space frame.
 *
  *
 * The shapes are created using createShape which can be one of many basic geomtries.
 * See the examples Check_shape_*.xml in
 * [dd4hep's examples/ClientTests/compact](https://github.com/AIDASoft/DD4hep/tree/master/examples/ClientTests/compact)
 * directory.
 *
 *
 * - Optional "frame" tag within the module element.
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
  //map<string, std::vector<VolPlane>> volplane_surfaces;
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

  // loop over the modules
  // to parse GDML files
  
  TGDMLParseBiggerFiles* parser = new TGDMLParseBiggerFiles();
  for (xml_coll_t mi(x_det, _U(module)); mi; ++mi) {
    xml_comp_t x_mod = mi;
    string m_nam     = x_mod.nameStr();

    if (volumes.find(m_nam) != volumes.end()) {
      printout(ERROR, "BarrelTrackerWithFrame",
               string((string("Module with named ") + m_nam + string(" already exists."))).c_str());
      throw runtime_error("Logics error in building modules.");
    }

    int ncomponents        = 0;
    int sensor_number      = 1;
    double total_thickness = 0;

    // Compute module total thickness from components
    xml_coll_t ci(x_mod, _U(module_component));
    //for (ci.reset(), total_thickness = 0.0; ci; ++ci) {
      //total_thickness += xml_comp_t(ci).thickness();
    //}
    Assembly m_vol(m_nam);
    volumes[m_nam] = m_vol;
    m_vol.setVisAttributes(description.visAttributes(x_mod.visStr()));

    double thickness_so_far = 0.0;
    //double thickness_sum    = -total_thickness / 2.0;
    for (xml_coll_t mci(x_mod, _U(module_component)); mci; ++mci, ++ncomponents) {
      xml_comp_t x_comp  = mci;
      xml_comp_t x_pos   = x_comp.position(false);
      xml_comp_t x_rot   = x_comp.rotation(false);
      const string c_nam = _toString(ncomponents, "component%d");

      //old code which constructed pieces as boxes
      //Box c_box(x_comp.width() / 2, x_comp.length() / 2, x_comp.thickness() / 2);
      //Volume c_vol(c_nam, c_box, description.material(x_comp.materialStr()));

      //new code that consturcts them from GDML files
      //import the GDML file from the "file" attribute of the module_component
      std::string gdml_file =
          getAttrOrDefault<std::string>(x_comp, _Unicode(file), " ");
      printout(WARNING, "BarrelTrackerOuter", gdml_file);
        
      //std::string L3L4_name = "L3L4";
      //int L3L4_id = 10002;

      //note that I accumulate the GDML data in the heap with a Volume* object 
      //as the gdml file might be big
      //parse the file
      Volume c_vol(c_nam);
      
        printout(WARNING, "BarrelTrackerOuter", "Parsing a large GDML file may lead to segfault or heap overflow.");
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
      TessellatedSolid c_sol = c_vol.solid();
      c_sol->CloseShape(true, true, true); //otherwise you get an infinite bounding box
      c_sol->CheckClosure(true, true); //fix any flipped orientation in facets, the second 'true' is for verbose
      c_vol.setSolid(c_sol);
      //c_vol.setSolid(Box(12 * mm, 12 * mm, 120 * mm));
      

      
      // Utility variable for the relative z-offset based off the previous components
      //const double zoff = thickness_sum + x_comp.thickness() / 2.0;
      const double zoff = 0; //this value might cause issues, keep an eye out
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
      //pv = m_vol.placeVolume(*c_vol, Position(0, 0, zoff));
      //printout(WARNING, "BarrelTrackerOuter", "L3/L4 Limits: %s", x_comp.limitsStr().c_str());
      //printout(WARNING, "BarrelTrackerOuter", "L3/L4 Limits:");
      //c_vol->setRegion(description, x_comp.regionStr());
      //c_vol->setLimitSet(description, x_comp.limitsStr());
      c_vol.setVisAttributes(description, x_comp.visStr());
      if (x_comp.isSensitive()) {
        printout(WARNING, "BarrelTrackingOuter", "SENSITIVE DETECTOR FOUND");
        pv.addPhysVolID("sensor", sensor_number);
        sensor_number = sensor_number + 1;
        c_vol.setSensitiveDetector(sens);
        sensitives[m_nam].push_back(pv);
        module_thicknesses[m_nam] = {thickness_so_far + x_comp.thickness() / 2.0,
                                     total_thickness - thickness_so_far - x_comp.thickness() / 2.0};

        // -------- create a measurement plane for the tracking surface attched to the sensitive volume -----
        Vector3D u(-1., 0., 0.);
        Vector3D v(0., -1., 0.);
        Vector3D n(0., 0., 1.);
        //    Vector3D o( 0. , 0. , 0. ) ;

        // compute the inner and outer thicknesses that need to be assigned to the tracking surface
        // depending on whether the support is above or below the sensor
        //double inner_thickness = module_thicknesses[m_nam][0];
        //double outer_thickness = module_thicknesses[m_nam][1];

        SurfaceType type(SurfaceType::Sensitive);

        // if( isStripDetector )
        //  type.setProperty( SurfaceType::Measurement1D , true ) ;

        //VolPlane surf(*c_vol, type, inner_thickness, outer_thickness, u, v, n); //,o ) ;
        //volplane_surfaces[m_nam].push_back(surf);

        //--------------------------------------------
        
      }
      //thickness_sum += x_comp.thickness();
      //thickness_so_far += x_comp.thickness();
      // apply relative offsets in z-position used to stack components side-by-side
      //if (x_pos) {
        //thickness_sum += x_pos.z(0);
        //thickness_so_far += x_pos.z(0);
      //}
      //free the memory from the c_vol extracted from gdml
      
    }
    // the module assembly volume
    
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
    //lay_vol.setSensitiveDetector(sens);

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

    //module_env.setSensitiveDetector(sens);

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

          //volSurfaceList(comp_de)->push_back(volplane_surfaces[m_nam][ic]);
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
  /*
  //OLD CODE
  //import the GDML file of the outer two layers
  //probable change: I will import the two rings as two separate files
  //xml_comp_t x_det_L3L4_gdmlfile = x_det.child("L3L4_gdmlfile");
  //std::string L3L4_gdml_file =
      //getAttrOrDefault<std::string>(x_det_L3L4_gdmlfile, _Unicode(file), " ");
  //std::string L3L4_gdml_material =
      //getAttrOrDefault<std::string>(x_det_L3L4_gdmlfile, _Unicode(material), " ");
  //now parse the file
  TGDMLParse* parser = new TGDMLParse();
  std::string L3L4_name = "L3L4";
  int L3L4_id = 10002;
  Volume* L3L4_vol = new Volume(L3L4_name);
   printout(WARNING, "BarrelTrackerOuter", "Parsing GDML may lead to segfault.");
  *L3L4_vol = parser->GDMLReadFile(L3L4_gdml_file.c_str());
 
  if (!L3L4_vol->isValid()) {
    printout(WARNING, "BarrelTrackerOuter", "%s", L3L4_gdml_file.c_str());
    printout(WARNING, "BarrelTrackerOuter", "L3L4_vol invalid, GDML parser failed!");
    std::_Exit(EXIT_FAILURE);
  }
  delete parser;
  L3L4_vol->import();

  //configure: set solid and material etc.
  L3L4_vol->setVisAttributes(description, x_det.visStr());
  TessellatedSolid L3L4_solid = L3L4_vol->solid();
  L3L4_solid->CloseShape(true, true, true); // tesselated solid not closed by import!
  //Material L3L4_material = description.material(L3L4_gdml_material.c_str());
  //Material L3L4_material = silicon;
  //L3L4_vol->setMaterial(L3L4_material);
  L3L4_vol->setMaterial(air);
  

  //TEMPORARY: SET THE ENTIRE THING AS SENSITIVE
  L3L4_vol->setSensitiveDetector(sens);
  
/
 

  //for exercise, adding a volume that contains all the detector layers
  Position L3L4_pos(0, 0, 0); //placed at the origin
  pv = assembly.placeVolume(*L3L4_vol, L3L4_pos);
  
  delete L3L4_vol;
		    
			    
  //make it a sensitive element by adding it to the detector
  DetElement L3L4_elt(sdet, L3L4_name, L3L4_id); 
  pv.addPhysVolID("L3L4Volume", L3L4_id);
  L3L4_elt.setPlacement(pv);
  */
  

  //finally, place the world

  sdet.setAttributes(description, assembly, x_det.regionStr(), x_det.limitsStr(), x_det.visStr());
  assembly.setVisAttributes(description.invisible());
  pv = description.pickMotherVolume(sdet).placeVolume(assembly);
  pv.addPhysVolID("system", det_id); // Set the subdetector system ID.
  sdet.setPlacement(pv);
  printout(WARNING, "BarrelTrackerOuter", "DetElement instance \"sdet\" might be corrupted if the GDML design file is too big.");	
  
  //some debugging
  acts::InitLogLevel=trace;
  
  return sdet;

}

//@}
// clang-format off
//Macros to access the XML files
//DECLARE_DETELEMENT(epic_BarrelTrackerWithFrame, create_BarrelTrackerOuter)
//DECLARE_DETELEMENT(epic_TrackerBarrel,   create_BarrelTrackerOuter)
//DECLARE_DETELEMENT(epic_VertexBarrelOuter,    create_BarrelTrackerOuter)
//DECLARE_DETELEMENT(epic_TOFBarrel,       create_BarrelTrackerOuter)
//DECLARE_DETELEMENT(epic_InnerMPGDBarrel,       create_BarrelTrackerOuter)
DECLARE_DETELEMENT(epic_SiliconBarrel,    create_BarrelTrackerOuter)