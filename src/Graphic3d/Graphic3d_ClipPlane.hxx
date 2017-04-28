// Created on: 2013-07-12
// Created by: Anton POLETAEV
// Copyright (c) 2013-2014 OPEN CASCADE SAS
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

#ifndef _Graphic3d_ClipPlane_HeaderFile
#define _Graphic3d_ClipPlane_HeaderFile

#include <Aspect_HatchStyle.hxx>
#include <Graphic3d_AspectFillArea3d.hxx>
#include <Graphic3d_AspectFillCapping.hxx>
#include <Graphic3d_Mat4.hxx>
#include <Graphic3d_TextureMap.hxx>
#include <NCollection_Handle.hxx>
#include <NCollection_Vec4.hxx>
#include <Standard_Macro.hxx>
#include <Standard_Transient.hxx>
#include <Standard_TypeDef.hxx>
#include <gp_Pln.hxx>

//! Container for properties describing graphic driver clipping planes.
//! It is up to application to create instances of this class and specify its
//! properties. The instances are passed into graphic driver or other facilities
//! that implement clipping features (e.g. selection).
//! Depending on usage context the class can be used to specify:
//! - Global clipping applied over the whole scene.
//! - Object-level clipping applied for each particular object.
//! The plane equation is specified in "world" coordinate system.
//! Please note that the set of planes can define convex clipping volume.
//! Be aware that number of clip planes supported by OpenGl is implementation
//! dependent: at least 6 planes are available. Thus, take into account
//! number of clipping planes passed for rendering: the object planes plus
//! the view defined ones.
class Graphic3d_ClipPlane : public Standard_Transient
{
public:

  typedef NCollection_Vec4<Standard_Real> Equation;

public:

  //! Default constructor.
  //! Initializes clip plane container with the following properties:
  //! - Equation (0.0, 0.0, 1.0, 0)
  //! - IsOn (True),
  //! - IsCapping (False),
  //! - Material (Graphic3d_NOM_DEFAULT),
  //! - Texture (NULL),
  //! - HatchStyle (Aspect_HS_HORIZONTAL),
  //! - IsHatchOn (False)
  Standard_EXPORT Graphic3d_ClipPlane();

  //! Copy constructor.
  //! @param theOther [in] the copied plane.
  Standard_EXPORT Graphic3d_ClipPlane (const Graphic3d_ClipPlane& theOther);

  //! Construct clip plane for the passed equation.
  //! By default the plane is on, capping is turned off.
  //! @param theEquation [in] the plane equation.
  Standard_EXPORT Graphic3d_ClipPlane (const Equation& theEquation);

  //! Construct clip plane from the passed geometrical definition.
  //! By default the plane is on, capping is turned off.
  //! @param thePlane [in] the plane.
  Standard_EXPORT Graphic3d_ClipPlane (const gp_Pln& thePlane);

  //! Set plane equation by its geometrical definition.
  //! The equation is specified in "world" coordinate system.
  //! @param thePlane [in] the plane.
  Standard_EXPORT void SetEquation (const gp_Pln& thePlane);

  //! Set 4-component equation vector for clipping plane.
  //! The equation is specified in "world" coordinate system.
  //! @param theEquation [in] the XYZW (or "ABCD") equation vector.
  Standard_EXPORT void SetEquation (const Equation& theEquation);

  //! Get 4-component equation vector for clipping plane.
  //! @return clipping plane equation vector.
  const Equation& GetEquation() const { return myEquation; }

  //! Check that the clipping plane is turned on.
  //! @return boolean flag indicating whether the plane is in on or off state.
  Standard_Boolean IsOn() const { return myIsOn; }

  //! Change state of the clipping plane.
  //! @param theIsOn [in] the flag specifying whether the graphic driver
  //! clipping by this plane should be turned on or off.
  Standard_EXPORT void SetOn(const Standard_Boolean theIsOn);

