// Created on: 2011-09-20
// Created by: Sergey ZERCHANINOV
// Copyright (c) 2011-2014 OPEN CASCADE SAS
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

#include <stdio.h>
#include <stdlib.h>

#include <OpenGl_GlCore11.hxx>

#include <gp_Quaternion.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <Graphic3d_StructureManager.hxx>
#include <Graphic3d_TextureParams.hxx>
#include <Graphic3d_Texture2Dmanual.hxx>
#include <Graphic3d_TransformUtils.hxx>
#include <Image_AlienPixMap.hxx>

#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <NCollection_LocalArray.hxx>
#include <NCollection_Mat4.hxx>

#include <OpenGl_AspectLine.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameStats.hxx>
#include <OpenGl_Matrix.hxx>
#include <OpenGl_Workspace.hxx>
#include <OpenGl_View.hxx>
#include <OpenGl_GraduatedTrihedron.hxx>
#include <OpenGl_PrimitiveArray.hxx>
#include <OpenGl_ShaderManager.hxx>
#include <OpenGl_ShaderProgram.hxx>
#include <OpenGl_Structure.hxx>
#include <OpenGl_ArbFBO.hxx>

#ifdef HAVE_OPENVR
  #include <openvr.h>
#endif

namespace
{
  //! Format Frame Buffer format for logging messages.
  static TCollection_AsciiString printFboFormat (const Handle(OpenGl_FrameBuffer)& theFbo)
  {
    return TCollection_AsciiString() + theFbo->GetInitVPSizeX() + "x" + theFbo->GetInitVPSizeY() + "@" + theFbo->NbSamples();
  }

  //! Return TRUE if Frame Buffer initialized has failed with the same parameters.
  static bool checkWasFailedFbo (const Handle(OpenGl_FrameBuffer)& theFboToCheck,
                                 Standard_Integer theSizeX,
                                 Standard_Integer theSizeY,
                                 Standard_Integer theNbSamples)
  {
    return !theFboToCheck->IsValid()
        &&  theFboToCheck->GetInitVPSizeX() == theSizeX
        &&  theFboToCheck->GetInitVPSizeY() == theSizeY
        &&  theFboToCheck->NbSamples()      == theNbSamples;
  }

  //! Return TRUE if Frame Buffer initialized has failed with the same parameters.
  static bool checkWasFailedFbo (const Handle(OpenGl_FrameBuffer)& theFboToCheck,
                                 const Handle(OpenGl_FrameBuffer)& theFboRef)
  {
    return checkWasFailedFbo (theFboToCheck, theFboRef->GetVPSizeX(), theFboRef->GetVPSizeY(), theFboRef->NbSamples());
  }

#ifdef HAVE_OPENVR
  //! Retrieve string property from OpenVR.
  static TCollection_AsciiString getVrTrackedDeviceString (vr::IVRSystem* theHmd,
                                                           vr::TrackedDeviceIndex_t theDevice,
                                                           vr::TrackedDeviceProperty theProperty,
                                                           vr::TrackedPropertyError* theError = NULL)
  {
    const uint32_t aBuffLen = theHmd->GetStringTrackedDeviceProperty(theDevice, theProperty, NULL, 0, theError);
    if (aBuffLen == 0)
    {
      return TCollection_AsciiString();
    }

    NCollection_LocalArray<char> aBuffer (aBuffLen + 1);
    theHmd->GetStringTrackedDeviceProperty (theDevice, theProperty, aBuffer, aBuffLen, theError);
    aBuffer[aBuffLen] = '\0';
    const TCollection_AsciiString aResult (aBuffer);
    return aResult;
  }

  //! Print OpenVR compositor error.
  static TCollection_AsciiString getVRCompositorError (vr::EVRCompositorError theVRError)
  {
    switch (theVRError)
    {
      case vr::VRCompositorError_None:
        return "None";
      case vr::VRCompositorError_RequestFailed:
        return "Compositor Error: Request Failed";
      case vr::VRCompositorError_IncompatibleVersion:
        return "Compositor Error: Incompatible Version";
      case vr::VRCompositorError_DoNotHaveFocus:
        return "Compositor Error: Do not have focus";
      case vr::VRCompositorError_InvalidTexture:
        return "Compositor Error: Invalid Texture";
      case vr::VRCompositorError_IsNotSceneApplication:
        return "Compositor Error: Is not scene application";
      case vr::VRCompositorError_TextureIsOnWrongDevice:
        return "Compositor Error: Texture is on wrong device";
      case vr::VRCompositorError_TextureUsesUnsupportedFormat:
        return "Compositor Error: Texture uses unsupported format";
      case vr::VRCompositorError_SharedTexturesNotSupported:
        return "Compositor Error: Shared textures not supported";
      case vr::VRCompositorError_IndexOutOfRange:
        return "Compositor Error: Index out of range";
      case vr::VRCompositorError_AlreadySubmitted:
        return "Compositor Error: Already submitted";
    }
    return TCollection_AsciiString ("Compositor Error: UNKNOWN #") + int(theVRError);
  }

  //! Define transformation based on OpenVR transformation matrix
  //! and camera definition as reference coordinate system.
  static gp_Trsf trsfVr2Occ (const Graphic3d_Camera&  theCamera,
                             const vr::HmdMatrix34_t& theTrsf)
  {
    gp_Trsf aTrsfVr;
    aTrsfVr.SetValues (theTrsf.m[0][0], theTrsf.m[0][1], theTrsf.m[0][2], theTrsf.m[0][3],
                       theTrsf.m[1][0], theTrsf.m[1][1], theTrsf.m[1][2], theTrsf.m[1][3],
                       theTrsf.m[2][0], theTrsf.m[2][1], theTrsf.m[2][2], theTrsf.m[2][3]);

    gp_Trsf aTrsfVr2Occ, aTrsfOcc2Vr;
    const gp_Dir aBaseDX = gp_Vec (theCamera.Direction()) ^ gp_Vec (theCamera.OrthogonalizedUp());
    aTrsfOcc2Vr.SetTransformation (gp_Ax3 (gp::Origin(), theCamera.Direction(), aBaseDX),
                                   gp_Ax3 (gp::Origin(), -gp::DZ(), gp::DX()));
    aTrsfVr2Occ = aTrsfOcc2Vr.Inverted();

    return aTrsfOcc2Vr * aTrsfVr * aTrsfVr2Occ;
  }
#endif
}

//=======================================================================
//function : drawBackground
//purpose  :
//=======================================================================
void OpenGl_View::drawBackground (const Handle(OpenGl_Workspace)& theWorkspace)
{
  const Handle(OpenGl_Context)& aCtx = theWorkspace->GetGlContext();

  if (!myBgTextureArray->IsDefined()   // no texture
   && !myBgGradientArray->IsDefined()) // no gradient
  {
    return;
  }

  const Standard_Boolean wasUsedZBuffer = theWorkspace->SetUseZBuffer (Standard_False);
  if (wasUsedZBuffer)
  {
    aCtx->core11fwd->glDisable (GL_DEPTH_TEST);
  }

  // Drawing background gradient if:
  // - gradient fill type is not Aspect_GFM_NONE and
  // - either background texture is no specified or it is drawn in Aspect_FM_CENTERED mode
  if (myBgGradientArray->IsDefined()
    && (!myTextureParams->Aspect()->ToMapTexture()
      || myBgTextureArray->TextureFillMethod() == Aspect_FM_CENTERED
      || myBgTextureArray->TextureFillMethod() == Aspect_FM_NONE))
  {
  #if !defined(GL_ES_VERSION_2_0)
    GLint aShadingModelOld = GL_SMOOTH;
    if (aCtx->core11 != NULL
     && aCtx->caps->ffpEnable)
    {
      aCtx->core11fwd->glDisable (GL_LIGHTING);
      aCtx->core11fwd->glGetIntegerv (GL_SHADE_MODEL, &aShadingModelOld);
      aCtx->core11->glShadeModel (GL_SMOOTH);
    }
  #endif

    myBgGradientArray->Render (theWorkspace);

  #if !defined(GL_ES_VERSION_2_0)
    if (aCtx->core11 != NULL
     && aCtx->caps->ffpEnable)
    {
      aCtx->core11->glShadeModel (aShadingModelOld);
    }
  #endif
  }

  // Drawing background image if it is defined
  // (texture is defined and fill type is not Aspect_FM_NONE)
  if (myBgTextureArray->IsDefined()
   && myTextureParams->Aspect()->ToMapTexture())
  {
    aCtx->core11fwd->glDisable (GL_BLEND);

    const OpenGl_AspectFace* anOldAspectFace = theWorkspace->SetAspectFace (myTextureParams);
    myBgTextureArray->Render (theWorkspace);
    theWorkspace->SetAspectFace (anOldAspectFace);
  }

  if (wasUsedZBuffer)
  {
    theWorkspace->SetUseZBuffer (Standard_True);
    aCtx->core11fwd->glEnable (GL_DEPTH_TEST);
  }
}

// =======================================================================
// function : releaseOpenVR
// purpose  :
// =======================================================================
void OpenGl_View::releaseOpenVR()
{
  myAboutVrDevice.Clear();
#ifdef HAVE_OPENVR
  if (myVrHmd != NULL)
  {
    vr::VR_Shutdown();
    myVrHmd = NULL;
  }
#endif
}

