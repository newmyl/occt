// Created on: 2013-10-02
// Created by: Denis BOGOLEPOV
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

#ifndef _OpenGl_State_HeaderFile
#define _OpenGl_State_HeaderFile

#include <NCollection_List.hxx>
#include <Graphic3d_LightSet.hxx>
#include <OpenGl_Element.hxx>
#include <OpenGl_Vec.hxx>

//! Defines interface for OpenGL state.
class OpenGl_StateInterface
{
public:

  //! Creates new state.
  Standard_EXPORT OpenGl_StateInterface();

  //! Returns current state index.
  Standard_Size Index() const { return myIndex; }

  //! Increment current state.
  void Update() { ++myIndex; }

protected:

  Standard_Size myIndex; //!< current state index

};

//! Defines state of OCCT projection transformation.
class OpenGl_ProjectionState : public OpenGl_StateInterface
{
public:

  //! Creates uninitialized projection state.
  Standard_EXPORT OpenGl_ProjectionState();

  //! Sets new projection matrix.
  Standard_EXPORT void Set (const OpenGl_Mat4& theProjectionMatrix);

  //! Returns current projection matrix.
  Standard_EXPORT const OpenGl_Mat4& ProjectionMatrix() const;

  //! Returns inverse of current projection matrix.
  Standard_EXPORT const OpenGl_Mat4& ProjectionMatrixInverse() const;

private:

  OpenGl_Mat4         myProjectionMatrix;        //!< OCCT projection matrix
  mutable OpenGl_Mat4 myProjectionMatrixInverse; //!< Inverse of OCCT projection matrix
  bool                myInverseNeedUpdate;       //!< Is inversed matrix outdated?

};

//! Defines state of OCCT model-world transformation.
class OpenGl_ModelWorldState : public OpenGl_StateInterface
{
public:

  //! Creates uninitialized model-world state.
  Standard_EXPORT OpenGl_ModelWorldState();

  //! Sets new model-world matrix.
  Standard_EXPORT void Set (const OpenGl_Mat4& theModelWorldMatrix);

  //! Returns current model-world matrix.
  Standard_EXPORT const OpenGl_Mat4& ModelWorldMatrix() const;

  //! Returns inverse of current model-world matrix.
  Standard_EXPORT const OpenGl_Mat4& ModelWorldMatrixInverse() const;

private:

  OpenGl_Mat4         myModelWorldMatrix;        //!< OCCT model-world matrix
  mutable OpenGl_Mat4 myModelWorldMatrixInverse; //!< Inverse of OCCT model-world matrix
  bool                myInverseNeedUpdate;       //!< Is inversed matrix outdated?
  
};

//! Defines state of OCCT world-view transformation.
class OpenGl_WorldViewState : public OpenGl_StateInterface
{
public:

  //! Creates uninitialized world-view state.
  Standard_EXPORT OpenGl_WorldViewState();

  //! Sets new world-view matrix.
  Standard_EXPORT void Set (const OpenGl_Mat4& theWorldViewMatrix);

  //! Returns current world-view matrix.
  Standard_EXPORT const OpenGl_Mat4& WorldViewMatrix() const;

  //! Returns inverse of current world-view matrix.
  Standard_EXPORT const OpenGl_Mat4& WorldViewMatrixInverse() const;

private:

  OpenGl_Mat4         myWorldViewMatrix;        //!< OCCT world-view matrix
  mutable OpenGl_Mat4 myWorldViewMatrixInverse; //!< Inverse of OCCT world-view matrix
  bool                myInverseNeedUpdate;      //!< Is inversed matrix outdated?

};

//! Defines state of OCCT light sources.
class OpenGl_LightSourceState : public OpenGl_StateInterface
{
public:

  //! Creates uninitialized state of light sources.
  OpenGl_LightSourceState() {}

  //! Sets new light sources.
  void Set (const Handle(Graphic3d_LightSet)& theLightSources) { myLightSources = theLightSources; }

  //! Returns current list of light sources.
  const Handle(Graphic3d_LightSet)& LightSources() const { return myLightSources; }

private:

