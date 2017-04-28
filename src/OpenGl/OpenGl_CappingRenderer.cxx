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

#include <OpenGl_CappingRenderer.hxx>

#include <OpenGl_AspectFace.hxx>
#include <OpenGl_AspectFillCapping.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_Element.hxx>
#include <OpenGl_PrimitiveArray.hxx>
#include <OpenGl_Resource.hxx>
#include <OpenGl_ShaderManager.hxx>
#include <OpenGl_Structure.hxx>
#include <OpenGl_View.hxx>
#include <OpenGl_Workspace.hxx>

#include <Graphic3d_Camera.hxx>
#include <NCollection_AlignedAllocator.hxx>

IMPLEMENT_STANDARD_RTTIEXT(OpenGl_CappingRenderFilter, OpenGl_RenderFilter)

namespace
{
  //! interleaved (4x position, 4x normals, 4x (u,v) coordinates).
  //!              o ----o
  //! representing | \ / |
  //! these        |  o  |
  //! triangles:   | / \ |
  //!              o --- o
  static const GLfloat THE_CAPPING_PLN_VERTS[12 * (4 + 4 + 4)] =
  {
    0.0f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,   0.0f,-1.0f, 0.0f, 0.0f,

    0.0f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,   0.0f,-1.0f, 0.0f, 0.0f,
   -1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f, 0.0f,

    0.0f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f,
   -1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f,-1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,

    0.0f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f,-1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, 0.0f
  };

  class OpenGl_SharedElement : public OpenGl_Resource
  {
  public:
    OpenGl_SharedElement (OpenGl_Element* theGlElement) : myGlElement (theGlElement) {}
    virtual void Release (OpenGl_Context* theGlCtx) Standard_OVERRIDE
    {
      OpenGl_Element::Destroy (theGlCtx, myGlElement);
    }
    OpenGl_Element* GlElement() const { return myGlElement; }

  private:
    OpenGl_Element* myGlElement;

  public:

    DEFINE_STANDARD_RTTI_INLINE (OpenGl_SharedElement, OpenGl_Resource)
  };


  static const TCollection_AsciiString  THE_QUAD_PARRAY    = "OpenGl_CappingRenderer_Quad";
  static const TCollection_AsciiString  THE_PLANE_STYLE    = "OpenGl_CappingRenderer_CappingStyle_";
  static const OpenGl_AspectFillCapping THE_DEFAULT_ASPECT = OpenGl_AspectFillCapping (new Graphic3d_AspectFillCapping);
}

// =======================================================================
// function : Render
// purpose  : Renders capping surfaces for enabled clipping planes.
// =======================================================================
void OpenGl_CappingRenderer::Render (const Handle(OpenGl_Workspace)& theWorkspace,
                                     const OpenGl_Structure&         theStructure)
{
  const Handle(OpenGl_Context)& aContext = theWorkspace->GetGlContext();
  if (!aContext->Clipping().IsCappingOn())
  {
    return;
  }

  const OpenGl_PrimitiveArray* aCappingQuad = initQuad (aContext);
  if (!aCappingQuad)
  {
    return;
  }

  const OpenGl_AspectFace*           aPrevAspect    = theWorkspace->AspectFace();
  Handle(OpenGl_RenderFilter)        aPrevFilter    = theWorkspace->GetRenderFilter();
  Handle(OpenGl_CappingRenderFilter) aCappingFilter = theWorkspace->CappingRenderFilter();
  aCappingFilter->SetPreviousFilter (aPrevFilter);

  theWorkspace->SetRenderFilter (aCappingFilter);

  GLint aDepthFuncPrev;
  glGetIntegerv (GL_DEPTH_FUNC, &aDepthFuncPrev);
  glDepthFunc (GL_LESS);
  glEnable (GL_STENCIL_TEST);

  GLboolean aPrevBlend = glIsEnabled (GL_BLEND);
  GLint     aPrevBlendSrc = GL_ONE;
  GLint     aPrevBlendDst = GL_ZERO;
  if (aPrevBlend == GL_TRUE)
  {
    glGetIntegerv (GL_BLEND_SRC_ALPHA, &aPrevBlendSrc);
    glGetIntegerv (GL_BLEND_DST_ALPHA, &aPrevBlendDst);
    glDisable (GL_BLEND);
  }

  for (OpenGl_ClippingIterator aCappingIt (aContext->Clipping()); aCappingIt.More(); aCappingIt.Next())
  {
    const Handle(Graphic3d_ClipPlane)& aPlane = aCappingIt.Value();
    if (!aPlane->IsCapping()
      || aCappingIt.IsDisabled())
    {
      continue;
    }

    Handle(OpenGl_AspectFillCapping) anAspect;

    TCollection_AsciiString aSharedStyleId;
    if (!aPlane->CappingSectionStyle().IsNull())
    {
      aSharedStyleId = THE_PLANE_STYLE + aPlane->GetId();

      if (!aContext->GetResource (aSharedStyleId, anAspect))
      {
        anAspect     = new OpenGl_AspectFillCapping  (aPlane->CappingSectionStyle());
        aContext->ShareResource (aSharedStyleId, anAspect);
      }
    }

    renderOne (theWorkspace, theStructure, aCappingIt, anAspect.get(), aCappingQuad);

    if (!aSharedStyleId.IsEmpty())
    {
      // schedule release of resource if not used
      aContext->ReleaseResource (aSharedStyleId, Standard_True);
    }
  }

  glClear (GL_STENCIL_BUFFER_BIT);
  glDepthFunc (aDepthFuncPrev);
  glStencilFunc (GL_ALWAYS, 0, 0xFF);
  glDisable (GL_STENCIL_TEST);

  if (aPrevBlend == GL_TRUE)
  {
    glEnable (GL_BLEND);
    glBlendFunc (aPrevBlendSrc, aPrevBlendDst);
  }

  theWorkspace->SetRenderFilter (aPrevFilter);
  theWorkspace->SetAspectFace   (aPrevAspect);
}