// =======================================================================
// function : initOpenVR
// purpose  :
// =======================================================================
bool OpenGl_View::initOpenVR()
{
  if (myVrHmd != NULL)
  {
    return true;
  }

#ifdef HAVE_OPENVR
  vr::EVRInitError aVrError = vr::VRInitError_None;
  myVrHmd = vr::VR_Init (&aVrError, vr::VRApplication_Scene);
  if (aVrError != vr::VRInitError_None)
  {
    myVrHmd = NULL;
    Message::DefaultMessenger()->Send (TCollection_AsciiString ("OpenVR, Unable to init VR runtime: ") + vr::VR_GetVRInitErrorAsEnglishDescription (aVrError),
                                       Message_Fail);
    releaseOpenVR();
    return false;
  }

  /*vr::IVRRenderModels* aRenderModels = (vr::IVRRenderModels* )vr::VR_GetGenericInterface (vr::IVRRenderModels_Version, &aVrError);
  if (aRenderModels == NULL)
  {
    Message::DefaultMessenger()->Send (TCollection_AsciiString ("Unable to get render model interface: ") + vr::VR_GetVRInitErrorAsEnglishDescription (aVrError),
                                       Message_Fail);;
  }*/

  const TCollection_AsciiString aVrManuf   = getVrTrackedDeviceString (myVrHmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ManufacturerName_String);
  const TCollection_AsciiString aVrDriver  = getVrTrackedDeviceString (myVrHmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
  const TCollection_AsciiString aVrDisplay = getVrTrackedDeviceString (myVrHmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

  {
    uint32_t aRenderSizeX = 0;
    uint32_t aRenderSizeY = 0;
    myVrHmd->GetRecommendedRenderTargetSize (&aRenderSizeX, &aRenderSizeY);
    myVrRendSizeX = int(aRenderSizeX);
    myVrRendSizeY = int(aRenderSizeY);
    if (myVrRendSizeX <= 0
     || myVrRendSizeY <= 0)
    {
      releaseOpenVR();
      return false;
    }
  }
  myAboutVrDevice = aVrManuf + " " + aVrDriver + "\n"
                  + aVrDisplay + " [" + myVrRendSizeX + "x" + myVrRendSizeY + "]";
#endif
  return myVrHmd != NULL;
}

// =======================================================================
// function : processOpenVrEvents
// purpose  :
// =======================================================================
void OpenGl_View::processOpenVrEvents()
{
  if (myVrHmd == NULL)
  {
    return;
  }

#ifdef HAVE_OPENVR
  // process OpenVR events
  for (vr::VREvent_t aVREvent; myVrHmd->PollNextEvent(&aVREvent, sizeof(aVREvent));)
  {
    switch (aVREvent.eventType)
    {
      case vr::VREvent_TrackedDeviceActivated:
      {
        Message::DefaultMessenger()->Send (TCollection_AsciiString ("OpenVR, Device ") + (int )aVREvent.trackedDeviceIndex + " attached", Message_Trace);
        break;
      }
      case vr::VREvent_TrackedDeviceDeactivated:
      {
        Message::DefaultMessenger()->Send (TCollection_AsciiString ("OpenVR, Device ") + (int )aVREvent.trackedDeviceIndex + " detached", Message_Trace);
        break;
      }
      case vr::VREvent_TrackedDeviceUpdated:
      {
        Message::DefaultMessenger()->Send (TCollection_AsciiString ("OpenVR, Device ") + (int )aVREvent.trackedDeviceIndex + " updated", Message_Trace);
        break;
      }
    }
  }

  // process OpenVR controller state
  for (vr::TrackedDeviceIndex_t aDevIter = 0; aDevIter < vr::k_unMaxTrackedDeviceCount; ++aDevIter)
  {
    vr::VRControllerState_t aCtrlState;
    if (myVrHmd->GetControllerState (aDevIter, &aCtrlState, sizeof(aCtrlState)))
    {
      // aCtrlState.ulButtonPressed == 0;
    }
  }
#endif
}

//=======================================================================
//function : ProcessInput
//purpose  :
//=======================================================================
void OpenGl_View::ProcessInput()
{
double aZNearOld = myEffectiveCamera->ZNear(); ///
double aZFarOld = myEffectiveCamera->ZFar(); ///
  myEffectiveCamera->Copy (myCameraBase);
  if (myVrHmd == NULL)
  {
    return;
  }
  processOpenVrEvents();

#ifdef HAVE_OPENVR
  const vr::EVRCompositorError aVRError = vr::VRCompositor()->WaitGetPoses (myVrTrackedPoses, vr::k_unMaxTrackedDeviceCount, NULL, 0);
  if (aVRError != vr::VRCompositorError_None)
  {
    Message::DefaultMessenger()->Send (getVRCompositorError (aVRError), Message_Trace);
  }

  if (myVrTrackedPoses[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
  {
    myVrHeadOrient = trsfVr2Occ (*myCameraBase, myVrTrackedPoses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking);
  }
#endif
  Graphic3d_Camera::FrustumLRBT<float> aFrustL, aFrustR;
  myVrHmd->GetProjectionRaw (vr::Eye_Left,  &aFrustL.Left, &aFrustL.Right, &aFrustL.Top, &aFrustL.Bottom);
  myVrHmd->GetProjectionRaw (vr::Eye_Right, &aFrustR.Left, &aFrustR.Right, &aFrustR.Top, &aFrustR.Bottom);

  vr::HmdMatrix34_t aLeftToHead  = myVrHmd->GetEyeToHeadTransform (vr::Eye_Left);
  vr::HmdMatrix34_t aRightToHead = myVrHmd->GetEyeToHeadTransform (vr::Eye_Right);

  vr::HmdMatrix44_t aMatL = myVrHmd->GetProjectionMatrix (vr::Eye_Left,  aZNearOld, aZFarOld);
  vr::HmdMatrix44_t aMatR = myVrHmd->GetProjectionMatrix (vr::Eye_Right, aZNearOld, aZFarOld);

Graphic3d_Vec2d aTanHalfFov (Graphic3d_Vec4(-aFrustL.Left, aFrustL.Right,  -aFrustR.Left, aFrustR.Right).maxComp(),
                             Graphic3d_Vec4(-aFrustL.Top,  aFrustL.Bottom, -aFrustR.Top,  aFrustR.Bottom).maxComp());
Standard_Real anAspect = aTanHalfFov.x() / aTanHalfFov.y();
double anIod = aRightToHead.m[0][3] - aLeftToHead.m[0][3];
double aFieldOfView = 2.0 * ATan(aTanHalfFov.y()) * 180.0 / M_PI;
///double aZFocus = anIod * myEffectiveCamera->ZNear() / (aFrustL.Left - aFrustL.Right);
///double aZFocus = anIod / (aFrustL.Left - aFrustL.Right);
double aZFocus = -anIod / (aFrustL.Left - aTanHalfFov.y());

/*std::cerr << " L: " << aLeftToHead.m[0][0] << " " << aLeftToHead.m[0][1] << " " << aLeftToHead.m[0][2] << " " << aLeftToHead.m[0][3] << "\n"
          << "    " << aLeftToHead.m[1][0] << " " << aLeftToHead.m[1][1] << " " << aLeftToHead.m[1][2] << " " << aLeftToHead.m[1][3] << "\n"
          << "    " << aLeftToHead.m[2][0] << " " << aLeftToHead.m[2][1] << " " << aLeftToHead.m[2][2] << " " << aLeftToHead.m[2][3] << "\n"; ///
std::cerr << " R: " << aRightToHead.m[0][0] << " " << aRightToHead.m[0][1] << " " << aRightToHead.m[0][2] << " " << aRightToHead.m[0][3] << "\n"
          << "    " << aRightToHead.m[1][0] << " " << aRightToHead.m[1][1] << " " << aRightToHead.m[1][2] << " " << aRightToHead.m[1][3] << "\n"
          << "    " << aRightToHead.m[2][0] << " " << aRightToHead.m[2][1] << " " << aRightToHead.m[2][2] << " " << aRightToHead.m[2][3] << "\n"; ///
          */

///std::cerr << " Mat: " << aMatL.m[0][3] << " " << aMatL.m[1][3] << " " << aMatL.m[2][3] << " " << aMatL.m[3][3] << "\n"; ///


/*std::cerr << " FOV: " << aFieldOfView << "; "
        << "Aspect: " << anAspect << " (" << (static_cast<Standard_Real> (myVrRendSizeX) / static_cast<Standard_Real> (myVrRendSizeY)) << "); "
        << "Iod: " << anIod << "; "
        << "ZFocus: " << aZFocus << "; "
        << "L2H: " << aLeftToHead.m[0][3]  << " " << aLeftToHead.m[1][3]  << " " << aLeftToHead.m[2][3] << "; "
        << "R2H: " << aRightToHead.m[0][3] << " " << aRightToHead.m[1][3] << " " << aRightToHead.m[2][3] << "; "
        <<"\n";*/

  myEffectiveCamera->SetAspect (anAspect);
  myEffectiveCamera->SetFOVy (aFieldOfView);

gp_Dir anUpNew  = myCameraBase->OrthogonalizedUp().Transformed (myVrHeadOrient);
gp_Dir aDirNew  = myCameraBase->Direction().Transformed (myVrHeadOrient);
gp_Pnt anEyeNew = myCameraBase->Eye().XYZ() + myVrHeadOrient.TranslationPart();

myEffectiveCamera->SetUp (anUpNew);
myEffectiveCamera->SetEye (anEyeNew);
///myEffectiveCamera->SetCenter (myEffectiveCamera->Eye().XYZ() + aDirNew.XYZ() * 1.0); ///
myEffectiveCamera->SetCenter (myEffectiveCamera->Eye().XYZ() + aDirNew.XYZ() * myCameraBase->Distance()); ///

/*std::cerr << " EyeOld: " << myCameraBase->Eye().X() << " " << myCameraBase->Eye().Y() << " " << myCameraBase->Eye().Z() << "\n"
          << " EyeNew: " << anEyeNew.X() << " " << anEyeNew.Y() << " " << anEyeNew.Z() << "\n"
          << " Trs:    " << myVrHeadOrient.TranslationPart().X() << " " << myVrHeadOrient.TranslationPart().Y() << " " << myVrHeadOrient.TranslationPart().Z() << "\n";*/

  //myEffectiveCamera->SetCenter (myEffectiveCamera->Eye().Translated (myEffectiveCamera->Direction().XYZ() * 1.0));
  myEffectiveCamera->SetIOD (Graphic3d_Camera::IODType_Absolute, anIod);
  //myEffectiveCamera->SetZFocus (Graphic3d_Camera::FocusType_Absolute, aZFocus);

  Graphic3d_Camera::FrustumLRBT<Standard_Real> aFrustL2, aFrustR2;
  aFrustL2.Left   = aFrustL.Left;
  aFrustL2.Right  = aFrustL.Right;
  aFrustL2.Bottom = aFrustL.Bottom;
  aFrustL2.Top    = aFrustL.Top;
  aFrustR2.Left   = aFrustR.Left;
  aFrustR2.Right  = aFrustR.Right;
  aFrustR2.Bottom = aFrustR.Bottom;
  aFrustR2.Top    = aFrustR.Top;
  std::swap (aFrustL2.Top, aFrustL2.Bottom);
  std::swap (aFrustR2.Top, aFrustR2.Bottom);
myEffectiveCamera->SetCustomFrustum (aFrustL2, aFrustR2); ///
}

//=======================================================================
//function : Redraw
//purpose  :
//=======================================================================
void OpenGl_View::Redraw()
{
  const Standard_Boolean wasDisabledMSAA = myToDisableMSAA;
  const Standard_Boolean hadFboBlit      = myHasFboBlit;
  if (myRenderParams.Method == Graphic3d_RM_RAYTRACING
  && !myCaps->vboDisable
  && !myCaps->keepArrayData)
  {
    // caps are shared across all views, thus we need to invalidate all of them
    // if (myWasRedrawnGL) { myStructureManager->SetDeviceLost(); }
    myDriver->setDeviceLost();
    myCaps->keepArrayData = Standard_True;
  }

  if (!myWorkspace->Activate())
  {
    return;
  }

  ++myFrameCounter;
  const Graphic3d_StereoMode   aStereoMode  = myRenderParams.StereoMode;
  Graphic3d_Camera::Projection aProjectType = myEffectiveCamera->ProjectionType();
  Handle(OpenGl_Context)       aCtx         = myWorkspace->GetGlContext();
  aCtx->FrameStats()->FrameStart (myWorkspace);

  // release pending GL resources
  aCtx->ReleaseDelayed();

  // fetch OpenGl context state
  aCtx->FetchState();

  OpenGl_FrameBuffer* aFrameBuffer = myFBO.operator->();
  if (aStereoMode != Graphic3d_StereoMode_OpenVR
  ||  aProjectType != Graphic3d_Camera::Projection_Stereo
  ||  aFrameBuffer != NULL
  || !initOpenVR())
  {
    releaseOpenVR();
  }

  // implicitly disable VSync when using HMD composer (can be mirrored in window for debugging)
  myWindow->SetSwapInterval (myVrHmd != NULL);
  bool toSwap = aCtx->IsRender()
            && !aCtx->caps->buffersNoSwap
            &&  aFrameBuffer == NULL
            &&  (myVrHmd == NULL || myRenderParams.ToMirrorComposer);

  Standard_Integer aSizeX = myWindow->Width();
  Standard_Integer aSizeY = myWindow->Height();
  if (aFrameBuffer != NULL)
  {
    aSizeX = aFrameBuffer->GetVPSizeX();
    aSizeY = aFrameBuffer->GetVPSizeY();
  }
  else if (myVrHmd != NULL)
  {
    aSizeX = myVrRendSizeX;
    aSizeY = myVrRendSizeY;
  }

  Standard_Integer aRendSizeX = Standard_Integer(myRenderParams.RenderResolutionScale * aSizeX + 0.5f);
  Standard_Integer aRendSizeY = Standard_Integer(myRenderParams.RenderResolutionScale * aSizeY + 0.5f);

  // determine multisampling parameters
  Standard_Integer aNbSamples = !myToDisableMSAA && aSizeX == aRendSizeX
                              ? Max (Min (myRenderParams.NbMsaaSamples, aCtx->MaxMsaaSamples()), 0)
                              : 0;
  if (aNbSamples != 0)
  {
    aNbSamples = OpenGl_Context::GetPowerOfTwo (aNbSamples, aCtx->MaxMsaaSamples());
  }

  bool toUseOit = myRenderParams.TransparencyMethod == Graphic3d_RTM_BLEND_OIT
               && checkOitCompatibility (aCtx, aNbSamples > 0);

  const bool toInitImmediateFbo = myTransientDrawToFront
                               && (!aCtx->caps->useSystemBuffer || (toUseOit && HasImmediateStructures()));
  if ( aFrameBuffer == NULL
   && !aCtx->DefaultFrameBuffer().IsNull()
   &&  aCtx->DefaultFrameBuffer()->IsValid())
  {
    aFrameBuffer = aCtx->DefaultFrameBuffer().operator->();
  }

  if (myHasFboBlit
   && (myTransientDrawToFront
    || aProjectType == Graphic3d_Camera::Projection_Stereo
    || aNbSamples != 0
    || toUseOit
    || aSizeX != aRendSizeX))
  {
    if (myMainSceneFbos[0]->GetVPSizeX() != aRendSizeX
     || myMainSceneFbos[0]->GetVPSizeY() != aRendSizeY
     || myMainSceneFbos[0]->NbSamples()  != aNbSamples)
    {
      if (!myTransientDrawToFront)
      {
        myImmediateSceneFbos[0]->Release (aCtx.operator->());
        myImmediateSceneFbos[1]->Release (aCtx.operator->());
        myImmediateSceneFbos[0]->ChangeViewport (0, 0);
        myImmediateSceneFbos[1]->ChangeViewport (0, 0);
      }

      // prepare FBOs containing main scene
      // for further blitting and rendering immediate presentations on top
      if (aCtx->core20fwd != NULL)
      {
        const bool wasFailedMain0 = checkWasFailedFbo (myMainSceneFbos[0], aRendSizeX, aRendSizeY, aNbSamples);
        if (!myMainSceneFbos[0]->Init (aCtx, aRendSizeX, aRendSizeY, myFboColorFormat, myFboDepthFormat, aNbSamples)
         && !wasFailedMain0)
        {
          TCollection_ExtendedString aMsg = TCollection_ExtendedString() + "Error! Main FBO "
                                          + printFboFormat (myMainSceneFbos[0]) + " initialization has failed";
          aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, aMsg);
        }
      }
    }
    if (myMainSceneFbos[0]->IsValid() && (toInitImmediateFbo || myImmediateSceneFbos[0]->IsValid()))
    {
      const bool wasFailedImm0 = checkWasFailedFbo (myImmediateSceneFbos[0], myMainSceneFbos[0]);
      if (!myImmediateSceneFbos[0]->InitLazy (aCtx, *myMainSceneFbos[0])
       && !wasFailedImm0)
      {
        TCollection_ExtendedString aMsg = TCollection_ExtendedString() + "Error! Immediate FBO "
                                        + printFboFormat (myImmediateSceneFbos[0]) + " initialization has failed";
        aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, aMsg);
      }
    }
  }
  else
  {
    myMainSceneFbos     [0]->Release (aCtx.operator->());
    myMainSceneFbos     [1]->Release (aCtx.operator->());
    myImmediateSceneFbos[0]->Release (aCtx.operator->());
    myImmediateSceneFbos[1]->Release (aCtx.operator->());
    myMainSceneFbos     [0]->ChangeViewport (0, 0);
    myMainSceneFbos     [1]->ChangeViewport (0, 0);
    myImmediateSceneFbos[0]->ChangeViewport (0, 0);
    myImmediateSceneFbos[1]->ChangeViewport (0, 0);
  }

  if (aProjectType == Graphic3d_Camera::Projection_Stereo
   && myMainSceneFbos[0]->IsValid()
   && myVrHmd == NULL) // no frame caching with HMD
  {
    const bool wasFailedMain1 = checkWasFailedFbo (myMainSceneFbos[1], myMainSceneFbos[0]);
    if (!myMainSceneFbos[1]->InitLazy (aCtx, *myMainSceneFbos[0])
     && !wasFailedMain1)
    {
      TCollection_ExtendedString aMsg = TCollection_ExtendedString() + "Error! Main FBO (second) "
                                      + printFboFormat (myMainSceneFbos[1]) + " initialization has failed";
      aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, aMsg);
    }
    if (!myMainSceneFbos[1]->IsValid())
    {
      // no enough memory?
      aProjectType = Graphic3d_Camera::Projection_Perspective;
    }
    else if (!myTransientDrawToFront)
    {
      //
    }
    else if (!aCtx->HasStereoBuffers() || aStereoMode != Graphic3d_StereoMode_QuadBuffer)
    {
      const bool wasFailedImm0 = checkWasFailedFbo (myImmediateSceneFbos[0], myMainSceneFbos[0]);
      const bool wasFailedImm1 = checkWasFailedFbo (myImmediateSceneFbos[1], myMainSceneFbos[0]);
      if (!myImmediateSceneFbos[0]->InitLazy (aCtx, *myMainSceneFbos[0])
       && !wasFailedImm0)
      {
        TCollection_ExtendedString aMsg = TCollection_ExtendedString() + "Error! Immediate FBO (first) "
                                        + printFboFormat (myImmediateSceneFbos[0]) + " initialization has failed";
        aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, aMsg);
      }
      if (!myImmediateSceneFbos[1]->InitLazy (aCtx, *myMainSceneFbos[0])
       && !wasFailedImm1)
      {
        TCollection_ExtendedString aMsg = TCollection_ExtendedString() + "Error! Immediate FBO (first) "
                                        + printFboFormat (myImmediateSceneFbos[1]) + " initialization has failed";
        aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, aMsg);
      }
      if (!myImmediateSceneFbos[0]->IsValid()
       || !myImmediateSceneFbos[1]->IsValid())
      {
        aProjectType = Graphic3d_Camera::Projection_Perspective;
      }
    }
  }

  // create color and coverage accumulation buffers required for OIT algorithm
  if (toUseOit)
  {
    Standard_Integer anFboIt = 0;
    for (; anFboIt < 2; ++anFboIt)
    {
      Handle(OpenGl_FrameBuffer)& aMainSceneFbo          = myMainSceneFbos        [anFboIt];
      Handle(OpenGl_FrameBuffer)& aMainSceneFboOit       = myMainSceneFbosOit     [anFboIt];
      Handle(OpenGl_FrameBuffer)& anImmediateSceneFbo    = myImmediateSceneFbos   [anFboIt];
      Handle(OpenGl_FrameBuffer)& anImmediateSceneFboOit = myImmediateSceneFbosOit[anFboIt];
      if (aMainSceneFbo->IsValid()
       && (aMainSceneFboOit->GetVPSizeX() != aRendSizeX
        || aMainSceneFboOit->GetVPSizeY() != aRendSizeY
        || aMainSceneFboOit->NbSamples()  != aNbSamples))
      {
        Standard_Integer aColorConfig = 0;
        for (;;) // seemly responding to driver limitation (GL_FRAMEBUFFER_UNSUPPORTED)
        {
          if (myFboOitColorConfig.IsEmpty())
          {
            if (!chooseOitColorConfiguration (aCtx, aColorConfig++, myFboOitColorConfig))
            {
              break;
            }
          }
          if (aMainSceneFboOit->Init (aCtx, aRendSizeX, aRendSizeY, myFboOitColorConfig, aMainSceneFbo->DepthStencilTexture(), aNbSamples))
          {
            break;
          }
          myFboOitColorConfig.Clear();
        }
        if (!aMainSceneFboOit->IsValid())
        {
          break;
        }
      }
      else if (!aMainSceneFbo->IsValid())
      {
        aMainSceneFboOit->Release (aCtx.operator->());
        aMainSceneFboOit->ChangeViewport (0, 0);
      }

      if (anImmediateSceneFbo->IsValid()
       && (anImmediateSceneFboOit->GetVPSizeX() != aRendSizeX
        || anImmediateSceneFboOit->GetVPSizeY() != aRendSizeY
        || anImmediateSceneFboOit->NbSamples()  != aNbSamples))
      {
        if (!anImmediateSceneFboOit->Init (aCtx, aRendSizeX, aRendSizeY, myFboOitColorConfig,
                                           anImmediateSceneFbo->DepthStencilTexture(), aNbSamples))
        {
          break;
        }
      }
      else if (!anImmediateSceneFbo->IsValid())
      {
        anImmediateSceneFboOit->Release (aCtx.operator->());
        anImmediateSceneFboOit->ChangeViewport (0, 0);
      }
    }
    if (anFboIt == 0) // only the first OIT framebuffer is mandatory
    {
      aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH,
                         "Initialization of float texture framebuffer for use with\n"
                         "  blended order-independent transparency rendering algorithm has failed.\n"
                         "  Blended order-independent transparency will not be available.\n");
      if (aNbSamples > 0)
      {
        myToDisableOITMSAA = Standard_True;
      }
      else
      {
        myToDisableOIT     = Standard_True;
      }
      toUseOit = false;
    }
  }
  if (!toUseOit && myMainSceneFbosOit[0]->IsValid())
  {
    myMainSceneFbosOit     [0]->Release (aCtx.operator->());
    myMainSceneFbosOit     [1]->Release (aCtx.operator->());
    myImmediateSceneFbosOit[0]->Release (aCtx.operator->());
    myImmediateSceneFbosOit[1]->Release (aCtx.operator->());
    myMainSceneFbosOit     [0]->ChangeViewport (0, 0);
    myMainSceneFbosOit     [1]->ChangeViewport (0, 0);
    myImmediateSceneFbosOit[0]->ChangeViewport (0, 0);
    myImmediateSceneFbosOit[1]->ChangeViewport (0, 0);
  }

  if (aProjectType == Graphic3d_Camera::Projection_Stereo)
  {
    OpenGl_FrameBuffer* aMainFbos[2] =
    {
      myMainSceneFbos[0]->IsValid() ? myMainSceneFbos[0].operator->() : NULL,
      myMainSceneFbos[1]->IsValid() ? myMainSceneFbos[1].operator->() : NULL
    };
    OpenGl_FrameBuffer* aMainFbosOit[2] =
    {
      myMainSceneFbosOit[0]->IsValid() ? myMainSceneFbosOit[0].operator->() : NULL,
      myMainSceneFbosOit[1]->IsValid() ? myMainSceneFbosOit[1].operator->() :
        myMainSceneFbosOit[0]->IsValid() ? myMainSceneFbosOit[0].operator->() : NULL
    };

    OpenGl_FrameBuffer* anImmFbos[2] =
    {
      myImmediateSceneFbos[0]->IsValid() ? myImmediateSceneFbos[0].operator->() : NULL,
      myImmediateSceneFbos[1]->IsValid() ? myImmediateSceneFbos[1].operator->() : NULL
    };
    OpenGl_FrameBuffer* anImmFbosOit[2] =
    {
      myImmediateSceneFbosOit[0]->IsValid() ? myImmediateSceneFbosOit[0].operator->() : NULL,
      myImmediateSceneFbosOit[1]->IsValid() ? myImmediateSceneFbosOit[1].operator->() :
        myImmediateSceneFbosOit[0]->IsValid() ? myImmediateSceneFbosOit[0].operator->() : NULL
    };

    if (myVrHmd != NULL)
    {
      // use single frame for both views - caching main scene content makes no sense
      // when head position is expected to be updated each frame redraw with high accuracy
      aMainFbos[1]    = aMainFbos[0];
      aMainFbosOit[1] = aMainFbosOit[0];
      anImmFbos[0]    = aMainFbos[0];
      anImmFbos[1]    = aMainFbos[1];
      anImmFbosOit[0] = aMainFbosOit[0];
      anImmFbosOit[1] = aMainFbosOit[1];
    }
    else if (!myTransientDrawToFront)
    {
      anImmFbos   [0] = aMainFbos   [0];
      anImmFbos   [1] = aMainFbos   [1];
      anImmFbosOit[0] = aMainFbosOit[0];
      anImmFbosOit[1] = aMainFbosOit[1];
    }
    else if (aStereoMode == Graphic3d_StereoMode_SoftPageFlip
          || aStereoMode == Graphic3d_StereoMode_QuadBuffer)
    {
      anImmFbos   [0] = NULL;
      anImmFbos   [1] = NULL;
      anImmFbosOit[0] = NULL;
      anImmFbosOit[1] = NULL;
    }

  #if !defined(GL_ES_VERSION_2_0)
    aCtx->SetReadDrawBuffer (aStereoMode == Graphic3d_StereoMode_QuadBuffer ? GL_BACK_LEFT : GL_BACK);
  #endif
    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         aMainFbos[0] != NULL ? myRenderParams.RenderResolutionScale : 1.0f);

    redraw (Graphic3d_Camera::Projection_MonoLeftEye, aMainFbos[0], aMainFbosOit[0]);
    myBackBufferRestored = Standard_True;
    myIsImmediateDrawn   = Standard_False;
  #if !defined(GL_ES_VERSION_2_0)
    aCtx->SetReadDrawBuffer (aStereoMode == Graphic3d_StereoMode_QuadBuffer ? GL_BACK_LEFT : GL_BACK);
  #endif
    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         anImmFbos[0] != NULL ? myRenderParams.RenderResolutionScale : 1.0f);
    if (!redrawImmediate (Graphic3d_Camera::Projection_MonoLeftEye, aMainFbos[0], anImmFbos[0], anImmFbosOit[0]))
    {
      toSwap = false;
    }
    else if (aStereoMode == Graphic3d_StereoMode_SoftPageFlip && toSwap)
    {
      aCtx->SwapBuffers();
    }

  #ifdef HAVE_OPENVR
    if (myVrHmd != NULL)
    {
      // push Left frame to HMD display composer
      vr::Texture_t aVRTexture = { (void* )(size_t )aMainFbos[0]->ColorTexture()->TextureId(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
      const vr::EVRCompositorError aVRError = vr::VRCompositor()->Submit (vr::Eye_Left, &aVRTexture);
      if (aVRError != vr::VRCompositorError_None)
      {
        Message::DefaultMessenger()->Send (getVRCompositorError (aVRError), Message_Trace);
      }
    }
  #endif

  #if !defined(GL_ES_VERSION_2_0)
    aCtx->SetReadDrawBuffer (aStereoMode == Graphic3d_StereoMode_QuadBuffer ? GL_BACK_RIGHT : GL_BACK);
  #endif
    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         aMainFbos[1] != NULL ? myRenderParams.RenderResolutionScale : 1.0f);

    redraw (Graphic3d_Camera::Projection_MonoRightEye, aMainFbos[1], aMainFbosOit[1]);
    myBackBufferRestored = Standard_True;
    myIsImmediateDrawn   = Standard_False;
    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         anImmFbos[1] != NULL ? myRenderParams.RenderResolutionScale : 1.0f);
    if (!redrawImmediate (Graphic3d_Camera::Projection_MonoRightEye, aMainFbos[1], anImmFbos[1], anImmFbosOit[1]))
    {
      toSwap = false;
    }

  #ifdef HAVE_OPENVR
    if (myVrHmd != NULL)
    {
      // push Right frame to HMD display composer
      {
        vr::Texture_t aVRTexture = { (void* )(size_t )aMainFbos[1]->ColorTexture()->TextureId(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        const vr::EVRCompositorError aVRError = vr::VRCompositor()->Submit (vr::Eye_Right, &aVRTexture);
        if (aVRError != vr::VRCompositorError_None)
        {
          Message::DefaultMessenger()->Send (getVRCompositorError (aVRError), Message_Trace);
        }
      }
      ::glFinish();

      if (myRenderParams.ToMirrorComposer)
      {
        blitBuffers (aMainFbos[0], aFrameBuffer, myToFlipOutput);
      }
    }
    else
  #endif
    if (anImmFbos[0] != NULL)
    {
      aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(), 1.0f);
      drawStereoPair (aFrameBuffer);
    }
  }
  else
  {
    OpenGl_FrameBuffer* aMainFbo    = myMainSceneFbos[0]->IsValid() ? myMainSceneFbos[0].operator->() : aFrameBuffer;
    OpenGl_FrameBuffer* aMainFboOit = myMainSceneFbosOit[0]->IsValid() ? myMainSceneFbosOit[0].operator->() : NULL;
    OpenGl_FrameBuffer* anImmFbo    = aFrameBuffer;
    OpenGl_FrameBuffer* anImmFboOit = NULL;
    if (!myTransientDrawToFront)
    {
      anImmFbo    = aMainFbo;
      anImmFboOit = aMainFboOit;
    }
    else if (myImmediateSceneFbos[0]->IsValid())
    {
      anImmFbo    = myImmediateSceneFbos[0].operator->();
      anImmFboOit = myImmediateSceneFbosOit[0]->IsValid() ? myImmediateSceneFbosOit[0].operator->() : NULL;
    }

  #if !defined(GL_ES_VERSION_2_0)
    if (aMainFbo == NULL)
    {
      aCtx->SetReadDrawBuffer (GL_BACK);
    }
  #endif
    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         aMainFbo != aFrameBuffer ? myRenderParams.RenderResolutionScale : 1.0f);

    redraw (aProjectType, aMainFbo, aMainFboOit);
    myBackBufferRestored = Standard_True;
    myIsImmediateDrawn   = Standard_False;
    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         anImmFbo != aFrameBuffer ? myRenderParams.RenderResolutionScale : 1.0f);
    if (!redrawImmediate (aProjectType, aMainFbo, anImmFbo, anImmFboOit))
    {
      toSwap = false;
    }

    if (anImmFbo != NULL
     && anImmFbo != aFrameBuffer)
    {
      blitBuffers (anImmFbo, aFrameBuffer, myToFlipOutput);
    }
  }

  if (myRenderParams.Method == Graphic3d_RM_RAYTRACING
   && myRenderParams.IsGlobalIlluminationEnabled)
  {
    myAccumFrames++;
  }

  // bind default FBO
  bindDefaultFbo();

  if (wasDisabledMSAA != myToDisableMSAA
   || hadFboBlit      != myHasFboBlit)
  {
    // retry on error
    Redraw();
  }

  // reset state for safety
  aCtx->BindProgram (Handle(OpenGl_ShaderProgram)());
  if (aCtx->caps->ffpEnable)
  {
    aCtx->ShaderManager()->PushState (Handle(OpenGl_ShaderProgram)());
  }

  // Swap the buffers
  if (toSwap)
  {
    aCtx->SwapBuffers();
    if (!myMainSceneFbos[0]->IsValid())
    {
      myBackBufferRestored = Standard_False;
    }
  }
  else
  {
    aCtx->core11fwd->glFlush();
  }

  // reset render mode state
  aCtx->FetchState();
  aCtx->FrameStats()->FrameEnd (myWorkspace);

  myWasRedrawnGL = Standard_True;
}

