// Created on: 1992-01-22
// Created by: GG 
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

#ifndef _V3d_SpotLight_HeaderFile
#define _V3d_SpotLight_HeaderFile

#include <V3d_PositionLight.hxx>
#include <V3d_TypeOfOrientation.hxx>

//! Creation and modification of a spot.
class V3d_SpotLight : public V3d_PositionLight
{
  DEFINE_STANDARD_RTTIEXT(V3d_SpotLight, V3d_PositionLight)
public:

  //! Creates a light source of the Spot type in the viewer.
  //! The attenuation factor F which determines
  //! the illumination of a surface depends on the following formula :
  //! F = 1/(theConstAttenuation + theLinearAttenuation*Length)
  //! Length is the distance from the source to the surface.
  //! The default values (1.0,0.0) correspond to a minimum
  //! of attenuation.
  //! The concentration factor determines the dispersion
  //! of the light on the surface, the default value
  //! (1.0) corresponds to a minimum of dispersion.
  //! Warning! raises BadValue from V3d  -
  //! If one of the coefficients is not between 0 and 1.
  //! If the lighting angle is <= 0 or > PI.
  Standard_EXPORT V3d_SpotLight (const Handle(V3d_Viewer)& theViewer,
                                 const Standard_Real theX,
                                 const Standard_Real theY,
                                 const Standard_Real theZ,
                                 const V3d_TypeOfOrientation theDirection = V3d_XnegYnegZpos,
                                 const Quantity_Color& theColor = Quantity_NOC_WHITE,
                                 const Standard_Real theConstAttenuation = 1.0,
                                 const Standard_Real theLinearAttenuation = 0.0,
                                 const Standard_Real theConcentration = 1.0,
                                 const Standard_Real theAngle = 0.523599);
  
  //! Creates a light source of the Spot type in the viewer.
  //! theXt, theYt, theZt : Coordinate of light source Target.
  //! theXp, theYp, theZp : Coordinate of light source Position.
  //! The others parameters describe before.
  //! Warning! raises BadValue from V3d  -
  //! If one of the coefficients is not between 0 and 1.
  //! If the lighting angle is <= 0 or > PI.
  Standard_EXPORT V3d_SpotLight (const Handle(V3d_Viewer)& theViewer,
                                 const Standard_Real theXt,
                                 const Standard_Real theYt,
                                 const Standard_Real theZt,
                                 const Standard_Real theXp,
                                 const Standard_Real theYp,
                                 const Standard_Real theZp,
                                 const Quantity_Color& theColor = Quantity_NOC_WHITE,
                                 const Standard_Real theConstAttenuation = 1.0,
                                 const Standard_Real theLinearAttenuation = 0.0,
                                 const Standard_Real theConcentration = 1.0,
                                 const Standard_Real theAngle = 0.523599);

  //! Defines the position of the light source.
  virtual void SetPosition (Standard_Real theX,
                            Standard_Real theY,
                            Standard_Real theZ) Standard_OVERRIDE
  {
    myPosition.x() = theX;
    myPosition.y() = theY;
    myPosition.z() = theZ;
  }

  //! Defines the direction of the light source.
  //! If the normal vector is NULL.
  void SetDirection (Standard_Real theVx,
                     Standard_Real theVy,
                     Standard_Real theVz)
  {
    myDirection.x() = static_cast<Standard_ShortReal> (theVx);
    myDirection.y() = static_cast<Standard_ShortReal> (theVy);
    myDirection.z() = static_cast<Standard_ShortReal> (theVz);
  }

  //! Defines the direction of the light source
  //! according to a predefined directional vector.
  Standard_EXPORT void SetDirection (V3d_TypeOfOrientation theOrientation);

  //! Display the graphic structure of light source
  //! in the chosen view. We have three type of representation
  //! - SIMPLE   : Only the light source is displayed.
  //! - PARTIAL  : The light source and the light space are
  //! displayed.
  //! - COMPLETE : The light source, the light space and the
  //! radius of light space are displayed.
  //! We can choose the "SAMELAST" as parameter of representation
  //! In this case the graphic structure representation will be
  //! the last displayed.
  Standard_EXPORT void Display (const Handle(V3d_View)& theView,
                                const V3d_TypeOfRepresentation theRepresentation) Standard_OVERRIDE;

  //! Returns the direction of the light source defined by theVx, theVy, theVz.
  void Direction (Standard_Real& theVx,
                  Standard_Real& theVy,
                  Standard_Real& theVz) const
  {
    theVx = myDirection.x();
    theVy = myDirection.y();
    theVz = myDirection.z();
  }

  //! Returns the position of the light source.
  virtual void Position (Standard_Real& theX,
                         Standard_Real& theY,
                         Standard_Real& theZ) const Standard_OVERRIDE
  {
    theX = myPosition.x();
    theY = myPosition.y();
    theZ = myPosition.z();
  }

private:

  //! Defines the representation of the spot light source.
  Standard_EXPORT void Symbol (const Handle(Graphic3d_Group)& theSymbol,
                               const Handle(V3d_View)& theView) const Standard_OVERRIDE;
};

DEFINE_STANDARD_HANDLE(V3d_SpotLight, V3d_PositionLight)

#endif // _V3d_SpotLight_HeaderFile
