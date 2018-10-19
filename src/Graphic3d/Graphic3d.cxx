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

#include <Graphic3d.hxx>

#include <TCollection_AsciiString.hxx>

namespace
{
  static Standard_CString Graphic3d_Table_PrintZLayerId[6] =
  {
    "INKNOWN", "DEFAULT", "TOP", "TOPMOST", "TOP_OSD", "BOT_OSD"
  };

  static Standard_CString Graphic3d_Table_PrintHorizontalTextAlignment[3] =
  {
    "LEFT", "CENTER", "RIGHT"
  };

  static Standard_CString Graphic3d_Table_PrintVerticalTextAlignment[4] =
  {
    "BOTTOM", "CENTER", "TOP", "TOP_FIRST_LINE"
  };

  static Standard_CString Graphic3d_Table_PrintTextPath[4] =
  {
    "UP", "DOWN", "LEFT", "RIGHT"
  };

  static Standard_CString Graphic3d_Table_PrintAlphaMode[4] =
  {
    "BLEND_AUTO", "OPAQUE", "MASK", "BLEND"
  };

  static Standard_CString Graphic3d_Table_PrintTypeOfShadingModel[5] =
  {
    "DEFAULT", "UNLIT", "FACET", "VERTEX", "FRAGMENT"
  };

  static Standard_CString Graphic3d_Table_PrintTypeOfAttribute[5] =
  {
    "POS", "NORM", "UV", "COLOR", "CUSTOM"
  };

  static Standard_CString Graphic3d_Table_PrintTypeOfData[7] =
  {
    "USHORT", "UINT", "VEC2", "VEC3", "VEC4", "VEC4UB", "FLOAT"
  };
}

//=======================================================================
//function : ZLayerIdToString
//purpose  :
//=======================================================================
Standard_CString Graphic3d::ZLayerIdToString (Graphic3d_ZLayerId theType)
{
  switch (theType)
  {
    case Graphic3d_ZLayerId_UNKNOWN: return Graphic3d_Table_PrintZLayerId[0]; break;
    case Graphic3d_ZLayerId_Default: return Graphic3d_Table_PrintZLayerId[1]; break;
    case Graphic3d_ZLayerId_Top: return Graphic3d_Table_PrintZLayerId[2]; break;
    case Graphic3d_ZLayerId_Topmost: return Graphic3d_Table_PrintZLayerId[3]; break;
    case Graphic3d_ZLayerId_TopOSD: return Graphic3d_Table_PrintZLayerId[4]; break;
    case Graphic3d_ZLayerId_BotOSD: return Graphic3d_Table_PrintZLayerId[5]; break;
  }
  return "";
}

