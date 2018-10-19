// Created on: 1992-08-26
// Created by: Jean Louis FRENKEL
// Copyright (c) 1992-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#ifndef _Prs3d_HeaderFile
#define _Prs3d_HeaderFile

#include <Graphic3d_ArrayOfPrimitives.hxx>
#include <Prs3d_DatumAttribute.hxx>
#include <Prs3d_DatumAxes.hxx>
#include <Prs3d_DatumParts.hxx>
#include <Prs3d_DimensionArrowOrientation.hxx>
#include <Prs3d_DimensionTextHorizontalPosition.hxx>
#include <Prs3d_DimensionTextVerticalPosition.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_NListOfSequenceOfPnt.hxx>
#include <Prs3d_TypeOfHighlight.hxx>
#include <Prs3d_TypeOfHLR.hxx>
#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>
#include <Standard_Boolean.hxx>
#include <Standard_Real.hxx>
#include <Prs3d_VertexDrawMode.hxx>

class TopoDS_Shape;
class Prs3d_Presentation;

//! The Prs3d package provides the following services
//! -   a presentation object (the context for all
//! modifications to the display, its presentation will be
//! displayed in every view of an active viewer)
//! -   an attribute manager governing how objects such
//! as color, width, and type of line are displayed;
//! these are generic objects, whereas those in
//! StdPrs are specific geometries and topologies.
//! -   generic   algorithms providing default settings for
//! objects such as points, curves, surfaces and shapes
//! -   a root object which provides the abstract
//! framework for the DsgPrs definitions at work in
//! display of dimensions, relations and trihedra.
class Prs3d 
{
public:

  DEFINE_STANDARD_ALLOC
  
  //! draws an arrow at a given location, with respect
  //! to a given direction.
  Standard_EXPORT static Standard_Boolean MatchSegment (const Standard_Real X, const Standard_Real Y, const Standard_Real Z, const Standard_Real aDistance, const gp_Pnt& p1, const gp_Pnt& p2, Standard_Real& dist);
  
  //! Computes the absolute deflection value depending on
  //! the type of deflection in theDrawer:
  //! <ul>
  //! <li><b>Aspect_TOD_RELATIVE</b>: the absolute deflection is computed using the relative
  //! deviation coefficient from theDrawer and the shape's bounding box;</li>
  //! <li><b>Aspect_TOD_ABSOLUTE</b>: the maximal chordial deviation from theDrawer is returned.</li>
  //! </ul>
  //! In case of the type of deflection in theDrawer computed relative deflection for shape
  //! is stored as absolute deflection. It is necessary to use it later on for sub-shapes.
  //! This function should always be used to compute the deflection value for building
  //! discrete representations of the shape (triangualtion, wireframe) to avoid incosistencies
  //! between different representations of the shape and undesirable visual artifacts.
  Standard_EXPORT static Standard_Real GetDeflection (const TopoDS_Shape& theShape, const Handle(Prs3d_Drawer)& theDrawer);

  //! Assembles array of primitives for sequence of polylines.
  //! @param thePoints [in] the polylines sequence
  //! @return array of primitives
  Standard_EXPORT static Handle(Graphic3d_ArrayOfPrimitives) PrimitivesFromPolylines (const Prs3d_NListOfSequenceOfPnt& thePoints);

  //! Add primitives into new group in presentation and clear the list of polylines.
  Standard_EXPORT static void AddPrimitivesGroup (const Handle(Prs3d_Presentation)& thePrs,
                                                  const Handle(Prs3d_LineAspect)&   theAspect,
                                                  Prs3d_NListOfSequenceOfPnt&       thePolylines);

