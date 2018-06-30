//
// App.cpp
// Implementation of the App class.
//

#include "App.h"

#include "OcctAspectWindow.h"

#include <OpenGl_GraphicDriver.hxx>
#include <OSD_Environment.hxx>
#include <NCollection_String.hxx>
#include <Font_FontMgr.hxx>

#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>

using namespace uwp;

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// Implementation of the IFrameworkViewSource interface, necessary to run our app.
ref class SimpleApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
  virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
  {
    return ref new App();
  }
};

// The main function creates an IFrameworkViewSource for our app, and runs the app.
[Platform::MTAThread]
int main (Platform::Array<Platform::String^>^)
{
  auto simpleApplicationSource = ref new SimpleApplicationSource();
  Windows::ApplicationModel::Core::CoreApplication::Run(simpleApplicationSource);
  return 0;
}

// =======================================================================
// function : App
// purpose  :
// =======================================================================
App::App()
: myEglDisplay (EGL_NO_DISPLAY),
  myEglContext (EGL_NO_CONTEXT),
  myEglSurface (EGL_NO_SURFACE),
  myEglConfig  (nullptr),
  myPanelWidth (0),
  myPanelHeight(0),
  myIsWindowClosed (false),
  myIsWindowVisible(true)
{
  //
}

// =======================================================================
// function : Initialize
// purpose  :
// =======================================================================
void App::Initialize (Windows::ApplicationModel::Core::CoreApplicationView^ theAppView)
{
  Windows::Storage::StorageFolder^ anAppFolderRef = Windows::ApplicationModel::Package::Current->InstalledLocation;
  const NCollection_String anAppFolder (anAppFolderRef->Path->Data());
  const TCollection_AsciiString anAssetsFolder = (anAppFolder + "\\Assets").ToCString();

  // assign paths to OCCT file resources
  OSD_Environment aShadersDir ("CSF_ShadersDirectory");
  aShadersDir.SetValue (anAssetsFolder);
  aShadersDir.Build();

  // register custom fonts - system fonts are unavailable for OCCT within UWP
  TCollection_AsciiString aFontPath = anAssetsFolder + "\\OpenSans-Regular.ttf";
  Handle(Font_FontMgr)    aMgr = Font_FontMgr::GetInstance();
  Handle(Font_SystemFont) aFont = aMgr->CheckFont (aFontPath.ToCString());
  if (aFont.IsNull())
  {
    throw Exception::CreateException(E_FAIL, L"Error: font can not be loaded!");
  }
  else
  {
    aFont = new Font_SystemFont(new TCollection_HAsciiString("Courier"), Font_FA_Regular, new TCollection_HAsciiString (aFontPath));
    aMgr->RegisterFont(aFont, Standard_True);
  }

  // Register event handlers for app lifecycle. This example includes Activated,
  // so that we can make the CoreWindow active and start rendering on the window.
  theAppView->Activated += ref new Windows::Foundation::TypedEventHandler <Windows::ApplicationModel::Core::CoreApplicationView^, IActivatedEventArgs^> (this, &App::onActivated);

  // Logic for other event handlers could go here.
  // Information about the Suspending and Resuming event handlers can be found here:
  // http://msdn.microsoft.com/en-us/library/windows/apps/xaml/hh994930.aspx
}

// =======================================================================
// function : SetWindow
// purpose  :
// =======================================================================
void App::SetWindow (Windows::UI::Core::CoreWindow^ theWindow)
{
  theWindow->VisibilityChanged += ref new TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::VisibilityChangedEventArgs^> (this, &App::onVisibilityChanged);
  theWindow->Closed            += ref new TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::CoreWindowEventArgs^>        (this, &App::onWindowClosed);

  // The CoreWindow has been created, so EGL can be initialized.
  initializeEGL (theWindow);
}

// =======================================================================
// function : Load
// purpose  :
// =======================================================================
void App::Load (Platform::String^ theEntryPoint)
{
  recreateRenderer();
}

