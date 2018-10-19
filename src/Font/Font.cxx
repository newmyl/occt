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

#include <Font.hxx>

#include <TCollection_AsciiString.hxx>

namespace
{
  static Standard_CString Font_Table_PrintFontAspect[5] =
  {
    "UNDEFINED", "REGULAR", "BOLD", "ITALIC", "BOLD_ITALIC"
  };

}

//=======================================================================
//function : FontAspectToString
//purpose  :
//=======================================================================
Standard_CString Font::FontAspectToString (Font_FontAspect theType)
{
  return Font_Table_PrintFontAspect[theType];
}

//=======================================================================
//function : FontAspectFromString
//purpose  :
//=======================================================================
Standard_Boolean Font::FontAspectFromString (Standard_CString theTypeString,
                                             Font_FontAspect& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Font_FA_BoldItalic; ++aTypeIter)
  {
    Standard_CString aTypeName = Font_Table_PrintFontAspect[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Font_FontAspect (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

