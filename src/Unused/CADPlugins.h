//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================

/// Framework include files
#include <DD4hep/DetFactoryHelper.h>
#include <DD4hep/DetectorTools.h>
#include <DD4hep/Printout.h>
#include <XML/Utilities.h>
#include <DDCAD/ASSIMPReader.h>
#include <DDCAD/ASSIMPWriter.h>

// C/C++ include files
#include <filesystem>

using dd4hep::except;
using dd4hep::printout;

//I defined this additional method
static dd4hep::Handle<TObject> create_CAD_Shape(dd4hep::Detector& dsc, xml_h x_comp)   {
  dd4hep::cad::ASSIMPReader rdr(dsc);
  double unit = dd4hep::mm;
  std::string fname = dd4hep::getAttrOrDefault<std::string>(x_comp, _Unicode(file), " ");

  auto shapes = rdr.readShapes(fname, unit);
  if ( shapes.empty() )   {
    except("CAD_Shape","+++ CAD file: %s does not contain any "
           "understandable tessellated shapes.", fname.c_str());
  }
  dd4hep::TessellatedSolid solid;
  std::size_t count = shapes.size();
  if ( count == 1 )   {
    solid = shapes[0].release();
  }
  else   {
    
      except("CAD_Shape","+++ CAD file: %s does contains %ld tessellated shapes. "
             "You need to add a selector.", fname.c_str(), shapes.size());
    
  }
  return solid;
}
//DECLARE_XML_SHAPE(CAD_Shape__shape_constructor,create_CAD_Shape)



