// Created on: 2018-06-10
// Created by: Natalia Ermolaeva
// Copyright (c) 2017 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <GeomAbs.hxx>

#include <TCollection_AsciiString.hxx>

namespace
{
  static Standard_CString GeomAbs_Table_PrintSurfaceTypeEnum[11] =
  {
    "Plane", "Cylinder", "Cone", "Sphere", "Torus", "BezierSurface", "BSplineSurface",
    "SurfaceOfRevolution", "SurfaceOfExtrusion", "OffsetSurface", "OtherSurface"
  };
}

//=======================================================================
//function : SurfaceTypeToString
//purpose  :
//=======================================================================

Standard_CString GeomAbs::SurfaceTypeToString (const GeomAbs_SurfaceType theValue)
{
  return GeomAbs_Table_PrintSurfaceTypeEnum[theValue];
}

//=======================================================================
//function : SurfaceTypeFromString
//purpose  :
//=======================================================================

Standard_Boolean GeomAbs::SurfaceTypeFromString (const Standard_CString theString,
                                                 GeomAbs_SurfaceType& theType)
{
  TCollection_AsciiString aName (theString);
  //aName.UpperCase();
  for (Standard_Integer aTypeIter = GeomAbs_Plane; aTypeIter <= GeomAbs_OtherSurface; ++aTypeIter)
  {
    Standard_CString aTypeName = GeomAbs_Table_PrintSurfaceTypeEnum[aTypeIter];
    if (aName == aTypeName)
    {
      theType = GeomAbs_SurfaceType (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}