// =======================================================================
// function : RedrawImmediate
// purpose  :
// =======================================================================
void OpenGl_View::RedrawImmediate()
{
  if (!myWorkspace->Activate())
    return;

  // no special handling of HMD display (myVrHmd),
  // since it will force full Redraw() due to no frame caching (myBackBufferRestored)
  Handle(OpenGl_Context) aCtx = myWorkspace->GetGlContext();
  if (!myTransientDrawToFront
   || !myBackBufferRestored
   || (aCtx->caps->buffersNoSwap && !myMainSceneFbos[0]->IsValid()))
  {
    Redraw();
    return;
  }

  const Graphic3d_StereoMode   aStereoMode  = myRenderParams.StereoMode;
  Graphic3d_Camera::Projection aProjectType = myEffectiveCamera->ProjectionType();
  OpenGl_FrameBuffer*          aFrameBuffer = myFBO.operator->();
  aCtx->FrameStats()->FrameStart (myWorkspace);

  if ( aFrameBuffer == NULL
   && !aCtx->DefaultFrameBuffer().IsNull()
   &&  aCtx->DefaultFrameBuffer()->IsValid())
  {
    aFrameBuffer = aCtx->DefaultFrameBuffer().operator->();
  }

  if (aProjectType == Graphic3d_Camera::Projection_Stereo)
  {
    if (myMainSceneFbos[0]->IsValid()
    && !myMainSceneFbos[1]->IsValid())
    {
      aProjectType = Graphic3d_Camera::Projection_Perspective;
    }
  }

  bool toSwap = false;
  if (aProjectType == Graphic3d_Camera::Projection_Stereo)
  {
    OpenGl_FrameBuffer* aMainFbos[2] =
    {
      myMainSceneFbos[0]->IsValid() ? myMainSceneFbos[0].operator->() : NULL,
      myMainSceneFbos[1]->IsValid() ? myMainSceneFbos[1].operator->() : NULL
    };
    OpenGl_FrameBuffer* anImmFbos[2] =
    {
      myImmediateSceneFbos[0]->IsValid() ? myImmediateSceneFbos[0].operator->() : NULL,
      myImmediateSceneFbos[1]->IsValid() ? myImmediateSceneFbos[1].operator->() : NULL
    };
    OpenGl_FrameBuffer* anImmFbosOit[2] =
    {
      myImmediateSceneFbosOit[0]->IsValid() ? myImmediateSceneFbosOit[0].operator->() : NULL,
      myImmediateSceneFbosOit[1]->IsValid() ? myImmediateSceneFbosOit[1].operator->() :
        myImmediateSceneFbosOit[0]->IsValid() ? myImmediateSceneFbosOit[0].operator->() : NULL
    };
    if (aStereoMode == Graphic3d_StereoMode_SoftPageFlip
     || aStereoMode == Graphic3d_StereoMode_QuadBuffer)
    {
      anImmFbos[0]    = NULL;
      anImmFbos[1]    = NULL;
      anImmFbosOit[0] = NULL;
      anImmFbosOit[1] = NULL;
    }

    if (aCtx->arbFBO != NULL)
    {
      aCtx->arbFBO->glBindFramebuffer (GL_FRAMEBUFFER, OpenGl_FrameBuffer::NO_FRAMEBUFFER);
    }
  #if !defined(GL_ES_VERSION_2_0)
    if (anImmFbos[0] == NULL)
    {
      aCtx->SetReadDrawBuffer (aStereoMode == Graphic3d_StereoMode_QuadBuffer ? GL_BACK_LEFT : GL_BACK);
    }
  #endif

    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         anImmFbos[0] != NULL ? myRenderParams.RenderResolutionScale : 1.0f);
    toSwap = redrawImmediate (Graphic3d_Camera::Projection_MonoLeftEye,
                              aMainFbos[0],
                              anImmFbos[0],
                              anImmFbosOit[0],
                              Standard_True) || toSwap;
    if (aStereoMode == Graphic3d_StereoMode_SoftPageFlip
    &&  toSwap
    && !aCtx->caps->buffersNoSwap)
    {
      aCtx->SwapBuffers();
    }

    if (aCtx->arbFBO != NULL)
    {
      aCtx->arbFBO->glBindFramebuffer (GL_FRAMEBUFFER, OpenGl_FrameBuffer::NO_FRAMEBUFFER);
    }
  #if !defined(GL_ES_VERSION_2_0)
    if (anImmFbos[1] == NULL)
    {
      aCtx->SetReadDrawBuffer (aStereoMode == Graphic3d_StereoMode_QuadBuffer ? GL_BACK_RIGHT : GL_BACK);
    }
  #endif
    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         anImmFbos[1] != NULL ? myRenderParams.RenderResolutionScale : 1.0f);
    toSwap = redrawImmediate (Graphic3d_Camera::Projection_MonoRightEye,
                              aMainFbos[1],
                              anImmFbos[1],
                              anImmFbosOit[1],
                              Standard_True) || toSwap;
    if (anImmFbos[0] != NULL)
    {
      drawStereoPair (aFrameBuffer);
    }
  }
  else
  {
    OpenGl_FrameBuffer* aMainFbo = myMainSceneFbos[0]->IsValid() ? myMainSceneFbos[0].operator->() : NULL;
    OpenGl_FrameBuffer* anImmFbo = aFrameBuffer;
    OpenGl_FrameBuffer* anImmFboOit = NULL;
    if (myImmediateSceneFbos[0]->IsValid())
    {
      anImmFbo    = myImmediateSceneFbos[0].operator->();
      anImmFboOit = myImmediateSceneFbosOit[0]->IsValid() ? myImmediateSceneFbosOit[0].operator->() : NULL;
    }
  #if !defined(GL_ES_VERSION_2_0)
    if (aMainFbo == NULL)
    {
      aCtx->SetReadDrawBuffer (GL_BACK);
    }
  #endif
    aCtx->SetResolution (myRenderParams.Resolution, myRenderParams.ResolutionRatio(),
                         anImmFbo != aFrameBuffer ? myRenderParams.RenderResolutionScale : 1.0f);
    toSwap = redrawImmediate (aProjectType,
                              aMainFbo,
                              anImmFbo,
                              anImmFboOit,
                              Standard_True) || toSwap;
    if (anImmFbo != NULL
     && anImmFbo != aFrameBuffer)
    {
      blitBuffers (anImmFbo, aFrameBuffer, myToFlipOutput);
    }
  }

  // bind default FBO
  bindDefaultFbo();

  // reset state for safety
  aCtx->BindProgram (Handle(OpenGl_ShaderProgram)());
  if (aCtx->caps->ffpEnable)
  {
    aCtx->ShaderManager()->PushState (Handle(OpenGl_ShaderProgram)());
  }

  if (toSwap && !aCtx->caps->buffersNoSwap)
  {
    aCtx->SwapBuffers();
  }
  else
  {
    aCtx->core11fwd->glFlush();
  }
  aCtx->FrameStats()->FrameEnd (myWorkspace);

  myWasRedrawnGL = Standard_True;
}

