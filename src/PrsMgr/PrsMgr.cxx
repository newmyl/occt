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

#include <PrsMgr.hxx>

#include <TCollection_AsciiString.hxx>

namespace
{
  static Standard_CString PrsMgr_Table_PrintTypeOfPresentation3d[2] =
  {
    "ALL_VIEW", "PROJECTOR_DEPENDANT"
  };
}

//=======================================================================
//function : TypeOfPresentation3dToString
//purpose  :
//=======================================================================
Standard_CString PrsMgr::TypeOfPresentation3dToString (PrsMgr_TypeOfPresentation3d theType)
{
  return PrsMgr_Table_PrintTypeOfPresentation3d[theType];
}

//=======================================================================
//function : TypeOfPresentation3dFromString
//purpose  :
//=======================================================================
Standard_Boolean PrsMgr::TypeOfPresentation3dFromString (Standard_CString theTypeString,
                                                   PrsMgr_TypeOfPresentation3d& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= PrsMgr_TOP_ProjectorDependant; ++aTypeIter)
  {
    Standard_CString aTypeName = PrsMgr_Table_PrintTypeOfPresentation3d[aTypeIter];
    if (aName == aTypeName)
    {
      theType = PrsMgr_TypeOfPresentation3d (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}