// =======================================================================
// function : recreateRenderer
// purpose  :
// =======================================================================
void App::recreateRenderer()
{
  if (myEglContext == EGL_NO_CONTEXT)
  {
    return;
  }

  eglQuerySurface (myEglDisplay, myEglSurface, EGL_WIDTH,  &myPanelWidth);
  eglQuerySurface (myEglDisplay, myEglSurface, EGL_HEIGHT, &myPanelHeight);
  if (!myViewer.IsNull())
  {
    Handle(OpenGl_GraphicDriver) aDriver = Handle(OpenGl_GraphicDriver)::DownCast(myViewer->Driver());
    Handle(OcctAspectWindow)     aWindow = Handle(OcctAspectWindow)::DownCast(myView->Window());
    if (!aDriver->InitEglContext (myEglDisplay, myEglContext, myEglConfig))
    {
      throw Exception::CreateException (E_FAIL, L"Error: OpenGl_GraphicDriver can not be initialized!");
    }

    aWindow->SetSize (myPanelWidth, myPanelHeight);
    myView->SetWindow (aWindow, (Aspect_RenderingContext )myEglContext);
    return;
  }

  Handle(OpenGl_GraphicDriver) aDriver = new OpenGl_GraphicDriver (Handle(Aspect_DisplayConnection)(), Standard_False);
  aDriver->ChangeOptions().buffersNoSwap = true;
  if (!aDriver->InitEglContext (myEglDisplay, myEglContext, myEglConfig))
  {
    throw Exception::CreateException(E_FAIL, L"Error: OpenGl_GraphicDriver can not be initialized!");
  }

  myViewer = new V3d_Viewer (aDriver, TCollection_ExtendedString("Viewer").ToExtString(), "", 1000.0,
                             V3d_XposYnegZpos, Quantity_NOC_BLACK, V3d_ZBUFFER, V3d_GOURAUD, V3d_WAIT,
                             Standard_False, Standard_False);
  myViewer->SetDefaultLights();

  Handle(OcctAspectWindow) aWindow = new OcctAspectWindow (myPanelWidth, myPanelHeight);
  myView = myViewer->CreateView();
  myView->SetImmediateUpdate (Standard_False);
  myView->TriedronDisplay (Aspect_TOTP_RIGHT_LOWER, Quantity_NOC_WHITE, 0.08, V3d_ZBUFFER);
  myView->SetWindow (aWindow, (Aspect_RenderingContext )myEglContext);

  myContext = new AIS_InteractiveContext(myViewer);
  myContext->SetDisplayMode (AIS_Shaded);


  TopoDS_Shape theBox = BRepPrimAPI_MakeBox (200, 60, 60);
  Handle(AIS_Shape) aShape = new AIS_Shape (theBox);
  myContext->Display (aShape);
  myView->FitAll();
}

// =======================================================================
// function : Run
// purpose  :
// =======================================================================
void App::Run()
{
  while (!myIsWindowClosed)
  {
    if (myIsWindowVisible)
    {
      Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents (Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);

      eglQuerySurface (myEglDisplay, myEglSurface, EGL_WIDTH,  &myPanelWidth);
      eglQuerySurface (myEglDisplay, myEglSurface, EGL_HEIGHT, &myPanelHeight);

      if (!myView.IsNull())
      {
        Handle(OcctAspectWindow) aWindow = Handle(OcctAspectWindow)::DownCast(myView->Window());
        if (aWindow->SetSize (myPanelWidth, myPanelHeight))
        {
          myView->MustBeResized();
          myView->Invalidate();
        }

        myView->Redraw();
      }

//      ::glViewport (0, 0, myPanelWidth, myPanelHeight);
//      ::glClearColor (1.0f, 0.0f, 0.0f, 1.0f);
//      ::glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // The call to eglSwapBuffers might not be successful (e.g. due to Device Lost)
      // If the call fails, then we must reinitialize EGL and the GL resources.
      if (eglSwapBuffers (myEglDisplay, myEglSurface) != EGL_TRUE)
      {
        cleanupEGL();
        initializeEGL (Windows::UI::Core::CoreWindow::GetForCurrentThread());
        recreateRenderer();
      }
    }
    else
    {
      Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents (Windows::UI::Core::CoreProcessEventsOption::ProcessOneAndAllPending);
    }
  }

  cleanupEGL();
}

// =======================================================================
// function : Uninitialize
// purpose  :
// =======================================================================
void App::Uninitialize()
{
}

// =======================================================================
// function : onActivated
// purpose  :
// =======================================================================
void App::onActivated (Windows::ApplicationModel::Core::CoreApplicationView^ theAppView,
                       IActivatedEventArgs^ theArgs)
{
  // Run() won't start until the CoreWindow is activated.
  Windows::UI::Core::CoreWindow::GetForCurrentThread()->Activate();
}

