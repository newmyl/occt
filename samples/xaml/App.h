//
// App.h
// Declaration of the App class.
//

#pragma once

// EGL includes
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

// ANGLE include for Windows Store
#include <angle_windowsstore.h>

#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>

namespace uwp
{
  /// <summary>
  /// Provides application-specific behavior to supplement the default Application class.
  /// </summary>
  ref class App sealed : public Windows::ApplicationModel::Core::IFrameworkView
  {
  public: // IFrameworkView Methods

    //! The first method called when the IFrameworkView is being created.
    virtual void Initialize (Windows::ApplicationModel::Core::CoreApplicationView^ theAppView);

    //! Called when the CoreWindow object is created (or re-created).
    virtual void SetWindow (Windows::UI::Core::CoreWindow^ theWindow);

    //! Initializes scene resources
    virtual void Load (Platform::String^ theEntryPoint);

    //! This method is called after the window becomes active.
    virtual void Run();

    //! Terminate events do not cause Uninitialize to be called.
    //! It will be called if your IFrameworkView class is torn down while the app is in the foreground.
    virtual void Uninitialize();

  internal:

    //! Initializes the singleton application object.
    //! This is the first line of authored code executed,
    //! and as such is the logical equivalent of main() or WinMain().
    App();

  private:

    void recreateRenderer();

    //! Application lifecycle event handlers.
    void onActivated (Windows::ApplicationModel::Core::CoreApplicationView^ theAppView,
                      Windows::ApplicationModel::Activation::IActivatedEventArgs^ theArgs);

    //! Window event handlers.
    void onVisibilityChanged (Windows::UI::Core::CoreWindow^ theSender,
                              Windows::UI::Core::VisibilityChangedEventArgs^ theArgs);
    void onWindowClosed (Windows::UI::Core::CoreWindow^ theSender,
                         Windows::UI::Core::CoreWindowEventArgs^ theArgs);

    void initializeEGL (Windows::UI::Core::CoreWindow^ theWindow);
    void cleanupEGL();

  private:

    Handle(AIS_InteractiveContext) myContext;
    Handle(V3d_Viewer) myViewer;
    Handle(V3d_View)   myView;

    EGLDisplay myEglDisplay;
    EGLContext myEglContext;
    EGLSurface myEglSurface;
    EGLConfig  myEglConfig;
    EGLint     myPanelWidth;
    EGLint     myPanelHeight;

    bool myIsWindowClosed;
    bool myIsWindowVisible;

  };
}