// =======================================================================
// function : renderOne
// purpose  : method to render capping section of one given plane for a structure.
// =======================================================================
void OpenGl_CappingRenderer::renderOne (const Handle(OpenGl_Workspace)&    theWorkspace,
                                        const OpenGl_Structure&            theStructure,
                                        const OpenGl_ClippingIterator&     thePlaneIter,
                                        const OpenGl_AspectFillCapping*    thePlaneAspectCapping,
                                        const OpenGl_PrimitiveArray*       theQuad)
{
  const Handle(Graphic3d_ClipPlane)& aPlane = thePlaneIter.Value();
  const Handle(OpenGl_Context)& aContext    = theWorkspace->GetGlContext();
  const Handle(Graphic3d_Camera) aCamera    = theWorkspace->View() != NULL
                                            ? theWorkspace->View()->Camera()
                                            : Handle(Graphic3d_Camera)();

  const OpenGl_Mat4& aPlaneMat    = OpenGl_Mat4::Map (aPlane->OrientationMatrix());
  Standard_ShortReal aRotateAngle = 0.0;
  Standard_ShortReal aViewScale   = ShortRealLast();
  OpenGl_Mat4        aRotateZoomMat;

  for (OpenGl_Structure::GroupIterator aGroupIter (theStructure.Groups()); aGroupIter.More(); aGroupIter.Next())
  {
    if (!aGroupIter.Value()->IsClosed())
    {
      continue;
    }

    // enable only the rendering plane to generate stencil mask
    aContext->ChangeClipping().DisableAllExcept (aContext, thePlaneIter);
    aContext->ShaderManager()->UpdateClippingState();

    glClear (GL_STENCIL_BUFFER_BIT);
    glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // override aspects, disable culling
    theWorkspace->SetAspectFace (&theWorkspace->NoneCulling());
    theWorkspace->ApplyAspectFace();

    // evaluate number of pair faces
    if (theWorkspace->UseZBuffer())
    {
      glDisable (GL_DEPTH_TEST);
    }
    if (theWorkspace->UseDepthWrite())
    {
      glDepthMask (GL_FALSE);
    }
    glStencilFunc (GL_ALWAYS, 1, 0x01);
    glStencilOp (GL_KEEP, GL_INVERT, GL_INVERT);

    aGroupIter.Value()->Render (theWorkspace);

    // override material, cull back faces
    theWorkspace->SetAspectFace (&theWorkspace->FrontCulling());
    theWorkspace->ApplyAspectFace();

    // enable all clip plane except the rendered one
    aContext->ChangeClipping().EnableAllExcept (aContext, thePlaneIter);
    aContext->ShaderManager()->UpdateClippingState();

    // render capping plane using the generated stencil mask
    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    if (theWorkspace->UseDepthWrite())
    {
      glDepthMask (GL_TRUE);
    }
    glStencilFunc (GL_EQUAL, 1, 0x01);
    glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
    if (theWorkspace->UseZBuffer())
    {
      glEnable (GL_DEPTH_TEST);
    }

    const OpenGl_AspectFace*        aGroupAspectFace    = aGroupIter.Value()->AspectFace();
    const OpenGl_AspectFillCapping* aGroupAspectCapping = aGroupIter.Value()->AspectFillCapping();
    const OpenGl_AspectFillCapping* anAspectCapping =
        thePlaneAspectCapping && (!aGroupAspectCapping || aGroupAspectCapping->Aspect().IsNull() || aPlane->ToOverrideCappingAspect())
      ? thePlaneAspectCapping
      : aGroupAspectCapping;

    if (anAspectCapping == NULL)
    {
      anAspectCapping = &THE_DEFAULT_ASPECT;
    }

    const OpenGl_AspectFace* anAspectFace     = anAspectCapping->CappingFaceAspect (aGroupAspectFace);
    const Standard_Boolean   hasHatch         = anAspectCapping->Aspect()->ToDrawHatch();
    const OpenGl_AspectFace* anAspectHatching = hasHatch ? anAspectCapping->HatchingFaceAspect() : NULL;
    const Standard_Boolean   hasTextureHatch  = hasHatch && !anAspectCapping->Aspect()->TextureHatch().IsNull();
    const Standard_Boolean   isRotatePers     = hasTextureHatch && !aCamera.IsNull() && anAspectCapping->Aspect()->IsHatchRotationPersistent();
    const Standard_Boolean   isZoomPers       = hasTextureHatch && !aCamera.IsNull() && anAspectCapping->Aspect()->IsHatchZoomPersistent();

    Standard_ShortReal aHatchScale = 1.0;
    Standard_ShortReal aHatchAngle = 0.0;

    if (isRotatePers || isZoomPers)
    {

      if (isRotatePers)
      {
        if (aRotateAngle == 0.0)
        {
          const gp_Dir aPlaneSide (aPlaneMat.GetValue (0, 0), aPlaneMat.GetValue (1, 0), aPlaneMat.GetValue (2, 0));
          const gp_Dir aPlaneUp   (aPlaneMat.GetValue (0, 2), aPlaneMat.GetValue (1, 2), aPlaneMat.GetValue (2, 2));
          const gp_Dir& aCameraUp  = aCamera->Up();
          const gp_Vec  aCameraPln = aPlaneSide.Dot (aCameraUp) * aPlaneSide + aPlaneUp.Dot (aCameraUp) * aPlaneUp;
          const gp_Dir& aCameraDir   = aCamera->Direction();
          aRotateAngle = static_cast<Standard_ShortReal> (aCameraPln.AngleWithRef (aPlaneUp, aCameraDir) / M_PI * 180.0);
        }

        aHatchAngle = aRotateAngle;
      }

      if (isZoomPers)
      {
        if (aViewScale == ShortRealLast())
        {
          const Standard_Real aFocus = aCamera->IsOrthographic()
                                      ? aCamera->Distance()
                                      : (aCamera->ZFocusType() == Graphic3d_Camera::FocusType_Relative
                                      ? Standard_Real(aCamera->ZFocus() * aCamera->Distance())
                                      : Standard_Real(aCamera->ZFocus()));

          const gp_XYZ aViewDim = aCamera->ViewDimensions (aFocus);
          aViewScale = static_cast<Standard_ShortReal> (aViewDim.Y() / aContext->Viewport()[3]);
        }

        aHatchScale = 1.0f / (aViewScale * anAspectHatching->TextureRes (aContext)->SizeY());
      }
    }

    renderSection (theWorkspace, theQuad, anAspectFace, hasHatch ? anAspectCapping->HatchingFaceAspect() : NULL, aPlaneMat, aHatchScale, aHatchAngle);

    // turn on the current plane to restore initial state
    aContext->ChangeClipping().SetEnabled (aContext, thePlaneIter, Standard_True);
    aContext->ShaderManager()->RevertClippingState();
    aContext->ShaderManager()->RevertClippingState();
  }

  if (theStructure.InstancedStructure() != NULL)
  {
    renderOne (theWorkspace, *theStructure.InstancedStructure(), thePlaneIter, thePlaneAspectCapping, theQuad);
  }
}

