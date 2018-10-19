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

#include <Aspect.hxx>

#include <TCollection_AsciiString.hxx>

namespace
{
  static Standard_CString Aspect_Table_PrintTypeOfDeflection[2] =
  {
    "RELATIVE", "ABSOLUTE"
  };

  static Standard_CString Aspect_Table_PrintTypeOfFacingModel[3] =
  {
    "BOTH", "BACK", "FRONT"
  };

  static Standard_CString Aspect_Table_PrintTypeOfHighlightMethod[2] =
  {
    "COLOR", "BOUNDBOX"
  };

  static Standard_CString Aspect_Table_PrintTypeOfLine[6] =
  {
    "EMPTY", "SOLID", "DASH", "DOT", "DOT_DASH", "USER_DEFINED"
  };

  static Standard_CString Aspect_Table_PrintTypeOfMarker[15] =
  {
    "EMPTY", "POINT", "PLUS", "STAR", "X", "O", "O_POINT", "O_PLUS", "O_STAR", "O_X",
    "RING1", "RING2", "RING3", "BALL", "USER_DEFINED"
  };

  static Standard_CString Aspect_Table_PrintTypeOfDisplayText[6] =
  {
    "NORMAL", "SUBTITLE", "DEKALE", "BLEND", "DIMENSION", "SHADOW"
  };

  static Standard_CString Aspect_Table_PrintTypeOfStyleText[2] =
  {
    "NORMAL", "ANNOTATION"
  };

  static Standard_CString Aspect_Table_PrintInteriorStyle[6] =
  {
    "EMPTY", "HOLLOW", "HATCH", "SOLID", "HIDDEN_LINE", "POINT"
  };

  static Standard_CString Aspect_Table_PrintPolygonOffsetMode[7] =
  {
    "OFF", "FILL", "LINE", "POINT", "ALL", "NONE", "MASK"
  };

  static Standard_CString Aspect_Table_PrintHatchStyle[14] =
  {
    "SOLID", "HORIZONTAL", "HORIZONTAL_WIDE", "VERTICAL", "VERTICAL_WIDE",
    "DIAGONAL_45", "DIAGONAL_45_WIDE", "DIAGONAL_135", "DIAGONAL_135_WIDE",
    "GRID", "GRID_WIDE", "GRID_DIAGONAL", "GRID_DIAGONAL_WIDE", "NB"
  };

  static Standard_CString Aspect_Table_PrintTypeOfTriedronPosition[9] =
  {
    "CENTER", "TOP", "BOTTOM", "LEFT", "RIGHT", "LEFT_LOWER", "LEFT_UPPER", "RIGHT_LOWER", "RIGHT_UPPER"
  };
}

//=======================================================================
//function : TypeOfDeflectionToString
//purpose  :
//=======================================================================
Standard_CString Aspect::TypeOfDeflectionToString (Aspect_TypeOfDeflection theType)
{
  return Aspect_Table_PrintTypeOfDeflection[theType];
}

