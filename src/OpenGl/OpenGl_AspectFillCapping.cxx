// Created on: 2017-04-14
// Created by: Anton POLETAEV
// Copyright (c) 2017 OPEN CASCADE SAS
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

#include <OpenGl_AspectFillCapping.hxx>

#include <OpenGl_AspectFace.hxx>
#include <OpenGl_Workspace.hxx>

namespace
{
  Handle(Graphic3d_AspectFillArea3d) defaultMaterial()
  {
    Handle(Graphic3d_AspectFillArea3d) anAspect;
    const Graphic3d_MaterialAspect aMaterial (Graphic3d_NOM_DEFAULT);
    anAspect = new Graphic3d_AspectFillArea3d();
    anAspect->SetDistinguishOff();
    anAspect->SetFrontMaterial (aMaterial);
    anAspect->SetInteriorStyle (Aspect_IS_SOLID);
    anAspect->SetInteriorColor (aMaterial.Color());
    anAspect->SetSuppressBackFaces (false);
    return anAspect;
  }
}

// =======================================================================
// function : OpenGl_AspectFillCapping
// purpose  :
// =======================================================================
OpenGl_AspectFillCapping::OpenGl_AspectFillCapping (const Handle(Graphic3d_AspectFillCapping)& theAspect)
: myCappingAspect  (defaultMaterial()),
  myHatchingAspect (defaultMaterial()),
  myHatchingState (0)
{
  SetAspect (theAspect);
}

// =======================================================================
// function : ~OpenGl_AspectFillCapping
// purpose  :
// =======================================================================
OpenGl_AspectFillCapping::~OpenGl_AspectFillCapping()
{
  //
}

// =======================================================================
// function : SetAspect
// purpose  :
// =======================================================================
void OpenGl_AspectFillCapping::SetAspect (const Handle(Graphic3d_AspectFillCapping)& theAspect)
{
  myAspect = theAspect;

  if (theAspect.IsNull())
  {
    return;
  }

  if (!theAspect->ToUseObjectMaterial()
   || !theAspect->ToUseObjectTexture()
   || !theAspect->ToUseObjectShader())
  {
    Handle(Graphic3d_AspectFillArea3d) aFillAspect = myCappingAspect.Aspect();

    if (!theAspect->ToUseObjectMaterial())
    {
      aFillAspect->SetFrontMaterial (theAspect->Material());
      aFillAspect->SetInteriorColor (theAspect->Material().Color());
    }

    if (!theAspect->ToUseObjectTexture())
    {
      aFillAspect->SetTextureMap (theAspect->Texture());

      if (!theAspect->Texture().IsNull())
      {
        aFillAspect->SetTextureMapOn();
      }
      else
      {
        aFillAspect->SetTextureMapOff();
      }
    }
    else
    {
      aFillAspect->SetTextureMap (Handle(Graphic3d_TextureMap)());
      aFillAspect->SetTextureMapOff();
    }

    if (!theAspect->ToUseObjectShader())
    {
      aFillAspect->SetShaderProgram (theAspect->Shader());
    }

    myCappingAspect.SetAspect (aFillAspect);
  }

  if (theAspect->ToDrawHatch()
    && (theAspect->IsTextureHatch()
     || theAspect->IsStippleHatch()))
  {
    Handle(Graphic3d_AspectFillArea3d) aFillAspect = myHatchingAspect.Aspect();

    aFillAspect->SetInteriorStyle (theAspect->IsStippleHatch() ? Aspect_IS_HATCH : Aspect_IS_SOLID);
    aFillAspect->SetHatchStyle    (theAspect->IsStippleHatch() ? theAspect->StippleHatch() : Handle(Graphic3d_HatchStyle)());
    aFillAspect->SetTextureMap    (theAspect->IsTextureHatch() ? theAspect->TextureHatch() : Handle(Graphic3d_TextureMap)());
    aFillAspect->SetFrontMaterial (theAspect->HatchMaterial());
    aFillAspect->SetInteriorColor (theAspect->HatchMaterial().Color());
    if (theAspect->IsTextureHatch())
    {
      aFillAspect->SetTextureMapOn();
    }
    else
    {
      aFillAspect->SetTextureMapOff();
    }

    myHatchingAspect.SetAspect (aFillAspect);
    myHatchingState = theAspect->HatchingState();
  }
}