// =======================================================================
// function : redraw
// purpose  :
// =======================================================================
void OpenGl_View::redraw (const Graphic3d_Camera::Projection theProjection,
                          OpenGl_FrameBuffer*                theReadDrawFbo,
                          OpenGl_FrameBuffer*                theOitAccumFbo)
{
  Handle(OpenGl_Context) aCtx = myWorkspace->GetGlContext();
  if (theReadDrawFbo != NULL)
  {
    theReadDrawFbo->BindBuffer    (aCtx);
    theReadDrawFbo->SetupViewport (aCtx);
  }
  else
  {
    const Standard_Integer aViewport[4] = { 0, 0, myWindow->Width(), myWindow->Height() };
    aCtx->ResizeViewport (aViewport);
  }

  // request reset of material
  aCtx->ShaderManager()->UpdateMaterialState();

  myWorkspace->UseZBuffer()    = Standard_True;
  myWorkspace->UseDepthWrite() = Standard_True;
  GLbitfield toClear = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
  glDepthFunc (GL_LEQUAL);
  glDepthMask (GL_TRUE);
  glEnable (GL_DEPTH_TEST);

#if !defined(GL_ES_VERSION_2_0)
  glClearDepth (1.0);
#else
  glClearDepthf (1.0f);
#endif

  const OpenGl_Vec4& aBgColor = myBgColor;
  glClearColor (aBgColor.r(), aBgColor.g(), aBgColor.b(), 0.0f);

  glClear (toClear);

  render (theProjection, theReadDrawFbo, theOitAccumFbo, Standard_False);
}