//=======================================================================
//function : TypeOfDeflectionFromString
//purpose  :
//=======================================================================
Standard_Boolean Aspect::TypeOfDeflectionFromString (Standard_CString theTypeString,
                                                     Aspect_TypeOfDeflection& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Aspect_TOD_ABSOLUTE; ++aTypeIter)
  {
    Standard_CString aTypeName = Aspect_Table_PrintTypeOfDeflection[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Aspect_TypeOfDeflection (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : TypeOfFacingModelToString
//purpose  :
//=======================================================================
Standard_CString Aspect::TypeOfFacingModelToString (Aspect_TypeOfFacingModel theType)
{
  return Aspect_Table_PrintTypeOfFacingModel[theType];
}

//=======================================================================
//function : TypeOfFacingModelFromString
//purpose  :
//=======================================================================
Standard_Boolean Aspect::TypeOfFacingModelFromString (Standard_CString theTypeString,
                                                   Aspect_TypeOfFacingModel& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Aspect_TOFM_FRONT_SIDE; ++aTypeIter)
  {
    Standard_CString aTypeName = Aspect_Table_PrintTypeOfFacingModel[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Aspect_TypeOfFacingModel (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : TypeOfHighlightMethodToString
//purpose  :
//=======================================================================
Standard_CString Aspect::TypeOfHighlightMethodToString (Aspect_TypeOfHighlightMethod theType)
{
  return Aspect_Table_PrintTypeOfHighlightMethod[theType];
}

//=======================================================================
//function : TypeOfHighlightMethodFromString
//purpose  :
//=======================================================================
Standard_Boolean Aspect::TypeOfHighlightMethodFromString (Standard_CString theTypeString,
                                                          Aspect_TypeOfHighlightMethod& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Aspect_TOHM_BOUNDBOX; ++aTypeIter)
  {
    Standard_CString aTypeName = Aspect_Table_PrintTypeOfHighlightMethod[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Aspect_TypeOfHighlightMethod (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : TypeOfLineToString
//purpose  :
//=======================================================================
Standard_CString Aspect::TypeOfLineToString (Aspect_TypeOfLine theType)
{
  return Aspect_Table_PrintTypeOfLine[theType];
}

//=======================================================================
//function : TypeOfLineFromString
//purpose  :
//=======================================================================
Standard_Boolean Aspect::TypeOfLineFromString (Standard_CString theTypeString,
                                               Aspect_TypeOfLine& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Aspect_TOL_EMPTY; aTypeIter <= Aspect_TOL_USERDEFINED; ++aTypeIter)
  {
    Standard_CString aTypeName = Aspect_Table_PrintTypeOfLine[aTypeIter + 1];
    if (aName == aTypeName)
    {
      theType = Aspect_TypeOfLine (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : TypeOfMarkerToString
//purpose  :
//=======================================================================
Standard_CString Aspect::TypeOfMarkerToString (Aspect_TypeOfMarker theType)
{
  return Aspect_Table_PrintTypeOfMarker[theType];
}

//=======================================================================
//function : TypeOfMarkerFromString
//purpose  :
//=======================================================================
Standard_Boolean Aspect::TypeOfMarkerFromString (Standard_CString theTypeString,
                                                 Aspect_TypeOfMarker& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Aspect_TOM_EMPTY; aTypeIter <= Aspect_TOM_USERDEFINED; ++aTypeIter)
  {
    Standard_CString aTypeName = Aspect_Table_PrintTypeOfMarker[aTypeIter + 1];
    if (aName == aTypeName)
    {
      theType = Aspect_TypeOfMarker (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : TypeOfDisplayTextToString
//purpose  :
//=======================================================================
Standard_CString Aspect::TypeOfDisplayTextToString (Aspect_TypeOfDisplayText theType)
{
  return Aspect_Table_PrintTypeOfDisplayText[theType];
}

//=======================================================================
//function : TypeOfDisplayTextFromString
//purpose  :
//=======================================================================
Standard_Boolean Aspect::TypeOfDisplayTextFromString (Standard_CString theTypeString,
                                                 Aspect_TypeOfDisplayText& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Aspect_TODT_NORMAL; aTypeIter <= Aspect_TODT_SHADOW; ++aTypeIter)
  {
    Standard_CString aTypeName = Aspect_Table_PrintTypeOfDisplayText[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Aspect_TypeOfDisplayText (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : TypeOfStyleTextToString
//purpose  :
//=======================================================================
Standard_CString Aspect::TypeOfStyleTextToString (Aspect_TypeOfStyleText theType)
{
  return Aspect_Table_PrintTypeOfStyleText[theType];
}

//=======================================================================
//function : TypeOfStyleTextFromString
//purpose  :
//=======================================================================
Standard_Boolean Aspect::TypeOfStyleTextFromString (Standard_CString theTypeString,
                                                 Aspect_TypeOfStyleText& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Aspect_TOST_NORMAL; aTypeIter <= Aspect_TOST_ANNOTATION; ++aTypeIter)
  {
    Standard_CString aTypeName = Aspect_Table_PrintTypeOfStyleText[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Aspect_TypeOfStyleText (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : InteriorStyleToString
//purpose  :
//=======================================================================
Standard_CString Aspect::InteriorStyleToString (Aspect_InteriorStyle theType)
{
  return Aspect_Table_PrintInteriorStyle[theType];
}

//=======================================================================
//function : InteriorStyleFromString
//purpose  :
//=======================================================================
Standard_Boolean Aspect::InteriorStyleFromString (Standard_CString theTypeString,
                                                  Aspect_InteriorStyle& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = Aspect_IS_EMPTY; aTypeIter <= Aspect_IS_POINT; ++aTypeIter)
  {
    Standard_CString aTypeName = Aspect_Table_PrintInteriorStyle[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Aspect_InteriorStyle (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : PolygonOffsetModeToString
//purpose  :
//=======================================================================
Standard_CString Aspect::PolygonOffsetModeToString (Aspect_PolygonOffsetMode theType)
{
  switch (theType)
  {
    case Aspect_POM_Off: return Aspect_Table_PrintPolygonOffsetMode[0];
    case Aspect_POM_Fill: return Aspect_Table_PrintPolygonOffsetMode[1];
    case Aspect_POM_Line: return Aspect_Table_PrintPolygonOffsetMode[2];
    case Aspect_POM_Point: return Aspect_Table_PrintPolygonOffsetMode[3];
    case Aspect_POM_All: return Aspect_Table_PrintPolygonOffsetMode[4];
    case Aspect_POM_None: return Aspect_Table_PrintPolygonOffsetMode[5];
    case Aspect_POM_Mask: return Aspect_Table_PrintPolygonOffsetMode[6];
  }
  return "";
}

//=======================================================================
//function : PolygonOffsetModeFromString
//purpose  :
//=======================================================================
Standard_Boolean Aspect::PolygonOffsetModeFromString (Standard_CString theTypeString,
                                                      Aspect_PolygonOffsetMode& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  if (aName == Aspect_Table_PrintPolygonOffsetMode[0]) { theType = Aspect_POM_Off; return Standard_True; }
  else if (aName == Aspect_Table_PrintPolygonOffsetMode[1]) { theType = Aspect_POM_Fill; return Standard_True; }
  else if (aName == Aspect_Table_PrintPolygonOffsetMode[2]) { theType = Aspect_POM_Line; return Standard_True; }
  else if (aName == Aspect_Table_PrintPolygonOffsetMode[3]) { theType = Aspect_POM_Point; return Standard_True; }
  else if (aName == Aspect_Table_PrintPolygonOffsetMode[4]) { theType = Aspect_POM_All; return Standard_True; }
  else if (aName == Aspect_Table_PrintPolygonOffsetMode[5]) { theType = Aspect_POM_None; return Standard_True; }
  else if (aName == Aspect_Table_PrintPolygonOffsetMode[6]) { theType = Aspect_POM_Mask; return Standard_True; }

  return Standard_False;
}

//=======================================================================
//function : HatchStyleToString
//purpose  :
//=======================================================================
Standard_CString Aspect::HatchStyleToString (Aspect_HatchStyle theType)
{
  return Aspect_Table_PrintHatchStyle[theType];
  switch (theType)
  {
    case Aspect_HS_SOLID: return Aspect_Table_PrintHatchStyle[0];
    case Aspect_HS_HORIZONTAL: return Aspect_Table_PrintHatchStyle[1];
    case Aspect_HS_HORIZONTAL_WIDE: return Aspect_Table_PrintHatchStyle[2];
    case Aspect_HS_VERTICAL: return Aspect_Table_PrintHatchStyle[3];
    case Aspect_HS_VERTICAL_WIDE: return Aspect_Table_PrintHatchStyle[4];
    case Aspect_HS_DIAGONAL_45: return Aspect_Table_PrintHatchStyle[5];
    case Aspect_HS_DIAGONAL_45_WIDE: return Aspect_Table_PrintHatchStyle[6];
    case Aspect_HS_DIAGONAL_135: return Aspect_Table_PrintHatchStyle[7];
    case Aspect_HS_DIAGONAL_135_WIDE: return Aspect_Table_PrintHatchStyle[8];
    case Aspect_HS_GRID: return Aspect_Table_PrintHatchStyle[9];
    case Aspect_HS_GRID_WIDE: return Aspect_Table_PrintHatchStyle[10];
    case Aspect_HS_GRID_DIAGONAL: return Aspect_Table_PrintHatchStyle[11];
    case Aspect_HS_GRID_DIAGONAL_WIDE: return Aspect_Table_PrintHatchStyle[12];
    case Aspect_HS_NB: return Aspect_Table_PrintHatchStyle[13];
  }
  return "";
}

//=======================================================================
//function : HatchStyleFromString
//purpose  :
//=======================================================================
Standard_Boolean Aspect::HatchStyleFromString (Standard_CString theTypeString,
                                               Aspect_HatchStyle& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  if (aName == Aspect_Table_PrintHatchStyle[0]) { theType = Aspect_HS_SOLID; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[1]) { theType = Aspect_HS_HORIZONTAL; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[2]) { theType = Aspect_HS_HORIZONTAL_WIDE; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[3]) { theType = Aspect_HS_VERTICAL; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[4]) { theType = Aspect_HS_VERTICAL_WIDE; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[5]) { theType = Aspect_HS_DIAGONAL_45; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[6]) { theType = Aspect_HS_DIAGONAL_45_WIDE; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[7]) { theType = Aspect_HS_DIAGONAL_135; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[8]) { theType = Aspect_HS_DIAGONAL_135_WIDE; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[9]) { theType = Aspect_HS_GRID; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[10]) { theType = Aspect_HS_GRID_WIDE; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[11]) { theType = Aspect_HS_GRID_DIAGONAL; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[12]) { theType = Aspect_HS_GRID_DIAGONAL_WIDE; return Standard_True; }
  else if (aName == Aspect_Table_PrintHatchStyle[13]) { theType = Aspect_HS_NB; return Standard_True; }
  return Standard_False;
}

//=======================================================================
//function : TypeOfTriedronPositionToString
//purpose  :
//=======================================================================
Standard_CString Aspect::TypeOfTriedronPositionToString (Aspect_TypeOfTriedronPosition theType)
{
  return Aspect_Table_PrintTypeOfTriedronPosition[theType];
  switch (theType)
  {
    case Aspect_TOTP_CENTER: return Aspect_Table_PrintTypeOfTriedronPosition[0];
    case Aspect_TOTP_TOP: return Aspect_Table_PrintTypeOfTriedronPosition[1];
    case Aspect_TOTP_BOTTOM: return Aspect_Table_PrintTypeOfTriedronPosition[2];
    case Aspect_TOTP_LEFT: return Aspect_Table_PrintTypeOfTriedronPosition[3];
    case Aspect_TOTP_RIGHT: return Aspect_Table_PrintTypeOfTriedronPosition[4];
    case Aspect_TOTP_LEFT_LOWER: return Aspect_Table_PrintTypeOfTriedronPosition[5];
    case Aspect_TOTP_LEFT_UPPER: return Aspect_Table_PrintTypeOfTriedronPosition[6];
    case Aspect_TOTP_RIGHT_LOWER: return Aspect_Table_PrintTypeOfTriedronPosition[7];
    case Aspect_TOTP_RIGHT_UPPER: return Aspect_Table_PrintTypeOfTriedronPosition[8];
  }
  return "";
}

//=======================================================================
//function : TypeOfTriedronPositionFromString
//purpose  :
//=======================================================================
Standard_Boolean Aspect::TypeOfTriedronPositionFromString (Standard_CString theTypeString,
                                                           Aspect_TypeOfTriedronPosition& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  if (aName == Aspect_Table_PrintTypeOfTriedronPosition[0]) { theType = Aspect_TOTP_CENTER; return Standard_True; }
  else if (aName == Aspect_Table_PrintTypeOfTriedronPosition[1]) { theType = Aspect_TOTP_TOP; return Standard_True; }
  else if (aName == Aspect_Table_PrintTypeOfTriedronPosition[2]) { theType = Aspect_TOTP_BOTTOM; return Standard_True; }
  else if (aName == Aspect_Table_PrintTypeOfTriedronPosition[3]) { theType = Aspect_TOTP_LEFT; return Standard_True; }
  else if (aName == Aspect_Table_PrintTypeOfTriedronPosition[4]) { theType = Aspect_TOTP_RIGHT; return Standard_True; }
  else if (aName == Aspect_Table_PrintTypeOfTriedronPosition[5]) { theType = Aspect_TOTP_LEFT_LOWER; return Standard_True; }
  else if (aName == Aspect_Table_PrintTypeOfTriedronPosition[6]) { theType = Aspect_TOTP_LEFT_UPPER; return Standard_True; }
  else if (aName == Aspect_Table_PrintTypeOfTriedronPosition[7]) { theType = Aspect_TOTP_RIGHT_LOWER; return Standard_True; }
  else if (aName == Aspect_Table_PrintTypeOfTriedronPosition[8]) { theType = Aspect_TOTP_RIGHT_UPPER; return Standard_True; }
  return Standard_False;
}
