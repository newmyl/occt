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

#include <SelectMgr.hxx>

#include <TCollection_AsciiString.hxx>

namespace
{
  static Standard_CString SelectMgr_Table_PrintPickingStrategy[2] =
  {
    "FIRST_ACCEPTABLE", "ONLY_TOPMOST"
  };
}

//=======================================================================
//function : TypeOfOrientationToString
//purpose  :
//=======================================================================
Standard_CString SelectMgr::PickingStrategyToString (SelectMgr_PickingStrategy theType)
{
  return SelectMgr_Table_PrintPickingStrategy[theType];
}

//=======================================================================
//function : PickingStrategyFromString
//purpose  :
//=======================================================================
Standard_Boolean SelectMgr::PickingStrategyFromString (Standard_CString theTypeString,
                                                       SelectMgr_PickingStrategy& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= SelectMgr_PickingStrategy_OnlyTopmost; ++aTypeIter)
  {
    Standard_CString aTypeName = SelectMgr_Table_PrintPickingStrategy[aTypeIter];
    if (aName == aTypeName)
    {
      theType = SelectMgr_PickingStrategy (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}
