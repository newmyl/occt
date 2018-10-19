// Copyright (c) 2018 OPEN CASCADE SAS
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

#ifndef _PrsMgr_HeaderFile
#define _PrsMgr_HeaderFile

#include <PrsMgr_TypeOfPresentation3d.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Macro.hxx>
#include <Standard_Type.hxx>

//! Provides methods to cast enumerations of package to string value
class PrsMgr 
{
public:

  DEFINE_STANDARD_ALLOC
  
  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfPresentation3dToString (PrsMgr_TypeOfPresentation3d theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or PrsMgr_TOP_AllView if string identifier is invalid
  static PrsMgr_TypeOfPresentation3d TypeOfPresentation3dFromString (Standard_CString theTypeString)
  {
    PrsMgr_TypeOfPresentation3d aType = PrsMgr_TOP_AllView;
    TypeOfPresentation3dFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfPresentation3dFromString (const Standard_CString theTypeString,
                                                                       PrsMgr_TypeOfPresentation3d& theType);

};

#endif // _PrsMgr_HeaderFile
