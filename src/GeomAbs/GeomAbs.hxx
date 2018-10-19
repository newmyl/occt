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

#ifndef _GeomAbs_HeaderFile
#define _GeomAbs_HeaderFile

#include <GeomAbs_SurfaceType.hxx>
#include <Standard.hxx>
#include <Standard_Macro.hxx>

//! Defines
//! - tools to work with geomAbs
class GeomAbs
{
public:

  DEFINE_STANDARD_ALLOC

  //! Returns the string name for a type.
  //! @param surface type
  //! @return string identifier from the enumeration
  Standard_EXPORT static Standard_CString SurfaceTypeToString (const GeomAbs_SurfaceType theSurfaceType);

  //! Returns the surface type from the given string identifier (using case-insensitive comparison).
  //! @param theString string identifier
  //! @return type or GeomAbs_OtherSurface if string identifier is invalid
  static GeomAbs_SurfaceType SurfaceTypeFromString (const Standard_CString theString)
  {
    GeomAbs_SurfaceType aSurfaceType = GeomAbs_OtherSurface;
    SurfaceTypeFromString (theString, aSurfaceType);
    return aSurfaceType;
  }

  //! Determines the surface type from the given string identifier (using case-insensitive comparison).
  //! @param theString string identifier
  //! @param theSurfaceType detected surface type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean SurfaceTypeFromString (const Standard_CString theString,
                                                                 GeomAbs_SurfaceType& theType);
};

#endif // _GeomAbs_HeaderFile