// =======================================================================
// function : renderSection
// purpose  : 
// =======================================================================
void OpenGl_CappingRenderer::renderSection (const Handle(OpenGl_Workspace)& theWorkspace,
                                            const OpenGl_PrimitiveArray*    theQuad,
                                            const OpenGl_AspectFace*        theCappingAspect,
                                            const OpenGl_AspectFace*        theHatchAspect,
                                            const OpenGl_Mat4&              theCappingMatrix,
                                            const Standard_ShortReal        theHatchScale,
                                            const Standard_ShortReal        theHatchRotate)
{
  const Handle(OpenGl_Context)& aContext = theWorkspace->GetGlContext();

  const Standard_Boolean isTextureHatch =
      theHatchAspect != NULL
  &&  theHatchAspect->Aspect()->TextureMapState();

  aContext->ModelWorldState.Push();
  aContext->ModelWorldState.SetCurrent (theCappingMatrix);
  aContext->ApplyModelViewMatrix();

  theWorkspace->SetAspectFace (theCappingAspect);
  theWorkspace->ApplyAspectFace();

  theQuad->Render (theWorkspace);

  if (theHatchAspect != NULL)
  {
    Graphic3d_Vec2     aPrevScale;
    Standard_ShortReal aPrevRotate = 0.0;

    if (isTextureHatch)
    {
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      if (theHatchScale != 1.0 || theHatchRotate != 0.0)
      {
        const Handle(Graphic3d_TextureParams)& aTexParams = theHatchAspect->TextureParams();

        aPrevScale  = aTexParams->Scale();
        aPrevRotate = aTexParams->Rotation();

        const Standard_Boolean   isMirror = aPrevScale.x() * aPrevScale.y() < 0.0;
        aTexParams->SetScale    (aPrevScale * theHatchScale);
        aTexParams->SetRotation (isMirror ? aPrevRotate - theHatchRotate : aPrevRotate + theHatchRotate);
      }
    }

    theWorkspace->SetAspectFace (theHatchAspect);
    theWorkspace->ApplyAspectFace();

    glDepthFunc (GL_LEQUAL);

    theQuad->Render (theWorkspace);

    glDepthFunc (GL_LESS);

    if (isTextureHatch)
    {
      glDisable (GL_BLEND);

      if (theHatchScale != 1.0 || theHatchRotate != 0.0)
      {
        const Handle(Graphic3d_TextureParams)& aTexParams = theHatchAspect->TextureParams();

        aTexParams->SetScale (aPrevScale);
        aTexParams->SetRotation (aPrevRotate);
      }
    }
  }

  aContext->ModelWorldState.Pop();
  aContext->ApplyModelViewMatrix();
}

