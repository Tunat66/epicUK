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
#include "TGDMLParse.h"
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
  //Material silicon = description.silicon();
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

  //import the GDML file of the outer two layers
  //probable change: I will import the two rings as two separate files
  xml_comp_t x_det_L3L4_gdmlfile = x_det.child("L3L4_gdmlfile");
  std::string L3L4_gdml_file =
      getAttrOrDefault<std::string>(x_det_L3L4_gdmlfile, _Unicode(file), " ");
  std::string L3L4_gdml_material =
      getAttrOrDefault<std::string>(x_det_L3L4_gdmlfile, _Unicode(material), " ");
  //now parse the file
  TGDMLParse parser;
  std::string L3L4_name = "L3L4";
  int L3L4_id = 10002;
  Volume L3L4_vol(L3L4_name);
  L3L4_vol = parser.GDMLReadFile(L3L4_gdml_file.c_str());
  if (!L3L4_vol.isValid()) {
    printout(WARNING, "BarrelTrackerOuter", "%s", L3L4_gdml_file.c_str());
    printout(WARNING, "BarrelTrackerOuter", "L3L4_vol invalid, GDML parser failed!");
    std::_Exit(EXIT_FAILURE);
  }
  L3L4_vol.import();

  //configure: set solid and material etc.
  L3L4_vol.setVisAttributes(description, x_det.visStr());
  TessellatedSolid L3L4_solid = L3L4_vol.solid();
  L3L4_solid->CloseShape(true, true, true); // tesselated solid not closed by import!
  Material L3L4_material = description.material(L3L4_gdml_material.c_str());
  L3L4_vol.setMaterial(L3L4_material);


  //TEMPORARY: SET THE ENTIRE THING AS SENSITIVE
  L3L4_vol.setSensitiveDetector(sens);

/* CODE TO BE ACTIVATED LATER 

  //loop over the components (currently just one as we have only the assembly)
  Volume L3Stave[1]; //this will be much higher, for testing purposes the entire thing now is a single stave
  Volume L4Stave[1]; //currently unused, will use it later
  for (int j = 1; j < 2; j++) { 
 	
	std::string gdmlname;
	std::string solid_name
	
	//currently 10 is some arbitrary number, will change that later
	if (j < 2) {
		//L3 staves
		gdmlname   = _toString(j, "tile%d_gdmlfile");
     	        solid_name = _toString(j, "OuterHCalTile%02d");
	
	}	
	else {
		//L4 staves, implement later
	}

	 CODE TO BE ACTIVATED LATER 
// tile shape gdml file info
    xml_comp_t x_det_tgdmlfile = x_det.child(gdmlname);

    std::string tgdml_file = getAttrOrDefault<std::string>(x_det_tgdmlfile, _Unicode(file), " ");
    ;
    std::string tgdml_material =
        getAttrOrDefault<std::string>(x_det_tgdmlfile, _Unicode(material), " ");
    std::string tgdml_url   = getAttrOrDefault<std::string>(x_det_tgdmlfile, _Unicode(url), " ");
    std::string tgdml_cache = getAttrOrDefault<std::string>(x_det_tgdmlfile, _Unicode(cache), " ");

    EnsureFileFromURLExists(tgdml_url, tgdml_file, tgdml_cache);
    if (!fs::exists(fs::path(tgdml_file))) {
      printout(ERROR, "BarrelHCalCalorimeter_geo", "file " + tgdml_file + " does not exist");
      printout(ERROR, "BarrelHCalCalorimeter_geo",
               "use a FileLoader plugin before the field element");
      std::_Exit(EXIT_FAILURE);
    }

    Volume solidVolume = parser.GDMLReadFile(tgdml_file.c_str());
    if (!solidVolume.isValid()) {
      printout(WARNING, "BarrelHCalCalorimeter_geo", "%s", tgdml_file.c_str());
      printout(WARNING, "BarrelHCalCalorimeter_geo", "solidVolume invalid, GDML parser failed!");
      std::_Exit(EXIT_FAILURE);
    }
    solidVolume.import();
    solidVolume.setVisAttributes(description, x_det.visStr());
    TessellatedSolid volume_solid = solidVolume.solid();
    volume_solid->CloseShape(true, true, true); // tesselated solid not closed by import!
    Material tile_material = description.material(tgdml_material.c_str());
    solidVolume.setMaterial(tile_material);

    solidVolume.setSensitiveDetector(sens);
  
  
  }
*/ 
 

  //for exercise, adding a volume that contains all the detector layers
  Position L3L4_pos(0, 0, 0); //placed at the origin
  pv = assembly.placeVolume(L3L4_vol, L3L4_pos);
			    
			    
  //make it a sensitive element by adding it to the detector
  DetElement L3L4_elt(sdet, L3L4_name, L3L4_id); 
  pv.addPhysVolID("L3L4Volume", L3L4_id);
  L3L4_elt.setPlacement(pv);
 
  
  //finally, place the world

  sdet.setAttributes(description, assembly, x_det.regionStr(), x_det.limitsStr(), x_det.visStr());
  assembly.setVisAttributes(description.invisible());
  pv = description.pickMotherVolume(sdet).placeVolume(assembly);
  pv.addPhysVolID("system", det_id); // Set the subdetector system ID.
  sdet.setPlacement(pv);
  return sdet;
}

//@}
// clang-format off
//DECLARE_DETELEMENT(epic_BarrelTrackerWithFrame, create_BarrelTrackerWithFrame)
//DECLARE_DETELEMENT(epic_TrackerBarrel,   create_BarrelTrackerWithFrame)
DECLARE_DETELEMENT(epic_VertexBarrel,    create_BarrelTrackerOuter)
//DECLARE_DETELEMENT(epic_TOFBarrel,       create_BarrelTrackerWithFrame)
//DECLARE_DETELEMENT(epic_InnerMPGDBarrel,       create_BarrelTrackerWithFrame)