// =======================================================================
// function : Release
// purpose  :
// =======================================================================
void OpenGl_AspectFillCapping::Release (OpenGl_Context* theContext)
{
  myCappingAspect .Release (theContext);
  myHatchingAspect.Release (theContext);
}

// =======================================================================
// function : CappingFaceAspect
// purpose  :
// =======================================================================
const OpenGl_AspectFace* OpenGl_AspectFillCapping::CappingFaceAspect (const OpenGl_AspectFace* theObjectAspect) const
{
  if (myAspect.IsNull())
  {
    return NULL;
  }

  Handle(Graphic3d_AspectFillArea3d) aFillAspect = myCappingAspect.Aspect();

  if (myAspect->ToUseObjectMaterial() && theObjectAspect != NULL)
  {
    // only front material currently supported by capping rendering
    aFillAspect->SetFrontMaterial (theObjectAspect->Aspect()->FrontMaterial());
    aFillAspect->SetInteriorColor (theObjectAspect->Aspect()->InteriorColor());
  }
  else
  {
    aFillAspect->SetFrontMaterial (myAspect->Material());
    aFillAspect->SetInteriorColor (myAspect->Material().Color());
  }

  if (myAspect->ToUseObjectTexture() && theObjectAspect != NULL)
  {
    if (theObjectAspect->Aspect()->ToMapTexture())
    {
      aFillAspect->SetTextureMap (theObjectAspect->Aspect()->TextureMap());
      aFillAspect->SetTextureMapOn();
    }
    else
    {
      aFillAspect->SetTextureMapOff();
    }
  }
  else
  {
    aFillAspect->SetTextureMap (myAspect->Texture());
    if (!myAspect->Texture().IsNull())
    {
      aFillAspect->SetTextureMapOn();
    }
    else
    {
      aFillAspect->SetTextureMapOff();
    }
  }

  if (myAspect->ToUseObjectShader() && theObjectAspect != NULL)
  {
    aFillAspect->SetShaderProgram (theObjectAspect->Aspect()->ShaderProgram());
  }
  else
  {
    aFillAspect->SetShaderProgram (myAspect->Shader());
  }

  myCappingAspect.SetAspect (aFillAspect);

  return &myCappingAspect;
}

// =======================================================================
// function : HatchingFaceAspect
// purpose  :
// =======================================================================
const OpenGl_AspectFace* OpenGl_AspectFillCapping::HatchingFaceAspect() const
{
  if (myAspect.IsNull())
  {
    return NULL;
  }

  const Standard_Size aHatchingState = myAspect->HatchingState();
  if (myHatchingState != aHatchingState)
  {
    if (myAspect->ToDrawHatch())
    {
      Handle(Graphic3d_AspectFillArea3d) aFillAspect = myHatchingAspect.Aspect();

      aFillAspect->SetInteriorStyle (myAspect->IsStippleHatch() ? Aspect_IS_HATCH : Aspect_IS_SOLID);
      aFillAspect->SetHatchStyle    (myAspect->IsStippleHatch() ? myAspect->StippleHatch() : Handle(Graphic3d_HatchStyle)());
      aFillAspect->SetTextureMap    (myAspect->IsTextureHatch() ? myAspect->TextureHatch() : Handle(Graphic3d_TextureMap)());
      aFillAspect->SetFrontMaterial (myAspect->HatchMaterial());
      aFillAspect->SetInteriorColor (myAspect->HatchMaterial().Color());
      if (myAspect->IsTextureHatch())
      {
        aFillAspect->SetTextureMapOn();
      }
      else
      {
        aFillAspect->SetTextureMapOff();
      }

      myHatchingAspect.SetAspect (aFillAspect);
      myHatchingState = aHatchingState;
    }
  }

  return &myHatchingAspect;
}