  Handle(Graphic3d_LightSet) myLightSources; //!< List of OCCT light sources

};

//! Defines generic state of OCCT clipping state.
class OpenGl_ClippingState
{
public:

  //! Creates new clipping state.
  Standard_EXPORT OpenGl_ClippingState();

  //! Returns current state index.
  Standard_Size Index() const { return myIndex; }

  //! Updates current state.
  Standard_EXPORT void Update();

  //! Reverts current state.
  Standard_EXPORT void Revert();

protected:

  Standard_Size                   myIndex;      //!< Current state index
  Standard_Size                   myNextIndex;  //!< Next    state index
  NCollection_List<Standard_Size> myStateStack; //!< Stack of previous states

};

//! Defines generic state of order-independent transparency rendering properties.
class OpenGl_OitState : public OpenGl_StateInterface
{
public:

  //! Creates new uniform state.
  OpenGl_OitState() : myToEnableWrite (false), myDepthFactor (0.5f) {}

  //! Sets the uniform values.
  //! @param theToEnableWrite [in] flag indicating whether color and coverage
  //!  values for OIT processing should be written by shader program.
  //! @param theDepthFactor [in] scalar factor [0-1] defining influence of depth
  //!  component of a fragment to its final coverage coefficient.
  void Set (const bool  theToEnableWrite,
            const float theDepthFactor)
  {
    myToEnableWrite = theToEnableWrite;
    myDepthFactor   = static_cast<float> (Max (0.f, Min (1.f, theDepthFactor)));
  }

  //! Returns flag indicating whether writing of output for OIT processing
  //! should be enabled/disabled.
  bool ToEnableWrite() const { return myToEnableWrite; }

  //! Returns factor defining influence of depth component of a fragment
  //! to its final coverage coefficient.
  float DepthFactor() const { return myDepthFactor; }

private:

  bool  myToEnableWrite; //!< writing color and coverage.
  float myDepthFactor;   //!< factor of depth influence to coverage.
};

//! Defines state of wireframe properties.
class OpenGl_WireframeState
{
public:

  //! Creates new wireframe state.
  Standard_EXPORT OpenGl_WireframeState() :
    myViewport       (OpenGl_Vec4 (0.0)),
    myWireframeColor (Quantity_NOC_WHITE),
    myWireframeWidth (1.0f),
    myIsQuadMode     (Standard_False),
    myIsColoringEdge (Standard_False) {}

  //! Set fields which got from face aspect.
  Standard_EXPORT void SetAspects (const OpenGl_Vec3&       theWireframeColor,
                                   const Standard_ShortReal theWireframeWidth,
                                   const Standard_Boolean   theIsQuadMode,
                                   const Standard_Boolean   theIsColoringEdge)
  {
    myWireframeColor = theWireframeColor;
    myWireframeWidth = theWireframeWidth;
    myIsQuadMode     = theIsQuadMode;
    myIsColoringEdge = theIsColoringEdge;
  }

  //! Set viewport field
  void SetViewport (const OpenGl_Vec4& theViewport) { myViewport = theViewport; }

  //! Return viewport
  const OpenGl_Vec4& Viewport() const { return myViewport; }

  //! Return color of wireframe
  const OpenGl_Vec3& WireframeColor() const { return myWireframeColor; }

  //! Return width of wireframe
  Standard_ShortReal WireframeWidth() const { return myWireframeWidth; }

  //! Return quad presentation flag
  Standard_Boolean IsQuadMode() const { return myIsQuadMode; }

  //! Return coloring edges presentation flag
  Standard_Boolean IsColoringEdge() const { return myIsColoringEdge; }

private:

  OpenGl_Vec4        myViewport;       //!< viewport properties
  OpenGl_Vec3        myWireframeColor; //!< wireframe color
  Standard_ShortReal myWireframeWidth; //!< wireframe width
  Standard_Boolean   myIsQuadMode;     //!< quad presentation flag
  Standard_Boolean   myIsColoringEdge; //!< coloring edges presentation flag

};

#endif // _OpenGl_State_HeaderFile
