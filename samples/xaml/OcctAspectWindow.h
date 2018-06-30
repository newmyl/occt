//
// OcctAspectWindow.h
//

#pragma once

#include <Aspect_Window.hxx>

namespace uwp
{

//! This class defines a dummy window.
class OcctAspectWindow : public Aspect_Window
{
  DEFINE_STANDARD_RTTI_INLINE(OcctAspectWindow, Aspect_Window)
public:

  //! Creates a wrapper over existing Window handle
  OcctAspectWindow(const int theWidth, const int theHeight)
  : myWidth (theWidth), myHeight (theHeight) {}

  virtual Aspect_Drawable NativeHandle() const Standard_OVERRIDE { return 0; }
  virtual Aspect_Drawable NativeParentHandle() const Standard_OVERRIDE { return 0; }
  virtual Aspect_FBConfig NativeFBConfig() const Standard_OVERRIDE { return 0; }
  virtual void Destroy() {}
  virtual void Map() const Standard_OVERRIDE {}
  virtual void Unmap() const Standard_OVERRIDE {}
  virtual Aspect_TypeOfResize DoResize() const Standard_OVERRIDE { return Aspect_TOR_UNKNOWN; }
  virtual Standard_Boolean DoMapping() const Standard_OVERRIDE { return Standard_True; }
  virtual Standard_Boolean IsMapped() const Standard_OVERRIDE { return Standard_True; }

  //! Returns The Window RATIO equal to the physical WIDTH/HEIGHT dimensions
  virtual Quantity_Ratio Ratio() const Standard_OVERRIDE { return 1.0; }

  //! Returns The Window POSITION in PIXEL
  virtual void Position (Standard_Integer& X1, Standard_Integer& Y1,
                         Standard_Integer& X2, Standard_Integer& Y2) const Standard_OVERRIDE
  {
    X1 = 0; X2 = myWidth; Y1 = 0; Y2 = myHeight;
  }

  //! Returns The Window SIZE in PIXEL
  virtual void Size (Standard_Integer& theWidth,
                     Standard_Integer& theHeight) const Standard_OVERRIDE
  {
    theWidth  = myWidth;
    theHeight = myHeight;
  }

  //! Set The Window SIZE in PIXEL
  Standard_Boolean SetSize (const Standard_Integer theWidth,
                            const Standard_Integer theHeight)
  {
    if (myWidth  == theWidth
     && myHeight == theHeight)
    {
      return Standard_False;
    }

    myWidth  = theWidth;
    myHeight = theHeight;
    return Standard_True;
  }

private:

  int myWidth;
  int myHeight;

};

DEFINE_STANDARD_HANDLE(OcctAspectWindow, Aspect_Window)

}