// =======================================================================
// function : initQuad
// purpose  : initializes and returns vertex buffer for plane section
// =======================================================================
OpenGl_PrimitiveArray* OpenGl_CappingRenderer::initQuad (const Handle(OpenGl_Context)& theContext)
{
  Handle(OpenGl_SharedElement) aSharedResource;

  if (!theContext->GetResource (THE_QUAD_PARRAY, aSharedResource))
  {
    OpenGl_PrimitiveArray* aPArray = NULL;

    Handle(NCollection_AlignedAllocator) anAlloc = new NCollection_AlignedAllocator (16);
    Handle(Graphic3d_Buffer) anAttribs = new Graphic3d_Buffer (anAlloc);
    Graphic3d_Attribute anAttribInfo[] =
    {
      { Graphic3d_TOA_POS,  Graphic3d_TOD_VEC4 },
      { Graphic3d_TOA_NORM, Graphic3d_TOD_VEC4 },
      { Graphic3d_TOA_UV,   Graphic3d_TOD_VEC4 }
    };
    if (anAttribs->Init (12, anAttribInfo, 3))
    {
      memcpy (anAttribs->ChangeData(), THE_CAPPING_PLN_VERTS, sizeof (THE_CAPPING_PLN_VERTS));

      aPArray = new OpenGl_PrimitiveArray (NULL);
      aPArray->InitBuffers (NULL, Graphic3d_TOPA_TRIANGLES, NULL, anAttribs, NULL);
    }
    aSharedResource = new OpenGl_SharedElement (aPArray);

    theContext->ShareResource (THE_QUAD_PARRAY, aSharedResource);
  }

  return dynamic_cast<OpenGl_PrimitiveArray*> (aSharedResource->GlElement());
}

// =======================================================================
// function : CanRender
// purpose  :
// =======================================================================
Standard_Boolean OpenGl_CappingRenderFilter::ShouldRender (const Handle(OpenGl_Workspace)& theWorkspace,
                                                           const OpenGl_Element* theGlElement)
{
  if (!myFilter.IsNull() && !myFilter->ShouldRender (theWorkspace, theGlElement))
  {
    return Standard_False;
  }

  const OpenGl_PrimitiveArray* aPArray = dynamic_cast<const OpenGl_PrimitiveArray*> (theGlElement);
  return aPArray != NULL
      && aPArray->DrawMode() >= OpenGl_PrimitiveArray::THE_FILLPRIM_FROM
      && aPArray->DrawMode() <= OpenGl_PrimitiveArray::THE_FILLPRIM_TO;
}
