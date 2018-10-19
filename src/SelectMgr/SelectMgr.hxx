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

#ifndef _SelectMgr_HeaderFile
#define _SelectMgr_HeaderFile

#include <SelectMgr_PickingStrategy.hxx>
#include <SelectMgr_StateOfSelection.hxx>
#include <SelectMgr_TypeOfBVHUpdate.hxx>
#include <SelectMgr_TypeOfUpdate.hxx>

#include <Standard.hxx>
#include <Standard_Macro.hxx>
#include <Standard_TypeDef.hxx>

//! This package contains the set of commands and services of the SelectMgr
class SelectMgr
{
public:

  DEFINE_STANDARD_ALLOC

  //! Returns the string name for a given orientation type.
  //! @param theType orientation type
  //! @return string identifier from the list Xpos, Ypos, Zpos and others
  Standard_EXPORT static Standard_CString PickingStrategyToString (SelectMgr_PickingStrategy theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return orientation type or V3d_TypeOfOrientation if string identifier is invalid
  static SelectMgr_PickingStrategy PickingStrategyFromString (Standard_CString theTypeString)
  {
    SelectMgr_PickingStrategy aType = SelectMgr_PickingStrategy_FirstAcceptable;
    PickingStrategyFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the shape type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected shape type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean PickingStrategyFromString (const Standard_CString theTypeString,
                                                                     SelectMgr_PickingStrategy& theType);

};

#endif // _SelectMgr_HeaderFile