  //! Get geometrical definition.
  //! @return geometrical definition of clipping plane
  const gp_Pln& ToPlane() const { return myPlane; }

  //! Clone plane. Virtual method to simplify copying procedure if plane
  //! class is redefined at application level to add specific fields to it
  //! e.g. id, name, etc.
  //! @return new instance of clipping plane with same properties and attributes.
  Standard_EXPORT virtual Handle(Graphic3d_ClipPlane) Clone() const;

  //! This ID is used for managing associated resources in graphical driver.
  //! The clip plane can be assigned within a range of IO which can be
  //! displayed in separate OpenGl contexts. For each of the context an associated
  //! OpenGl resource for graphical aspects should be created and kept.
  //! The resources are stored in graphical driver for each of individual groups
  //! of shared context under the clip plane identifier.
  //! @return clip plane resource identifier string.
  const TCollection_AsciiString& GetId() const { return myEntityUID; }

public:

  //! Change state of capping surface rendering.
  //! @param theIsOn [in] the flag specifying whether the graphic driver should
  //! perform rendering of capping surface produced by this plane. The graphic
  //! driver produces this surface for convex graphics by means of stencil-test
  //! and multi-pass rendering.
  Standard_EXPORT void SetCapping(const Standard_Boolean theIsOn);

  //! Check state of capping surface rendering.
  //! @return true (turned on) or false depending on the state.
  Standard_Boolean IsCapping() const { return myIsCapping; }

  //! Sets clipping section filling aspect.
  Standard_EXPORT void SetCappingSectionStyle (const Handle(Graphic3d_AspectFillCapping)& theStyle);

  //! Returns style used for drawing capping section.
  const Handle(Graphic3d_AspectFillCapping)& CappingSectionStyle() const { return mySectionStyle; }

  //! Flag indicating whether section style of the plane should overrides similar property of object presentation.
  //! Default value: FALSE (use dedicated presentation aspect style).
  bool ToOverrideCappingAspect() const { return myOverrideObjectStyle; }

  //! Sets flag for controlling the preference of using section style between clip plane and object.
  void SetToOverrideCappingAspect (const bool theToOverride) { myOverrideObjectStyle = theToOverride; }

  //! Returns plane's orientation matrix.
  Standard_EXPORT const Graphic3d_Mat4& OrientationMatrix() const;

private:

  //! Initializes plane and makes unique identifier (UID) to differentiate clipping plane entities.
  void init (const gp_Pln& thePlane = gp_Pln(),
             const Standard_Boolean theIsOn = Standard_True,
             const Standard_Boolean theIsCapping = Standard_False,
             const Standard_Boolean theOverrideStyle = Standard_False,
             const Handle(Graphic3d_AspectFillCapping)& theStyle = Handle(Graphic3d_AspectFillCapping)());

private:

  TCollection_AsciiString             myEntityUID;            //!< Unique identifier for the plane
  gp_Pln                              myPlane;                //!< Plane definition
  Equation                            myEquation;             //!< Plane equation vector
  Standard_Boolean                    myIsOn;                 //!< State of the clipping plane
  Standard_Boolean                    myIsCapping;            //!< State of graphic driver capping
  Standard_Boolean                    myOverrideObjectStyle;  //!< Flag forcing to use plane's section style rather than section style defined for object
  Handle(Graphic3d_AspectFillCapping) mySectionStyle;         //!< Style set for drawing capped solid section.
  mutable Standard_Boolean            myOrientationDirty;     //!< Boolean flag indicating whether orientation matrix is dirty or not.
  mutable Graphic3d_Mat4              myOrientationMat;       //!< Plane orientation matrix (for visualization purposes).

public:

  DEFINE_STANDARD_RTTIEXT (Graphic3d_ClipPlane, Standard_Transient)
};

DEFINE_STANDARD_HANDLE (Graphic3d_ClipPlane, Standard_Transient)

#endif