// =======================================================================
// function : redrawMonoImmediate
// purpose  :
// =======================================================================
bool OpenGl_View::redrawImmediate (const Graphic3d_Camera::Projection theProjection,
                                   OpenGl_FrameBuffer*                theReadFbo,
                                   OpenGl_FrameBuffer*                theDrawFbo,
                                   OpenGl_FrameBuffer*                theOitAccumFbo,
                                   const Standard_Boolean             theIsPartialUpdate)
{
  Handle(OpenGl_Context) aCtx = myWorkspace->GetGlContext();
  GLboolean toCopyBackToFront = GL_FALSE;
  if (theDrawFbo == theReadFbo
   && theDrawFbo != NULL
   && theDrawFbo->IsValid())
  {
    myBackBufferRestored = Standard_False;
    theDrawFbo->BindBuffer (aCtx);
  }
  else if (theReadFbo != NULL
        && theReadFbo->IsValid()
        && aCtx->IsRender())
  {
    if (!blitBuffers (theReadFbo, theDrawFbo))
    {
      return true;
    }
  }
  else if (theDrawFbo == NULL)
  {
  #if !defined(GL_ES_VERSION_2_0)
    aCtx->core11fwd->glGetBooleanv (GL_DOUBLEBUFFER, &toCopyBackToFront);
  #endif
    if (toCopyBackToFront
     && myTransientDrawToFront)
    {
      if (!HasImmediateStructures()
       && !theIsPartialUpdate)
      {
        // prefer Swap Buffers within Redraw in compatibility mode (without FBO)
        return true;
      }
      if (!copyBackToFront())
      {
        toCopyBackToFront    = GL_FALSE;
        myBackBufferRestored = Standard_False;
      }
    }
    else
    {
      myBackBufferRestored = Standard_False;
    }
  }
  else
  {
    myBackBufferRestored = Standard_False;
  }
  myIsImmediateDrawn = Standard_True;

  myWorkspace->UseZBuffer()    = Standard_True;
  myWorkspace->UseDepthWrite() = Standard_True;
  glDepthFunc (GL_LEQUAL);
  glDepthMask (GL_TRUE);
  glEnable (GL_DEPTH_TEST);
#if !defined(GL_ES_VERSION_2_0)
  glClearDepth (1.0);
#else
  glClearDepthf (1.0f);
#endif

  render (theProjection, theDrawFbo, theOitAccumFbo, Standard_True);

  return !toCopyBackToFront;
}

//=======================================================================
//function : Render
//purpose  :
//=======================================================================
void OpenGl_View::render (Graphic3d_Camera::Projection theProjection,
                          OpenGl_FrameBuffer*          theOutputFBO,
                          OpenGl_FrameBuffer*          theOitAccumFbo,
                          const Standard_Boolean       theToDrawImmediate)
{
  // ==================================
  //      Step 1: Prepare for render
  // ==================================

  const Handle(OpenGl_Context)& aContext = myWorkspace->GetGlContext();
  aContext->SetSampleAlphaToCoverage (myRenderParams.ToEnableAlphaToCoverage);

#if !defined(GL_ES_VERSION_2_0)
  // Disable current clipping planes
  if (aContext->core11 != NULL)
  {
    const Standard_Integer aMaxPlanes = aContext->MaxClipPlanes();
    for (Standard_Integer aClipPlaneId = GL_CLIP_PLANE0; aClipPlaneId < GL_CLIP_PLANE0 + aMaxPlanes; ++aClipPlaneId)
    {
      aContext->core11fwd->glDisable (aClipPlaneId);
    }
  }
#endif

  // update states of OpenGl_BVHTreeSelector (frustum culling algorithm);
  // note that we pass here window dimensions ignoring Graphic3d_RenderingParams::RenderResolutionScale
  myBVHSelector.SetViewVolume (myEffectiveCamera);
  myBVHSelector.SetViewportSize (myWindow->Width(), myWindow->Height(), myRenderParams.ResolutionRatio());
  myBVHSelector.CacheClipPtsProjections();

  const Handle(OpenGl_ShaderManager)& aManager = aContext->ShaderManager();
  const Handle(Graphic3d_LightSet)&   aLights  = myShadingModel == Graphic3d_TOSM_UNLIT ? myNoShadingLight : myLights;
  Standard_Size aLightsRevision = 0;
  if (!aLights.IsNull())
  {
    aLightsRevision = aLights->UpdateRevision();
  }
  if (StateInfo (myCurrLightSourceState, aManager->LightSourceState().Index()) != myLastLightSourceState
   || aLightsRevision != myLightsRevision)
  {
    myLightsRevision = aLightsRevision;
    aManager->UpdateLightSourceStateTo (aLights);
    myLastLightSourceState = StateInfo (myCurrLightSourceState, aManager->LightSourceState().Index());
  }

  // Update matrices if camera has changed.
  Graphic3d_WorldViewProjState aWVPState = myEffectiveCamera->WorldViewProjState();
  if (myWorldViewProjState != aWVPState)
  {
    myAccumFrames = 0;
    myWorldViewProjState = aWVPState;
  }

  myLocalOrigin.SetCoord (0.0, 0.0, 0.0);
  aContext->SetCamera (myEffectiveCamera);
  if (aManager->ModelWorldState().Index() == 0)
  {
    aContext->ShaderManager()->UpdateModelWorldStateTo (OpenGl_Mat4());
  }

  // ====================================
  //      Step 2: Redraw background
  // ====================================

  // Render background
  if (!theToDrawImmediate)
  {
    drawBackground (myWorkspace);
  }

#if !defined(GL_ES_VERSION_2_0)
  // Switch off lighting by default
  if (aContext->core11 != NULL
   && aContext->caps->ffpEnable)
  {
    glDisable(GL_LIGHTING);
  }
#endif

  // =================================
  //      Step 3: Redraw main plane
  // =================================

  // Setup face culling
  GLboolean isCullFace = GL_FALSE;
  if (myBackfacing != Graphic3d_TOBM_AUTOMATIC)
  {
    isCullFace = glIsEnabled (GL_CULL_FACE);
    if (myBackfacing == Graphic3d_TOBM_DISABLE)
    {
      glEnable (GL_CULL_FACE);
      glCullFace (GL_BACK);
    }
    else
      glDisable (GL_CULL_FACE);
  }

#if !defined(GL_ES_VERSION_2_0)
  // if the view is scaled normal vectors are scaled to unit
  // length for correct displaying of shaded objects
  const gp_Pnt anAxialScale = aContext->Camera()->AxialScale();
  if (anAxialScale.X() != 1.F ||
      anAxialScale.Y() != 1.F ||
      anAxialScale.Z() != 1.F)
  {
    aContext->SetGlNormalizeEnabled (Standard_True);
  }
  else
  {
    aContext->SetGlNormalizeEnabled (Standard_False);
  }

  // Apply InteriorShadingMethod
  if (aContext->core11 != NULL)
  {
    aContext->core11->glShadeModel (myShadingModel == Graphic3d_TOSM_FACET
                                 || myShadingModel == Graphic3d_TOSM_UNLIT ? GL_FLAT : GL_SMOOTH);
  }
#endif

  aManager->SetShadingModel (myShadingModel);

  // Redraw 3d scene
  if (theProjection == Graphic3d_Camera::Projection_MonoLeftEye)
  {
    aContext->ProjectionState.SetCurrent (aContext->Camera()->ProjectionStereoLeftF());
    aContext->ApplyProjectionMatrix();
  }
  else if (theProjection == Graphic3d_Camera::Projection_MonoRightEye)
  {
    aContext->ProjectionState.SetCurrent (aContext->Camera()->ProjectionStereoRightF());
    aContext->ApplyProjectionMatrix();
  }

  myWorkspace->SetEnvironmentTexture (myTextureEnv);

  renderScene (theProjection, theOutputFBO, theOitAccumFbo, theToDrawImmediate);

  myWorkspace->SetEnvironmentTexture (Handle(OpenGl_TextureSet)());

  // ===============================
  //      Step 4: Trihedron
  // ===============================

  // Resetting GL parameters according to the default aspects
  // in order to synchronize GL state with the graphic driver state
  // before drawing auxiliary stuff (trihedrons, overlayer)
  myWorkspace->ResetAppliedAspect();

  // Render trihedron
  if (!theToDrawImmediate)
  {
    renderTrihedron (myWorkspace);

    // Restore face culling
    if (myBackfacing != Graphic3d_TOBM_AUTOMATIC)
    {
      if (isCullFace)
      {
        glEnable (GL_CULL_FACE);
        glCullFace (GL_BACK);
      }
      else
        glDisable (GL_CULL_FACE);
    }
  }
  else
  {
    renderFrameStats();
  }

  myWorkspace->ResetAppliedAspect();
  aContext->SetSampleAlphaToCoverage (false);

  // reset FFP state for safety
  aContext->BindProgram (Handle(OpenGl_ShaderProgram)());
  if (aContext->caps->ffpEnable)
  {
    aContext->ShaderManager()->PushState (Handle(OpenGl_ShaderProgram)());
  }

  // ==============================================================
  //      Step 6: Keep shader manager informed about last View
  // ==============================================================

  if (!aManager.IsNull())
  {
    aManager->SetLastView (this);
  }
}