//=======================================================================
//function : ZLayerIdFromString
//purpose  :
//=======================================================================
Standard_Boolean Graphic3d::ZLayerIdFromString (Standard_CString theTypeString,
                                                Graphic3d_ZLayerId& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= 5; ++aTypeIter)
  {
    Standard_CString aTypeName = Graphic3d_Table_PrintZLayerId[aTypeIter];
    if (aName == aTypeName)
    {
      switch (aTypeIter)
      {
        case 0: theType = Graphic3d_ZLayerId_UNKNOWN; break;
        case 1: theType = Graphic3d_ZLayerId_Default; break;
        case 2: theType = Graphic3d_ZLayerId_Top; break;
        case 3: theType = Graphic3d_ZLayerId_Topmost; break;
        case 4: theType = Graphic3d_ZLayerId_TopOSD; break;
        case 5: theType = Graphic3d_ZLayerId_BotOSD; break;
      }
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : HorizontalTextAlignmentToString
//purpose  :
//=======================================================================
Standard_CString Graphic3d::HorizontalTextAlignmentToString (Graphic3d_HorizontalTextAlignment theType)
{
  return Graphic3d_Table_PrintHorizontalTextAlignment[theType];
}

//=======================================================================
//function : HorizontalTextAlignmentFromString
//purpose  :
//=======================================================================
Standard_Boolean Graphic3d::HorizontalTextAlignmentFromString (Standard_CString theTypeString,
                                                               Graphic3d_HorizontalTextAlignment& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Graphic3d_HTA_LEFT; aTypeIter <= Graphic3d_HTA_RIGHT; ++aTypeIter)
  {
    Standard_CString aTypeName = Graphic3d_Table_PrintHorizontalTextAlignment[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Graphic3d_HorizontalTextAlignment (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : VerticalTextAlignmentToString
//purpose  :
//=======================================================================
Standard_CString Graphic3d::VerticalTextAlignmentToString (Graphic3d_VerticalTextAlignment theType)
{
  return Graphic3d_Table_PrintVerticalTextAlignment[theType];
}

//=======================================================================
//function : VerticalTextAlignmentFromString
//purpose  :
//=======================================================================
Standard_Boolean Graphic3d::VerticalTextAlignmentFromString (Standard_CString theTypeString,
                                                             Graphic3d_VerticalTextAlignment& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Graphic3d_VTA_BOTTOM; aTypeIter <= Graphic3d_VTA_TOPFIRSTLINE; ++aTypeIter)
  {
    Standard_CString aTypeName = Graphic3d_Table_PrintVerticalTextAlignment[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Graphic3d_VerticalTextAlignment (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : TextPathToString
//purpose  :
//=======================================================================
Standard_CString Graphic3d::TextPathToString (Graphic3d_TextPath theType)
{
  return Graphic3d_Table_PrintTextPath[theType];
}

//=======================================================================
//function : TextPathFromString
//purpose  :
//=======================================================================
Standard_Boolean Graphic3d::TextPathFromString (Standard_CString theTypeString,
                                                Graphic3d_TextPath& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Graphic3d_TP_UP; aTypeIter <= Graphic3d_TP_RIGHT; ++aTypeIter)
  {
    Standard_CString aTypeName = Graphic3d_Table_PrintTextPath[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Graphic3d_TextPath (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : AlphaModeToString
//purpose  :
//=======================================================================
Standard_CString Graphic3d::AlphaModeToString (Graphic3d_AlphaMode theType)
{
  return Graphic3d_Table_PrintAlphaMode[theType + 1];
}

//=======================================================================
//function : AlphaModeFromString
//purpose  :
//=======================================================================
Standard_Boolean Graphic3d::AlphaModeFromString (Standard_CString theTypeString,
                                                 Graphic3d_AlphaMode& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Graphic3d_AlphaMode_BlendAuto; aTypeIter <= Graphic3d_AlphaMode_Blend; ++aTypeIter)
  {
    Standard_CString aTypeName = Graphic3d_Table_PrintAlphaMode[aTypeIter + 1];
    if (aName == aTypeName)
    {
      theType = Graphic3d_AlphaMode (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : TypeOfShadingModelToString
//purpose  :
//=======================================================================
Standard_CString Graphic3d::TypeOfShadingModelToString (Graphic3d_TypeOfShadingModel theType)
{
  return Graphic3d_Table_PrintTypeOfShadingModel[theType + 1];
}

//=======================================================================
//function : TypeOfShadingModelFromString
//purpose  :
//=======================================================================
Standard_Boolean Graphic3d::TypeOfShadingModelFromString (Standard_CString theTypeString,
                                                 Graphic3d_TypeOfShadingModel& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Graphic3d_TOSM_DEFAULT; aTypeIter <= Graphic3d_TOSM_FRAGMENT; ++aTypeIter)
  {
    Standard_CString aTypeName = Graphic3d_Table_PrintTypeOfShadingModel[aTypeIter + 1];
    if (aName == aTypeName)
    {
      theType = Graphic3d_TypeOfShadingModel (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : TypeOfAttributeToString
//purpose  :
//=======================================================================
Standard_CString Graphic3d::TypeOfAttributeToString (Graphic3d_TypeOfAttribute theType)
{
  return Graphic3d_Table_PrintTypeOfAttribute[theType];
}

//=======================================================================
//function : TypeOfAttributeFromString
//purpose  :
//=======================================================================
Standard_Boolean Graphic3d::TypeOfAttributeFromString (Standard_CString theTypeString,
                                                 Graphic3d_TypeOfAttribute& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Graphic3d_TOA_POS; aTypeIter <= Graphic3d_TOA_CUSTOM; ++aTypeIter)
  {
    Standard_CString aTypeName = Graphic3d_Table_PrintTypeOfAttribute[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Graphic3d_TypeOfAttribute (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}



//=======================================================================
//function : TypeOfDataToString
//purpose  :
//=======================================================================
Standard_CString Graphic3d::TypeOfDataToString (Graphic3d_TypeOfData theType)
{
  return Graphic3d_Table_PrintTypeOfData[theType];
}

//=======================================================================
//function : TypeOfDataFromString
//purpose  :
//=======================================================================
Standard_Boolean Graphic3d::TypeOfDataFromString (Standard_CString theTypeString,
                                                 Graphic3d_TypeOfData& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Graphic3d_TOD_USHORT; aTypeIter <= Graphic3d_TOD_FLOAT; ++aTypeIter)
  {
    Standard_CString aTypeName = Graphic3d_Table_PrintTypeOfData[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Graphic3d_TypeOfData (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}