// =======================================================================
// function : onVisibilityChanged
// purpose  :
// =======================================================================
void App::onVisibilityChanged (Windows::UI::Core::CoreWindow^ theSender,
                               Windows::UI::Core::VisibilityChangedEventArgs^ theArgs)
{
  myIsWindowVisible = theArgs->Visible;
}

// =======================================================================
// function : onWindowClosed
// purpose  :
// =======================================================================
void App::onWindowClosed (Windows::UI::Core::CoreWindow^ theSender,
                          Windows::UI::Core::CoreWindowEventArgs^ theArgs)
{
  myIsWindowClosed = true;
}

// =======================================================================
// function : initializeEGL
// purpose  :
// =======================================================================
void App::initializeEGL (Windows::UI::Core::CoreWindow^ theWindow)
{
  // this code reproduces OpenGlesSample project template
  const EGLint aConfigAttributes[] =
  {
    EGL_RED_SIZE,     8,
    EGL_GREEN_SIZE,   8,
    EGL_BLUE_SIZE,    8,
    EGL_ALPHA_SIZE,   0,
    EGL_DEPTH_SIZE,   24,
    EGL_STENCIL_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_NONE
  };

  const EGLint aCtxAttributes[] =
  {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };

  const EGLint aSurfAttributes[] =
  {
  #ifdef EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER
    // EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER is part of the same optimization as EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER (see above).
    // If you have compilation issues with it then please update your Visual Studio templates.
    EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER, EGL_TRUE,
  #endif
    EGL_NONE
  };

  const EGLint aDefaultDisplayAttributes[] =
  {
    // These are the default display attributes, used to request ANGLE's D3D11 renderer.
    // eglInitialize will only succeed with these attributes if the hardware supports D3D11 Feature Level 10_0+.
    EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,

  #ifdef EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER
    // EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER is an optimization that can have large performance benefits on mobile devices.
    // Its syntax is subject to change, though. Please update your Visual Studio templates if you experience compilation issues with it.
    EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
  #endif

    // EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE is an option that enables ANGLE to automatically call 
    // the IDXGIDevice3::Trim method on behalf of the application when it gets suspended. 
    // Calling IDXGIDevice3::Trim when an application is suspended is a Windows Store application certification requirement.
    EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
    EGL_NONE,
  };

  const EGLint fl9_3DisplayAttributes[] =
  {
    // These can be used to request ANGLE's D3D11 renderer, with D3D11 Feature Level 9_3.
    // These attributes are used if the call to eglInitialize fails with the default display attributes.
    EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
    EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, 9,
    EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, 3,
  #ifdef EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER
    EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
  #endif
    EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
    EGL_NONE,
  };

  const EGLint aWarpDisplayAttributes[] =
  {
    // These attributes can be used to request D3D11 WARP.
    // They are used if eglInitialize fails with both the default display attributes and the 9_3 display attributes.
    EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
    EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
  #ifdef EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER
    EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
  #endif
    EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
    EGL_NONE,
  };

  myEglConfig = NULL;

  // eglGetPlatformDisplayEXT is an alternative to eglGetDisplay. It allows us to pass in display attributes, used to configure D3D11.
  PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress ("eglGetPlatformDisplayEXT"));
  if (eglGetPlatformDisplayEXT == nullptr)
  {
    throw Exception::CreateException (E_FAIL, L"Failed to get function eglGetPlatformDisplayEXT");
  }

  //
  // To initialize the display, we make three sets of calls to eglGetPlatformDisplayEXT and eglInitialize, with varying 
  // parameters passed to eglGetPlatformDisplayEXT:
  // 1) The first calls uses "aDefaultDisplayAttributes" as a parameter. This corresponds to D3D11 Feature Level 10_0+.
  // 2) If eglInitialize fails for step 1 (e.g. because 10_0+ isn't supported by the default GPU), then we try again 
  //    using "fl9_3DisplayAttributes". This corresponds to D3D11 Feature Level 9_3.
  // 3) If eglInitialize fails for step 2 (e.g. because 9_3+ isn't supported by the default GPU), then we try again 
  //    using "aWarpDisplayAttributes".  This corresponds to D3D11 Feature Level 11_0 on WARP, a D3D11 software rasterizer.
  //

  // This tries to initialize EGL to D3D11 Feature Level 10_0+. See above comment for details.
  myEglDisplay = eglGetPlatformDisplayEXT (EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, aDefaultDisplayAttributes);
  if (myEglDisplay == EGL_NO_DISPLAY)
  {
    throw Exception::CreateException (E_FAIL, L"Failed to get EGL display");
  }

  if (eglInitialize (myEglDisplay, NULL, NULL) == EGL_FALSE)
  {
    // This tries to initialize EGL to D3D11 Feature Level 9_3, if 10_0+ is unavailable (e.g. on some mobile devices).
    myEglDisplay = eglGetPlatformDisplayEXT (EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, fl9_3DisplayAttributes);
    if (myEglDisplay == EGL_NO_DISPLAY)
    {
      throw Exception::CreateException (E_FAIL, L"Failed to get EGL display");
    }

    if (eglInitialize (myEglDisplay, NULL, NULL) == EGL_FALSE)
    {
      // This initializes EGL to D3D11 Feature Level 11_0 on WARP, if 9_3+ is unavailable on the default GPU.
      myEglDisplay = eglGetPlatformDisplayEXT (EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, aWarpDisplayAttributes);
      if (myEglDisplay == EGL_NO_DISPLAY)
      {
        throw Exception::CreateException (E_FAIL, L"Failed to get EGL display");
      }

      if (eglInitialize (myEglDisplay, NULL, NULL) == EGL_FALSE)
      {
        // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
        throw Exception::CreateException (E_FAIL, L"Failed to initialize EGL");
      }
    }
  }

  EGLint aNbConfigs = 0;
  if (eglChooseConfig (myEglDisplay, aConfigAttributes, &myEglConfig, 1, &aNbConfigs) == EGL_FALSE
   || aNbConfigs == 0)
  {
    throw Exception::CreateException (E_FAIL, L"Failed to choose first EGLConfig");
  }

  // Create a PropertySet and initialize with the EGLNativeWindowType.
  PropertySet^ aSurfProps = ref new PropertySet();
  aSurfProps->Insert (ref new String(EGLNativeWindowTypeProperty), theWindow);

  // You can configure the surface to render at a lower resolution and be scaled up to 
  // the full window size. The scaling is often free on mobile hardware.
  //
  // One way to configure the SwapChainPanel is to specify precisely which resolution it should render at.
  // Size customRenderSurfaceSize = Size(800, 600);
  // aSurfProps->Insert (ref new String(EGLRenderSurfaceSizeProperty), PropertyValue::CreateSize (customRenderSurfaceSize));
  //
  // Another way is to tell the SwapChainPanel to render at a certain scale factor compared to its size.
  // e.g. if the SwapChainPanel is 1920x1280 then setting a factor of 0.5f will make the app render at 960x640
  // float customResolutionScale = 0.5f;
  // aSurfProps->Insert (ref new String(EGLRenderResolutionScaleProperty), PropertyValue::CreateSingle (customResolutionScale));
  myEglSurface = eglCreateWindowSurface (myEglDisplay, myEglConfig, reinterpret_cast<IInspectable*>(aSurfProps), aSurfAttributes);
  if (myEglSurface == EGL_NO_SURFACE)
  {
    throw Exception::CreateException(E_FAIL, L"Failed to create EGL fullscreen surface");
  }

  myEglContext = eglCreateContext (myEglDisplay, myEglConfig, EGL_NO_CONTEXT, aCtxAttributes);
  if (myEglContext == EGL_NO_CONTEXT)
  {
    throw Exception::CreateException (E_FAIL, L"Failed to create EGL context");
  }

  if (eglMakeCurrent (myEglDisplay, myEglSurface, myEglSurface, myEglContext) == EGL_FALSE)
  {
    throw Exception::CreateException (E_FAIL, L"Failed to make fullscreen EGLSurface current");
  }
}

// =======================================================================
// function : cleanupEGL
// purpose  :
// =======================================================================
void App::cleanupEGL()
{
  if (myEglDisplay != EGL_NO_DISPLAY
   && myEglSurface != EGL_NO_SURFACE)
  {
    eglDestroySurface (myEglDisplay, myEglSurface);
    myEglSurface = EGL_NO_SURFACE;
  }

  if (myEglDisplay != EGL_NO_DISPLAY
   && myEglContext != EGL_NO_CONTEXT)
  {
    eglDestroyContext (myEglDisplay, myEglContext);
    myEglContext = EGL_NO_CONTEXT;
  }

  if (myEglDisplay != EGL_NO_DISPLAY)
  {
    eglTerminate (myEglDisplay);
    myEglDisplay = EGL_NO_DISPLAY;
  }
}