  //! Returns the string name for a given orientation type.
  //! @param theType orientation type
  //! @return string identifier from the list Xpos, Ypos, Zpos and others
  Standard_EXPORT static Standard_CString TypeOfHighlightToString (Prs3d_TypeOfHighlight theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return orientation type or Prs3d_TypeOfHighlight_None if string identifier is invalid
  static Prs3d_TypeOfHighlight TypeOfHighlightFromString (Standard_CString theTypeString)
  {
    Prs3d_TypeOfHighlight aType = Prs3d_TypeOfHighlight_None;
    TypeOfHighlightFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the shape type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected shape type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfHighlightFromString (const Standard_CString theTypeString,
                                                                     Prs3d_TypeOfHighlight& theType);

  //! Returns the string name for a given orientation type.
  //! @param theType orientation type
  //! @return string identifier from the list Xpos, Ypos, Zpos and others
  Standard_EXPORT static Standard_CString TypeOfHLRToString (Prs3d_TypeOfHLR theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return orientation type or Prs3d_TOH_NotSet if string identifier is invalid
  static Prs3d_TypeOfHLR TypeOfHLRFromString (Standard_CString theTypeString)
  {
    Prs3d_TypeOfHLR aType = Prs3d_TOH_NotSet;
    TypeOfHLRFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the shape type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected shape type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean TypeOfHLRFromString (const Standard_CString theTypeString,
                                                               Prs3d_TypeOfHLR& theType);

  //! Returns the string name for a given orientation type.
  //! @param theType orientation type
  //! @return string identifier from the list Xpos, Ypos, Zpos and others
  Standard_EXPORT static Standard_CString VertexDrawModeToString (Prs3d_VertexDrawMode theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return orientation type or Prs3d_VDM_Isolated if string identifier is invalid
  static Prs3d_VertexDrawMode VertexDrawModeFromString (Standard_CString theTypeString)
  {
    Prs3d_VertexDrawMode aType = Prs3d_VDM_Isolated;
    VertexDrawModeFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the shape type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected shape type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean VertexDrawModeFromString (const Standard_CString theTypeString,
                                                                    Prs3d_VertexDrawMode& theType);

  //! Returns the string name for a given orientation type.
  //! @param theType orientation type
  //! @return string identifier from the list Xpos, Ypos, Zpos and others
  Standard_EXPORT static Standard_CString DatumPartsToString (Prs3d_DatumParts theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return orientation type or Prs3d_DP_None if string identifier is invalid
  static Prs3d_DatumParts DatumPartsFromString (Standard_CString theTypeString)
  {
    Prs3d_DatumParts aType = Prs3d_DP_None;
    DatumPartsFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the shape type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected shape type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean DatumPartsFromString (const Standard_CString theTypeString,
                                                                Prs3d_DatumParts& theType);

  //! Returns the string name for a given orientation type.
  //! @param theType orientation type
  //! @return string identifier from the list Xpos, Ypos, Zpos and others
  Standard_EXPORT static Standard_CString DatumAttributeToString (Prs3d_DatumAttribute theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return orientation type or Prs3d_DA_XAxisLength if string identifier is invalid
  static Prs3d_DatumAttribute DatumAttributeFromString (Standard_CString theTypeString)
  {
    Prs3d_DatumAttribute aType = Prs3d_DA_XAxisLength;
    DatumAttributeFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the shape type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected shape type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean DatumAttributeFromString (const Standard_CString theTypeString,
                                                                    Prs3d_DatumAttribute& theType);

  //! Returns the string name for a given orientation type.
  //! @param theType orientation type
  //! @return string identifier from the list Xpos, Ypos, Zpos and others
  Standard_EXPORT static Standard_CString DatumAxesToString (Prs3d_DatumAxes theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return orientation type or Prs3d_DA_XAxis if string identifier is invalid
  static Prs3d_DatumAxes DatumAxesFromString (Standard_CString theTypeString)
  {
    Prs3d_DatumAxes aType = Prs3d_DA_XAxis;
    DatumAxesFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the shape type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected shape type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean DatumAxesFromString (const Standard_CString theTypeString,
                                                               Prs3d_DatumAxes& theType);

  //! Returns the string name for a given orientation type.
  //! @param theType orientation type
  //! @return string identifier from the list Xpos, Ypos, Zpos and others
  Standard_EXPORT static Standard_CString DimensionArrowOrientationToString (Prs3d_DimensionArrowOrientation theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return orientation type or Prs3d_DAO_Internal if string identifier is invalid
  static Prs3d_DimensionArrowOrientation DimensionArrowOrientationFromString (Standard_CString theTypeString)
  {
    Prs3d_DimensionArrowOrientation aType = Prs3d_DAO_Internal;
    DimensionArrowOrientationFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the shape type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected shape type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean DimensionArrowOrientationFromString (const Standard_CString theTypeString,
                                                                               Prs3d_DimensionArrowOrientation& theType);

  //! Returns the string name for a given orientation type.
  //! @param theType orientation type
  //! @return string identifier from the list Xpos, Ypos, Zpos and others
  Standard_EXPORT static Standard_CString DimensionTextHorizontalPositionToString (Prs3d_DimensionTextHorizontalPosition theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return orientation type or Prs3d_DTHP_Left if string identifier is invalid
  static Prs3d_DimensionTextHorizontalPosition DimensionTextHorizontalPositionFromString (Standard_CString theTypeString)
  {
    Prs3d_DimensionTextHorizontalPosition aType = Prs3d_DTHP_Left;
    DimensionTextHorizontalPositionFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the shape type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected shape type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean DimensionTextHorizontalPositionFromString (const Standard_CString theTypeString,
                                                                                     Prs3d_DimensionTextHorizontalPosition& theType);

  //! Returns the string name for a given orientation type.
  //! @param theType orientation type
  //! @return string identifier from the list Xpos, Ypos, Zpos and others
  Standard_EXPORT static Standard_CString DimensionTextVerticalPositionToString (Prs3d_DimensionTextVerticalPosition theType);

  //! Returns the orientation type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @return orientation type or Prs3d_DTVP_Above if string identifier is invalid
  static Prs3d_DimensionTextVerticalPosition DimensionTextVerticalPositionFromString (Standard_CString theTypeString)
  {
    Prs3d_DimensionTextVerticalPosition aType = Prs3d_DTVP_Above;
    DimensionTextVerticalPositionFromString (theTypeString, aType);
    return aType;
  }

  //! Determines the shape type from the given string identifier (using case-insensitive comparison).
  //! @param theTypeString string identifier
  //! @param theType detected shape type
  //! @return TRUE if string identifier is known
  Standard_EXPORT static Standard_Boolean DimensionTextVerticalPositionFromString (const Standard_CString theTypeString,
                                                                                   Prs3d_DimensionTextVerticalPosition& theType);

};

#endif // _Prs3d_HeaderFile
