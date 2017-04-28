// Created on: 2013-09-05
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

#ifndef _OpenGl_CappingRenderer_H__
#define _OpenGl_CappingRenderer_H__

#include <OpenGl_RenderFilter.hxx>
#include <OpenGl_Vec.hxx>

// Forward declaration
class OpenGl_AspectFace;
class OpenGl_AspectFillCapping;
class OpenGl_AspectSection;
class OpenGl_CappingStyle;
class OpenGl_ClippingIterator;
class OpenGl_Context;
class OpenGl_Element;
class OpenGl_PrimitiveArray;
class OpenGl_Structure;
class OpenGl_Texture;
class OpenGl_Workspace;

//! Class implementing an OpenGL rasterization algorithm for drawing capping planes over clipped geometry.
class OpenGl_CappingRenderer
{
public:

  //! Renders capping surfaces for enabled clipping planes.
  //! @note Depth buffer must be generated for the passed groups.
  //! @param theWorkspace [in] the GL workspace, context state
  //! @param theStructure [in] the structure to be capped
  Standard_EXPORT static void Render (const Handle(OpenGl_Workspace)& theWorkspace,
                                      const OpenGl_Structure&         theStructure);

private:

  //! Method to render capping section of one given plane for a structure.
  static void renderOne (const Handle(OpenGl_Workspace)&    theWorkspace,
                         const OpenGl_Structure&            theStructure,
                         const OpenGl_ClippingIterator&     thePlaneIter,
                         const OpenGl_AspectFillCapping*    thePlaneAspectCapping,
                         const OpenGl_PrimitiveArray*       theQuad);

  //! Render section plane using the given aspects.
  static void renderSection (const Handle(OpenGl_Workspace)&    theWorkspace,
                             const OpenGl_PrimitiveArray*       theQuad,
                             const OpenGl_AspectFace*           theCappingAspect,
                             const OpenGl_AspectFace*           theHatchAspect,
                             const OpenGl_Mat4&                 theCappingMatrix,
                             const Standard_ShortReal           theHatchScale,
                             const Standard_ShortReal           theHatchRotate);

  //! Initializes and returns face draw element for rendering section plane.
  //! @note once initialized, context will hold resource until destroyed.
  static OpenGl_PrimitiveArray* initQuad (const Handle(OpenGl_Context)& theContext);

  //! Set rotation persistence components for the matrix to support persistent texturing.
  //! @param theWorkspace [in] the GL workspace, context state.
  //! @param thePlane [in] the plane.
  //! @param theRotateScaleMat [in/out] the matrix combining rotation and scaling persistence components.
  static void computePersistence (const Handle(OpenGl_Workspace)& theWorkspace,
                                  const OpenGl_ClippingIterator&  thePlane,
                                  const Handle(OpenGl_Texture)&   theTexture,
                                  const bool                      theRotatePers,
                                  const bool                      theZoomPers,
                                  OpenGl_Mat4&                    theRotateScaleMat);

  //! Set zoom persistence components for the matrix to support persistent texturing.
  //! @param theWorkspace [in] the GL workspace, context state.
  //! @param thePlane [in] the plane.
  //! @param theTexture [in] the texture.
  //! @param theRotateScaleMat [in/out] the matrix combining rotation and scaling persistence components.
  static void setZoomPersistence (const Handle(OpenGl_Workspace)& theWorkspace,
                                  const Handle(OpenGl_Texture)&   theTexture,
                                  OpenGl_Mat4&                    theRotateScaleMat);
};

//! Filters out everything except face primitives of solids.
class OpenGl_CappingRenderFilter : public OpenGl_RenderFilter
{
public:

  //! Default constructor.
  OpenGl_CappingRenderFilter() {}

  //! Sets the current active filter in workspace.
  //! @param thePrevFilter [in] the previously active filter that should have additive effect.
  void SetPreviousFilter (const Handle(OpenGl_RenderFilter)& thePrevFitler) { myFilter = thePrevFitler; }

  //! Checks whether the element can be rendered or not.
  //! @param theElement [in] the element to check.
  //! @return True if element can be rendered.
  Standard_EXPORT virtual
    Standard_Boolean ShouldRender (const Handle(OpenGl_Workspace)& theWorkspace,
                                   const OpenGl_Element*           theGlElement) Standard_OVERRIDE;

private:

  Handle(OpenGl_RenderFilter) myFilter; //!< Previous active filter that should be combined.

public:

  DEFINE_STANDARD_RTTIEXT (OpenGl_CappingRenderFilter, OpenGl_RenderFilter)
};

DEFINE_STANDARD_HANDLE (OpenGl_CappingRenderFilter, OpenGl_RenderFilter)

#endif