// =======================================================================
// function : InvalidateBVHData
// purpose  :
// =======================================================================
void OpenGl_View::InvalidateBVHData (const Graphic3d_ZLayerId theLayerId)
{
  myZLayers.InvalidateBVHData (theLayerId);
}

//=======================================================================
//function : renderStructs
//purpose  :
//=======================================================================
void OpenGl_View::renderStructs (Graphic3d_Camera::Projection theProjection,
                                 OpenGl_FrameBuffer*          theReadDrawFbo,
                                 OpenGl_FrameBuffer*          theOitAccumFbo,
                                 const Standard_Boolean       theToDrawImmediate)
{
  if ( myZLayers.NbStructures() <= 0 )
    return;

  Handle(OpenGl_Context) aCtx = myWorkspace->GetGlContext();
  Standard_Boolean toRenderGL = theToDrawImmediate ||
    myRenderParams.Method != Graphic3d_RM_RAYTRACING ||
    myRaytraceInitStatus == OpenGl_RT_FAIL ||
    aCtx->IsFeedback();

  myZLayers.UpdateCulling (myWorkspace, theToDrawImmediate);

  if (!toRenderGL)
  {
    toRenderGL = !initRaytraceResources (aCtx) ||
      !updateRaytraceGeometry (OpenGl_GUM_CHECK, myId, aCtx);

    toRenderGL |= !myIsRaytraceDataValid; // if no ray-trace data use OpenGL

    if (!toRenderGL)
    {
      const Standard_Integer aSizeX = theReadDrawFbo != NULL ? theReadDrawFbo->GetVPSizeX() : myWindow->Width();
      const Standard_Integer aSizeY = theReadDrawFbo != NULL ? theReadDrawFbo->GetVPSizeY() : myWindow->Height();
      myOpenGlFBO ->InitLazy (aCtx, aSizeX, aSizeY, myFboColorFormat, myFboDepthFormat, 0);

      if (myRaytraceFilter.IsNull())
        myRaytraceFilter = new OpenGl_RaytraceFilter;

      myRaytraceFilter->SetPrevRenderFilter (myWorkspace->GetRenderFilter());

      if (theReadDrawFbo != NULL)
        theReadDrawFbo->UnbindBuffer (aCtx);

      // Prepare preliminary OpenGL output
      if (aCtx->arbFBOBlit != NULL)
      {
        // Render bottom OSD layer
        myZLayers.Render (myWorkspace, theToDrawImmediate, OpenGl_LF_Bottom, theReadDrawFbo, theOitAccumFbo);

        myWorkspace->SetRenderFilter (myRaytraceFilter);
        {
          if (theReadDrawFbo != NULL)
          {
            theReadDrawFbo->BindDrawBuffer (aCtx);
          }
          else
          {
            aCtx->arbFBO->glBindFramebuffer (GL_DRAW_FRAMEBUFFER, 0);
          }

          // Render non-polygonal elements in default layer
          myZLayers.Render (myWorkspace, theToDrawImmediate, OpenGl_LF_Default, theReadDrawFbo, theOitAccumFbo);
        }
        myWorkspace->SetRenderFilter (myRaytraceFilter->PrevRenderFilter());
      }

      if (theReadDrawFbo != NULL)
      {
        theReadDrawFbo->BindBuffer (aCtx);
      }
      else
      {
        aCtx->arbFBO->glBindFramebuffer (GL_FRAMEBUFFER, 0);
      }

      // Reset OpenGl aspects state to default to avoid enabling of
      // backface culling which is not supported in ray-tracing.
      myWorkspace->ResetAppliedAspect();

      // Ray-tracing polygonal primitive arrays
      raytrace (aSizeX, aSizeY, theProjection, theReadDrawFbo, aCtx);

      // Render upper (top and topmost) OpenGL layers
      myZLayers.Render (myWorkspace, theToDrawImmediate, OpenGl_LF_Upper, theReadDrawFbo, theOitAccumFbo);
    }
  }

  // Redraw 3D scene using OpenGL in standard
  // mode or in case of ray-tracing failure
  if (toRenderGL)
  {
    myZLayers.Render (myWorkspace, theToDrawImmediate, OpenGl_LF_All, theReadDrawFbo, theOitAccumFbo);

    // Set flag that scene was redrawn by standard pipeline
    myWasRedrawnGL = Standard_True;
  }
}

//=======================================================================
//function : renderTrihedron
//purpose  :
//=======================================================================
void OpenGl_View::renderTrihedron (const Handle(OpenGl_Workspace) &theWorkspace)
{
  if (myToShowGradTrihedron)
  {
    myGraduatedTrihedron.Render (theWorkspace);
  }
}

//=======================================================================
//function : renderFrameStats
//purpose  :
//=======================================================================
void OpenGl_View::renderFrameStats()
{
  if (myRenderParams.ToShowStats
   && myRenderParams.CollectedStats != Graphic3d_RenderingParams::PerfCounters_NONE)
  {
    myFrameStatsPrs.Update (myWorkspace);
    myFrameStatsPrs.Render (myWorkspace);
  }
}

// =======================================================================
// function : Invalidate
// purpose  :
// =======================================================================
void OpenGl_View::Invalidate()
{
  myBackBufferRestored = Standard_False;
}

//=======================================================================
//function : renderScene
//purpose  :
//=======================================================================
void OpenGl_View::renderScene (Graphic3d_Camera::Projection theProjection,
                               OpenGl_FrameBuffer*          theReadDrawFbo,
                               OpenGl_FrameBuffer*          theOitAccumFbo,
                               const Standard_Boolean       theToDrawImmediate)
{
  const Handle(OpenGl_Context)& aContext = myWorkspace->GetGlContext();

  // Specify clipping planes in view transformation space
  aContext->ChangeClipping().Reset (myClipPlanes);
  if (!myClipPlanes.IsNull()
   && !myClipPlanes->IsEmpty())
  {
    aContext->ShaderManager()->UpdateClippingState();
  }

  renderStructs (theProjection, theReadDrawFbo, theOitAccumFbo, theToDrawImmediate);
  aContext->BindTextures (Handle(OpenGl_TextureSet)());

  // Apply restored view matrix.
  aContext->ApplyWorldViewMatrix();

  aContext->ChangeClipping().Reset (Handle(Graphic3d_SequenceOfHClipPlane)());
  if (!myClipPlanes.IsNull()
   && !myClipPlanes->IsEmpty())
  {
    aContext->ShaderManager()->RevertClippingState();
  }
}

// =======================================================================
// function : bindDefaultFbo
// purpose  :
// =======================================================================
void OpenGl_View::bindDefaultFbo (OpenGl_FrameBuffer* theCustomFbo)
{
  Handle(OpenGl_Context) aCtx = myWorkspace->GetGlContext();
  OpenGl_FrameBuffer* anFbo = (theCustomFbo != NULL && theCustomFbo->IsValid())
                            ?  theCustomFbo
                            : (!aCtx->DefaultFrameBuffer().IsNull()
                             && aCtx->DefaultFrameBuffer()->IsValid()
                              ? aCtx->DefaultFrameBuffer().operator->()
                              : NULL);
  if (anFbo != NULL)
  {
    anFbo->BindBuffer (aCtx);
    anFbo->SetupViewport (aCtx);
  }
  else
  {
  #if !defined(GL_ES_VERSION_2_0)
    aCtx->SetReadDrawBuffer (GL_BACK);
  #else
    if (aCtx->arbFBO != NULL)
    {
      aCtx->arbFBO->glBindFramebuffer (GL_FRAMEBUFFER, OpenGl_FrameBuffer::NO_FRAMEBUFFER);
    }
  #endif
    const Standard_Integer aViewport[4] = { 0, 0, myWindow->Width(), myWindow->Height() };
    aCtx->ResizeViewport (aViewport);
  }
}

// =======================================================================
// function : initBlitQuad
// purpose  :
// =======================================================================
OpenGl_VertexBuffer* OpenGl_View::initBlitQuad (const Standard_Boolean theToFlip)
{
  OpenGl_VertexBuffer* aVerts = NULL;
  if (!theToFlip)
  {
    aVerts = &myFullScreenQuad;
    if (!aVerts->IsValid())
    {
      OpenGl_Vec4 aQuad[4] =
      {
        OpenGl_Vec4( 1.0f, -1.0f, 1.0f, 0.0f),
        OpenGl_Vec4( 1.0f,  1.0f, 1.0f, 1.0f),
        OpenGl_Vec4(-1.0f, -1.0f, 0.0f, 0.0f),
        OpenGl_Vec4(-1.0f,  1.0f, 0.0f, 1.0f)
      };
      aVerts->Init (myWorkspace->GetGlContext(), 4, 4, aQuad[0].GetData());
    }
  }
  else
  {
    aVerts = &myFullScreenQuadFlip;
    if (!aVerts->IsValid())
    {
      OpenGl_Vec4 aQuad[4] =
      {
        OpenGl_Vec4( 1.0f, -1.0f, 1.0f, 1.0f),
        OpenGl_Vec4( 1.0f,  1.0f, 1.0f, 0.0f),
        OpenGl_Vec4(-1.0f, -1.0f, 0.0f, 1.0f),
        OpenGl_Vec4(-1.0f,  1.0f, 0.0f, 0.0f)
      };
      aVerts->Init (myWorkspace->GetGlContext(), 4, 4, aQuad[0].GetData());
    }
  }
  return aVerts;
}

