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

#ifndef _Graphic3d_HeaderFile
#define _Graphic3d_HeaderFile

#include <Graphic3d_AlphaMode.hxx>
#include <Graphic3d_Buffer.hxx>
#include <Graphic3d_HorizontalTextAlignment.hxx>
#include <Graphic3d_VerticalTextAlignment.hxx>
#include <Graphic3d_TextPath.hxx>
#include <Graphic3d_TypeOfShadingModel.hxx>

#include <Graphic3d_ZLayerId.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Macro.hxx>
#include <Standard_Type.hxx>

//! Provides methods to cast enumerations of package to string value
class Graphic3d
{
public:

  DEFINE_STANDARD_ALLOC
  
  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString ZLayerIdToString (Graphic3d_ZLayerId theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Graphic3d_ZLayerId_UNKNOWN if string identifier is invalid
  static Graphic3d_ZLayerId ZLayerIdFromString (Standard_CString theTypeString)
  {
    Graphic3d_ZLayerId aType = Graphic3d_ZLayerId_UNKNOWN;
    ZLayerIdFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean ZLayerIdFromString (const Standard_CString theTypeString,
                                                              Graphic3d_ZLayerId& theType);


  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString HorizontalTextAlignmentToString (Graphic3d_HorizontalTextAlignment theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Graphic3d_HTA_LEFT if string identifier is invalid
  static Graphic3d_HorizontalTextAlignment HorizontalTextAlignmentFromString (Standard_CString theTypeString)
  {
    Graphic3d_HorizontalTextAlignment aType = Graphic3d_HTA_LEFT;
    HorizontalTextAlignmentFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean HorizontalTextAlignmentFromString (const Standard_CString theTypeString,
                                                                             Graphic3d_HorizontalTextAlignment& theType);


  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString VerticalTextAlignmentToString (Graphic3d_VerticalTextAlignment theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Graphic3d_VTA_BOTTOM if string identifier is invalid
  static Graphic3d_VerticalTextAlignment VerticalTextAlignmentFromString (Standard_CString theTypeString)
  {
    Graphic3d_VerticalTextAlignment aType = Graphic3d_VTA_BOTTOM;
    VerticalTextAlignmentFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean VerticalTextAlignmentFromString (const Standard_CString theTypeString,
                                                                           Graphic3d_VerticalTextAlignment& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TextPathToString (Graphic3d_TextPath theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Graphic3d_TP_UP if string identifier is invalid
  static Graphic3d_TextPath TextPathFromString (Standard_CString theTypeString)
  {
    Graphic3d_TextPath aType = Graphic3d_TP_UP;
    TextPathFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TextPathFromString (const Standard_CString theTypeString,
                                                              Graphic3d_TextPath& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString AlphaModeToString (Graphic3d_AlphaMode theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Graphic3d_AlphaMode_Opaque if string identifier is invalid
  static Graphic3d_AlphaMode AlphaModeFromString (Standard_CString theTypeString)
  {
    Graphic3d_AlphaMode aType = Graphic3d_AlphaMode_Opaque;
    AlphaModeFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean AlphaModeFromString (const Standard_CString theTypeString,
                                                               Graphic3d_AlphaMode& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfShadingModelToString (Graphic3d_TypeOfShadingModel theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Graphic3d_TOSM_DEFAULT if string identifier is invalid
  static Graphic3d_TypeOfShadingModel TypeOfShadingModelFromString (Standard_CString theTypeString)
  {
    Graphic3d_TypeOfShadingModel aType = Graphic3d_TOSM_DEFAULT;
    TypeOfShadingModelFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfShadingModelFromString (const Standard_CString theTypeString,
                                                                        Graphic3d_TypeOfShadingModel& theType);
  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfAttributeToString (Graphic3d_TypeOfAttribute theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Graphic3d_TOA_POS if string identifier is invalid
  static Graphic3d_TypeOfAttribute TypeOfAttributeFromString (Standard_CString theTypeString)
  {
    Graphic3d_TypeOfAttribute aType = Graphic3d_TOA_POS;
    TypeOfAttributeFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfAttributeFromString (const Standard_CString theTypeString,
                                                                     Graphic3d_TypeOfAttribute& theType);

  //! Returns the string name for a given type.
  //! @param theType an enumeration type
  //! @return string identifier from the enumeration list
  Standard_EXPORT static Standard_CString TypeOfDataToString (Graphic3d_TypeOfData theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return enumeration type or Graphic3d_TOSM_DEFAULT if string identifier is invalid
  static Graphic3d_TypeOfData TypeOfDataFromString (Standard_CString theTypeString)
  {
    Graphic3d_TypeOfData aType = Graphic3d_TOD_USHORT;
    TypeOfDataFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfDataFromString (const Standard_CString theTypeString,
                                                                Graphic3d_TypeOfData& theType);

};

#endif // _Graphic3d_HeaderFile
