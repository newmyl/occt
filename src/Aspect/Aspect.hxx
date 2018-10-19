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

#ifndef _Aspect_HeaderFile
#define _Aspect_HeaderFile

#include <Aspect_HatchStyle.hxx>
#include <Aspect_InteriorStyle.hxx>
#include <Aspect_PolygonOffsetMode.hxx>
#include <Aspect_TypeOfDeflection.hxx>
#include <Aspect_TypeOfDisplayText.hxx>
#include <Aspect_TypeOfFacingModel.hxx>
#include <Aspect_TypeOfHighlightMethod.hxx>
#include <Aspect_TypeOfLine.hxx>
#include <Aspect_TypeOfMarker.hxx>
#include <Aspect_TypeOfStyleText.hxx>
#include <Aspect_TypeOfTriedronPosition.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Macro.hxx>
#include <Standard_Type.hxx>

//! Provides methods to cast enumerations of package to string value
class Aspect
{
public:

  DEFINE_STANDARD_ALLOC
  
  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfDeflectionToString (Aspect_TypeOfDeflection theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Aspect_TOFM_BOTH_SIDE if string identifier is invalid
  static Aspect_TypeOfDeflection TypeOfDeflectionFromString (Standard_CString theTypeString)
  {
    Aspect_TypeOfDeflection aType = Aspect_TOD_RELATIVE;
    TypeOfDeflectionFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfDeflectionFromString (const Standard_CString theTypeString,
                                                                      Aspect_TypeOfDeflection& theType);


  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfFacingModelToString (Aspect_TypeOfFacingModel theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Aspect_TOFM_BOTH_SIDE if string identifier is invalid
  static Aspect_TypeOfFacingModel TypeOfFacingModelFromString (Standard_CString theTypeString)
  {
    Aspect_TypeOfFacingModel aType = Aspect_TOFM_BOTH_SIDE;
    TypeOfFacingModelFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfFacingModelFromString (const Standard_CString theTypeString,
                                                                       Aspect_TypeOfFacingModel& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfHighlightMethodToString (Aspect_TypeOfHighlightMethod theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Aspect_TOHM_COLOR if string identifier is invalid
  static Aspect_TypeOfHighlightMethod TypeOfHighlightMethodFromString (Standard_CString theTypeString)
  {
    Aspect_TypeOfHighlightMethod aType = Aspect_TOHM_COLOR;
    TypeOfHighlightMethodFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfHighlightMethodFromString (const Standard_CString theTypeString,
                                                                           Aspect_TypeOfHighlightMethod& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfLineToString (Aspect_TypeOfLine theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Aspect_TOL_EMPTY if string identifier is invalid
  static Aspect_TypeOfLine TypeOfLineFromString (Standard_CString theTypeString)
  {
    Aspect_TypeOfLine aType = Aspect_TOL_EMPTY;
    TypeOfLineFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfLineFromString (const Standard_CString theTypeString,
                                                                Aspect_TypeOfLine& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfMarkerToString (Aspect_TypeOfMarker theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Aspect_TOM_EMPTY if string identifier is invalid
  static Aspect_TypeOfMarker TypeOfMarkerFromString (Standard_CString theTypeString)
  {
    Aspect_TypeOfMarker aType = Aspect_TOM_EMPTY;
    TypeOfMarkerFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfMarkerFromString (const Standard_CString theTypeString,
                                                                  Aspect_TypeOfMarker& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfDisplayTextToString (Aspect_TypeOfDisplayText theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Aspect_TODT_NORMAL if string identifier is invalid
  static Aspect_TypeOfDisplayText TypeOfDisplayTextFromString (Standard_CString theTypeString)
  {
    Aspect_TypeOfDisplayText aType = Aspect_TODT_NORMAL;
    TypeOfDisplayTextFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfDisplayTextFromString (const Standard_CString theTypeString,
                                                                       Aspect_TypeOfDisplayText& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfStyleTextToString (Aspect_TypeOfStyleText theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Aspect_TODT_NORMAL if string identifier is invalid
  static Aspect_TypeOfStyleText TypeOfStyleTextFromString (Standard_CString theTypeString)
  {
    Aspect_TypeOfStyleText aType = Aspect_TOST_NORMAL;
    TypeOfStyleTextFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfStyleTextFromString (const Standard_CString theTypeString,
                                                                     Aspect_TypeOfStyleText& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString InteriorStyleToString (Aspect_InteriorStyle theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Aspect_IS_EMPTY if string identifier is invalid
  static Aspect_InteriorStyle InteriorStyleFromString (Standard_CString theTypeString)
  {
    Aspect_InteriorStyle aType = Aspect_IS_EMPTY;
    InteriorStyleFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean InteriorStyleFromString (const Standard_CString theTypeString,
                                                                   Aspect_InteriorStyle& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString PolygonOffsetModeToString (Aspect_PolygonOffsetMode theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Aspect_POM_Off if string identifier is invalid
  static Aspect_PolygonOffsetMode PolygonOffsetModeFromString (Standard_CString theTypeString)
  {
    Aspect_PolygonOffsetMode aType = Aspect_POM_Off;
    PolygonOffsetModeFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean PolygonOffsetModeFromString (const Standard_CString theTypeString,
                                                                       Aspect_PolygonOffsetMode& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString HatchStyleToString (Aspect_HatchStyle theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Aspect_POM_Off if string identifier is invalid
  static Aspect_HatchStyle HatchStyleFromString (Standard_CString theTypeString)
  {
    Aspect_HatchStyle aType = Aspect_HS_SOLID;
    HatchStyleFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean HatchStyleFromString (const Standard_CString theTypeString,
                                                                Aspect_HatchStyle& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfTriedronPositionToString (Aspect_TypeOfTriedronPosition theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Aspect_TOTP_CENTER if string identifier is invalid
  static Aspect_TypeOfTriedronPosition TypeOfTriedronPositionFromString (Standard_CString theTypeString)
  {
    Aspect_TypeOfTriedronPosition aType = Aspect_TOTP_CENTER;
    TypeOfTriedronPositionFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfTriedronPositionFromString (const Standard_CString theTypeString,
                                                                            Aspect_TypeOfTriedronPosition& theType);

};

#endif // _Aspect_HeaderFile