// =======================================================================
// function : blitBuffers
// purpose  :
// =======================================================================
bool OpenGl_View::blitBuffers (OpenGl_FrameBuffer*    theReadFbo,
                               OpenGl_FrameBuffer*    theDrawFbo,
                               const Standard_Boolean theToFlip)
{
  Handle(OpenGl_Context) aCtx = myWorkspace->GetGlContext();
  const Standard_Integer aReadSizeX = theReadFbo != NULL ? theReadFbo->GetVPSizeX() : myWindow->Width();
  const Standard_Integer aReadSizeY = theReadFbo != NULL ? theReadFbo->GetVPSizeY() : myWindow->Height();
  const Standard_Integer aDrawSizeX = theDrawFbo != NULL ? theDrawFbo->GetVPSizeX() : myWindow->Width();
  const Standard_Integer aDrawSizeY = theDrawFbo != NULL ? theDrawFbo->GetVPSizeY() : myWindow->Height();
  if (theReadFbo == NULL || aCtx->IsFeedback())
  {
    return false;
  }
  else if (theReadFbo == theDrawFbo)
  {
    return true;
  }

  // clear destination before blitting
  if (theDrawFbo != NULL
  &&  theDrawFbo->IsValid())
  {
    theDrawFbo->BindBuffer (aCtx);
  }
  else
  {
    aCtx->arbFBO->glBindFramebuffer (GL_FRAMEBUFFER, OpenGl_FrameBuffer::NO_FRAMEBUFFER);
  }
  const Standard_Integer aViewport[4] = { 0, 0, aDrawSizeX, aDrawSizeY };
  aCtx->ResizeViewport (aViewport);

#if !defined(GL_ES_VERSION_2_0)
  aCtx->core20fwd->glClearDepth  (1.0);
#else
  aCtx->core20fwd->glClearDepthf (1.0f);
#endif
  aCtx->core20fwd->glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  if (aCtx->arbFBOBlit != NULL
   && theReadFbo->NbSamples() != 0)
  {
    GLbitfield aCopyMask = 0;
    theReadFbo->BindReadBuffer (aCtx);
    if (theDrawFbo != NULL
     && theDrawFbo->IsValid())
    {
      theDrawFbo->BindDrawBuffer (aCtx);
      if (theDrawFbo->HasColor()
       && theReadFbo->HasColor())
      {
        aCopyMask |= GL_COLOR_BUFFER_BIT;
      }
      if (theDrawFbo->HasDepth()
       && theReadFbo->HasDepth())
      {
        aCopyMask |= GL_DEPTH_BUFFER_BIT;
      }
    }
    else
    {
      if (theReadFbo->HasColor())
      {
        aCopyMask |= GL_COLOR_BUFFER_BIT;
      }
      if (theReadFbo->HasDepth())
      {
        aCopyMask |= GL_DEPTH_BUFFER_BIT;
      }
      aCtx->arbFBO->glBindFramebuffer (GL_DRAW_FRAMEBUFFER, OpenGl_FrameBuffer::NO_FRAMEBUFFER);
    }

    // we don't copy stencil buffer here... does it matter for performance?
    aCtx->arbFBOBlit->glBlitFramebuffer (0, 0, aReadSizeX, aReadSizeY,
                                         0, 0, aDrawSizeX, aDrawSizeY,
                                         aCopyMask, GL_NEAREST);
    const int anErr = ::glGetError();
    if (anErr != GL_NO_ERROR)
    {
      // glBlitFramebuffer() might fail in several cases:
      // - Both FBOs have MSAA and they are samples number does not match.
      //   OCCT checks that this does not happen,
      //   however some graphics drivers provide an option for overriding MSAA.
      //   In this case window MSAA might be non-zero (and application can not check it)
      //   and might not match MSAA of our offscreen FBOs.
      // - Pixel formats of FBOs do not match.
      //   This also might happen with window has pixel format,
      //   e.g. Mesa fails blitting RGBA8 -> RGB8 while other drivers support this conversion.
      TCollection_ExtendedString aMsg = TCollection_ExtendedString() + "FBO blitting has failed [Error #" + anErr + "]\n"
                                      + "  Please check your graphics driver settings or try updating driver.";
      if (theReadFbo->NbSamples() != 0)
      {
        myToDisableMSAA = true;
        aMsg += "\n  MSAA settings should not be overridden by driver!";
      }
      aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION,
                         GL_DEBUG_TYPE_ERROR,
                         0,
                         GL_DEBUG_SEVERITY_HIGH,
                         aMsg);
    }

    if (theDrawFbo != NULL
     && theDrawFbo->IsValid())
    {
      theDrawFbo->BindBuffer (aCtx);
    }
    else
    {
      aCtx->arbFBO->glBindFramebuffer (GL_FRAMEBUFFER, OpenGl_FrameBuffer::NO_FRAMEBUFFER);
    }
  }
  else
  {
    aCtx->core20fwd->glDepthFunc (GL_ALWAYS);
    aCtx->core20fwd->glDepthMask (GL_TRUE);
    aCtx->core20fwd->glEnable (GL_DEPTH_TEST);
  #if defined(GL_ES_VERSION_2_0)
    if (!aCtx->IsGlGreaterEqual (3, 0)
     && !aCtx->extFragDepth)
    {
      aCtx->core20fwd->glDisable (GL_DEPTH_TEST);
    }
  #endif

    aCtx->BindTextures (Handle(OpenGl_TextureSet)());

    const Graphic3d_TypeOfTextureFilter aFilter = (aDrawSizeX == aReadSizeX && aDrawSizeY == aReadSizeY) ? Graphic3d_TOTF_NEAREST : Graphic3d_TOTF_BILINEAR;
    const GLint aFilterGl = aFilter == Graphic3d_TOTF_NEAREST ? GL_NEAREST : GL_LINEAR;

    OpenGl_VertexBuffer* aVerts = initBlitQuad (theToFlip);
    const Handle(OpenGl_ShaderManager)& aManager = aCtx->ShaderManager();
    if (aVerts->IsValid()
     && aManager->BindFboBlitProgram())
    {
      theReadFbo->ColorTexture()->Bind (aCtx, Graphic3d_TextureUnit_0);
      if (theReadFbo->ColorTexture()->Sampler()->Parameters()->Filter() != aFilter)
      {
        theReadFbo->ColorTexture()->Sampler()->Parameters()->SetFilter (aFilter);
        aCtx->core20fwd->glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, aFilterGl);
        aCtx->core20fwd->glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, aFilterGl);
      }

      theReadFbo->DepthStencilTexture()->Bind (aCtx, Graphic3d_TextureUnit_1);
      if (theReadFbo->DepthStencilTexture()->Sampler()->Parameters()->Filter() != aFilter)
      {
        theReadFbo->DepthStencilTexture()->Sampler()->Parameters()->SetFilter (aFilter);
        aCtx->core20fwd->glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, aFilterGl);
        aCtx->core20fwd->glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, aFilterGl);
      }

      aVerts->BindVertexAttrib (aCtx, Graphic3d_TOA_POS);

      aCtx->core20fwd->glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

      aVerts->UnbindVertexAttrib (aCtx, Graphic3d_TOA_POS);
      theReadFbo->DepthStencilTexture()->Unbind (aCtx, Graphic3d_TextureUnit_1);
      theReadFbo->ColorTexture()       ->Unbind (aCtx, Graphic3d_TextureUnit_0);
      aCtx->BindProgram (NULL);
    }
    else
    {
      TCollection_ExtendedString aMsg = TCollection_ExtendedString()
        + "Error! FBO blitting has failed";
      aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION,
                         GL_DEBUG_TYPE_ERROR,
                         0,
                         GL_DEBUG_SEVERITY_HIGH,
                         aMsg);
      myHasFboBlit = Standard_False;
      theReadFbo->Release (aCtx.operator->());
      return true;
    }
  }
  return true;
}

// =======================================================================
// function : drawStereoPair
// purpose  :
// =======================================================================
void OpenGl_View::drawStereoPair (OpenGl_FrameBuffer* theDrawFbo)
{
  const Handle(OpenGl_Context)& aCtx = myWorkspace->GetGlContext();
  bindDefaultFbo (theDrawFbo);
  OpenGl_FrameBuffer* aPair[2] =
  {
    myImmediateSceneFbos[0]->IsValid() ? myImmediateSceneFbos[0].operator->() : NULL,
    myImmediateSceneFbos[1]->IsValid() ? myImmediateSceneFbos[1].operator->() : NULL
  };
  if (aPair[0] == NULL
  ||  aPair[1] == NULL
  || !myTransientDrawToFront)
  {
    aPair[0] = myMainSceneFbos[0]->IsValid() ? myMainSceneFbos[0].operator->() : NULL;
    aPair[1] = myMainSceneFbos[1]->IsValid() ? myMainSceneFbos[1].operator->() : NULL;
  }

  if (aPair[0] == NULL
   || aPair[1] == NULL)
  {
    return;
  }

  if (aPair[0]->NbSamples() != 0)
  {
    // resolve MSAA buffers before drawing
    if (!myOpenGlFBO ->InitLazy (aCtx, aPair[0]->GetVPSizeX(), aPair[0]->GetVPSizeY(), myFboColorFormat, myFboDepthFormat, 0)
     || !myOpenGlFBO2->InitLazy (aCtx, aPair[0]->GetVPSizeX(), aPair[0]->GetVPSizeY(), myFboColorFormat, 0, 0))
    {
      aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION,
                         GL_DEBUG_TYPE_ERROR,
                         0,
                         GL_DEBUG_SEVERITY_HIGH,
                         "Error! Unable to allocate FBO for blitting stereo pair");
      bindDefaultFbo (theDrawFbo);
      return;
    }

    if (!blitBuffers (aPair[0], myOpenGlFBO .operator->(), Standard_False)
     || !blitBuffers (aPair[1], myOpenGlFBO2.operator->(), Standard_False))
    {
      bindDefaultFbo (theDrawFbo);
      return;
    }

    aPair[0] = myOpenGlFBO .operator->();
    aPair[1] = myOpenGlFBO2.operator->();
    bindDefaultFbo (theDrawFbo);
  }

  struct
  {
    Standard_Integer left;
    Standard_Integer top;
    Standard_Integer right;
    Standard_Integer bottom;
    Standard_Integer dx() { return right  - left; }
    Standard_Integer dy() { return bottom - top; }
  } aGeom;

  myWindow->PlatformWindow()->Position (aGeom.left, aGeom.top, aGeom.right, aGeom.bottom);

  Standard_Boolean toReverse = myRenderParams.ToReverseStereo;
  const Standard_Boolean isOddY = (aGeom.top + aGeom.dy()) % 2 == 1;
  const Standard_Boolean isOddX =  aGeom.left % 2 == 1;
  if (isOddY
   && (myRenderParams.StereoMode == Graphic3d_StereoMode_RowInterlaced
    || myRenderParams.StereoMode == Graphic3d_StereoMode_ChessBoard))
  {
    toReverse = !toReverse;
  }
  if (isOddX
   && (myRenderParams.StereoMode == Graphic3d_StereoMode_ColumnInterlaced
    || myRenderParams.StereoMode == Graphic3d_StereoMode_ChessBoard))
  {
    toReverse = !toReverse;
  }

  if (toReverse)
  {
    std::swap (aPair[0], aPair[1]);
  }

  aCtx->core20fwd->glDepthFunc (GL_ALWAYS);
  aCtx->core20fwd->glDepthMask (GL_TRUE);
  aCtx->core20fwd->glEnable (GL_DEPTH_TEST);

  aCtx->BindTextures (Handle(OpenGl_TextureSet)());
  OpenGl_VertexBuffer* aVerts = initBlitQuad (myToFlipOutput);

  const Handle(OpenGl_ShaderManager)& aManager = aCtx->ShaderManager();
  if (aVerts->IsValid()
   && aManager->BindStereoProgram (myRenderParams.StereoMode))
  {
    if (myRenderParams.StereoMode == Graphic3d_StereoMode_Anaglyph)
    {
      OpenGl_Mat4 aFilterL, aFilterR;
      aFilterL.SetDiagonal (Graphic3d_Vec4 (0.0f, 0.0f, 0.0f, 0.0f));
      aFilterR.SetDiagonal (Graphic3d_Vec4 (0.0f, 0.0f, 0.0f, 0.0f));
      switch (myRenderParams.AnaglyphFilter)
      {
        case Graphic3d_RenderingParams::Anaglyph_RedCyan_Simple:
        {
          aFilterL.SetRow (0, Graphic3d_Vec4 (1.0f, 0.0f, 0.0f, 0.0f));
          aFilterR.SetRow (1, Graphic3d_Vec4 (0.0f, 1.0f, 0.0f, 0.0f));
          aFilterR.SetRow (2, Graphic3d_Vec4 (0.0f, 0.0f, 1.0f, 0.0f));
          break;
        }
        case Graphic3d_RenderingParams::Anaglyph_RedCyan_Optimized:
        {
          aFilterL.SetRow (0, Graphic3d_Vec4 ( 0.4154f,      0.4710f,      0.16666667f, 0.0f));
          aFilterL.SetRow (1, Graphic3d_Vec4 (-0.0458f,     -0.0484f,     -0.0257f,     0.0f));
          aFilterL.SetRow (2, Graphic3d_Vec4 (-0.0547f,     -0.0615f,      0.0128f,     0.0f));
          aFilterL.SetRow (3, Graphic3d_Vec4 ( 0.0f,         0.0f,         0.0f,        0.0f));
          aFilterR.SetRow (0, Graphic3d_Vec4 (-0.01090909f, -0.03636364f, -0.00606061f, 0.0f));
          aFilterR.SetRow (1, Graphic3d_Vec4 ( 0.37560000f,  0.73333333f,  0.01111111f, 0.0f));
          aFilterR.SetRow (2, Graphic3d_Vec4 (-0.06510000f, -0.12870000f,  1.29710000f, 0.0f));
          aFilterR.SetRow (3, Graphic3d_Vec4 ( 0.0f,                0.0f,  0.0f,        0.0f));
          break;
        }
        case Graphic3d_RenderingParams::Anaglyph_YellowBlue_Simple:
        {
          aFilterL.SetRow (0, Graphic3d_Vec4 (1.0f, 0.0f, 0.0f, 0.0f));
          aFilterL.SetRow (1, Graphic3d_Vec4 (0.0f, 1.0f, 0.0f, 0.0f));
          aFilterR.SetRow (2, Graphic3d_Vec4 (0.0f, 0.0f, 1.0f, 0.0f));
          break;
        }
        case Graphic3d_RenderingParams::Anaglyph_YellowBlue_Optimized:
        {
          aFilterL.SetRow (0, Graphic3d_Vec4 ( 1.062f, -0.205f,  0.299f, 0.0f));
          aFilterL.SetRow (1, Graphic3d_Vec4 (-0.026f,  0.908f,  0.068f, 0.0f));
          aFilterL.SetRow (2, Graphic3d_Vec4 (-0.038f, -0.173f,  0.022f, 0.0f));
          aFilterL.SetRow (3, Graphic3d_Vec4 ( 0.0f,    0.0f,    0.0f,   0.0f));
          aFilterR.SetRow (0, Graphic3d_Vec4 (-0.016f, -0.123f, -0.017f, 0.0f));
          aFilterR.SetRow (1, Graphic3d_Vec4 ( 0.006f,  0.062f, -0.017f, 0.0f));
          aFilterR.SetRow (2, Graphic3d_Vec4 ( 0.094f,  0.185f,  0.911f, 0.0f));
          aFilterR.SetRow (3, Graphic3d_Vec4 ( 0.0f,    0.0f,    0.0f,   0.0f));
          break;
        }
        case Graphic3d_RenderingParams::Anaglyph_GreenMagenta_Simple:
        {
          aFilterR.SetRow (0, Graphic3d_Vec4 (1.0f, 0.0f, 0.0f, 0.0f));
          aFilterL.SetRow (1, Graphic3d_Vec4 (0.0f, 1.0f, 0.0f, 0.0f));
          aFilterR.SetRow (2, Graphic3d_Vec4 (0.0f, 0.0f, 1.0f, 0.0f));
          break;
        }
        case Graphic3d_RenderingParams::Anaglyph_UserDefined:
        {
          aFilterL = myRenderParams.AnaglyphLeft;
          aFilterR = myRenderParams.AnaglyphRight;
          break;
        }
      }
      aCtx->ActiveProgram()->SetUniform (aCtx, "uMultL", aFilterL);
      aCtx->ActiveProgram()->SetUniform (aCtx, "uMultR", aFilterR);
    }

    aPair[0]->ColorTexture()->Bind (aCtx, Graphic3d_TextureUnit_0);
    aPair[1]->ColorTexture()->Bind (aCtx, Graphic3d_TextureUnit_1);
    aVerts->BindVertexAttrib (aCtx, 0);

    aCtx->core20fwd->glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

    aVerts->UnbindVertexAttrib (aCtx, 0);
    aPair[1]->ColorTexture()->Unbind (aCtx, Graphic3d_TextureUnit_1);
    aPair[0]->ColorTexture()->Unbind (aCtx, Graphic3d_TextureUnit_0);
  }
  else
  {
    TCollection_ExtendedString aMsg = TCollection_ExtendedString()
      + "Error! Anaglyph has failed";
    aCtx->PushMessage (GL_DEBUG_SOURCE_APPLICATION,
                       GL_DEBUG_TYPE_ERROR,
                       0,
                       GL_DEBUG_SEVERITY_HIGH,
                       aMsg);
  }
}

// =======================================================================
// function : copyBackToFront
// purpose  :
// =======================================================================
bool OpenGl_View::copyBackToFront()
{
  myIsImmediateDrawn = Standard_False;
#if !defined(GL_ES_VERSION_2_0)
  const Handle(OpenGl_Context)& aCtx = myWorkspace->GetGlContext();
  if (aCtx->core11 == NULL)
  {
    return false;
  }

  OpenGl_Mat4 aProjectMat;
  Graphic3d_TransformUtils::Ortho2D (aProjectMat,
                                     0.0f, static_cast<GLfloat> (myWindow->Width()),
                                     0.0f, static_cast<GLfloat> (myWindow->Height()));

  aCtx->WorldViewState.Push();
  aCtx->ProjectionState.Push();

  aCtx->WorldViewState.SetIdentity();
  aCtx->ProjectionState.SetCurrent (aProjectMat);

  aCtx->ApplyProjectionMatrix();
  aCtx->ApplyWorldViewMatrix();

  // synchronize FFP state before copying pixels
  aCtx->BindProgram (Handle(OpenGl_ShaderProgram)());
  aCtx->ShaderManager()->PushState (Handle(OpenGl_ShaderProgram)());
  aCtx->DisableFeatures();

  switch (aCtx->DrawBuffer())
  {
    case GL_BACK_LEFT:
    {
      aCtx->SetReadBuffer (GL_BACK_LEFT);
      aCtx->SetDrawBuffer (GL_FRONT_LEFT);
      break;
    }
    case GL_BACK_RIGHT:
    {
      aCtx->SetReadBuffer (GL_BACK_RIGHT);
      aCtx->SetDrawBuffer (GL_FRONT_RIGHT);
      break;
    }
    default:
    {
      aCtx->SetReadBuffer (GL_BACK);
      aCtx->SetDrawBuffer (GL_FRONT);
      break;
    }
  }

  aCtx->core11->glRasterPos2i (0, 0);
  aCtx->core11->glCopyPixels  (0, 0, myWindow->Width() + 1, myWindow->Height() + 1, GL_COLOR);
  //aCtx->core11->glCopyPixels  (0, 0, myWidth + 1, myHeight + 1, GL_DEPTH);

  aCtx->EnableFeatures();

  aCtx->WorldViewState.Pop();
  aCtx->ProjectionState.Pop();
  aCtx->ApplyProjectionMatrix();

  // read/write from front buffer now
  aCtx->SetReadBuffer (aCtx->DrawBuffer());
  return true;
#else
  return false;
#endif
}

// =======================================================================
// function : checkOitCompatibility
// purpose  :
// =======================================================================
Standard_Boolean OpenGl_View::checkOitCompatibility (const Handle(OpenGl_Context)& theGlContext,
                                                     const Standard_Boolean theMSAA)
{
  // determine if OIT is supported by current OpenGl context
  Standard_Boolean& aToDisableOIT = theMSAA ? myToDisableMSAA : myToDisableOIT;
  if (aToDisableOIT)
  {
    return Standard_False;
  }

  TCollection_ExtendedString aCompatibilityMsg;
  if (theGlContext->hasFloatBuffer     == OpenGl_FeatureNotAvailable
   && theGlContext->hasHalfFloatBuffer == OpenGl_FeatureNotAvailable)
  {
    aCompatibilityMsg += "OpenGL context does not support floating-point RGBA color buffer format.\n";
  }
  if (theMSAA && theGlContext->hasSampleVariables == OpenGl_FeatureNotAvailable)
  {
    aCompatibilityMsg += "Current version of GLSL does not support built-in sample variables.\n";
  }
  if (theGlContext->hasDrawBuffers == OpenGl_FeatureNotAvailable)
  {
    aCompatibilityMsg += "OpenGL context does not support multiple draw buffers.\n";
  }
  if (aCompatibilityMsg.IsEmpty())
  {
    return Standard_True;
  }

  aCompatibilityMsg += "  Blended order-independent transparency will not be available.\n";
  theGlContext->PushMessage (GL_DEBUG_SOURCE_APPLICATION,
                          GL_DEBUG_TYPE_ERROR,
                          0,
                          GL_DEBUG_SEVERITY_HIGH,
                          aCompatibilityMsg);

  aToDisableOIT = Standard_True;
  return Standard_False;
}

// =======================================================================
// function : chooseOitColorConfiguration
// purpose  :
// =======================================================================
bool OpenGl_View::chooseOitColorConfiguration (const Handle(OpenGl_Context)& theGlContext,
                                               const Standard_Integer theConfigIndex,
                                               OpenGl_ColorFormats& theFormats)
{
  theFormats.Clear();
  switch (theConfigIndex)
  {
    case 0: // choose best applicable color format combination
    {
      theFormats.Append (theGlContext->hasHalfFloatBuffer != OpenGl_FeatureNotAvailable ? GL_RGBA16F : GL_RGBA32F);
      theFormats.Append (theGlContext->hasHalfFloatBuffer != OpenGl_FeatureNotAvailable ? GL_R16F    : GL_R32F);
      return true;
    }
    case 1: // choose non-optimal applicable color format combination
    {
      theFormats.Append (theGlContext->hasHalfFloatBuffer != OpenGl_FeatureNotAvailable ? GL_RGBA16F : GL_RGBA32F);
      theFormats.Append (theGlContext->hasHalfFloatBuffer != OpenGl_FeatureNotAvailable ? GL_RGBA16F : GL_RGBA32F);
      return true;
    }
  }
  return false; // color combination does not exist
}
