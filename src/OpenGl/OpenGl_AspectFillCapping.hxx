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

#ifndef _OpenGl_AspectFillCapping_Header
#define _OpenGl_AspectFillCapping_Header

#include <Graphic3d_AspectFillCapping.hxx>
#include <OpenGl_AspectFace.hxx>
#include <OpenGl_Resource.hxx>

//! Gl resource holding particular aspects for filling capping.
class OpenGl_AspectFillCapping : public OpenGl_Resource
{
public:

  //! Create and assign style.
  Standard_EXPORT OpenGl_AspectFillCapping (const Handle(Graphic3d_AspectFillCapping)& theAspect);

  //! Destructor.
  Standard_EXPORT ~OpenGl_AspectFillCapping();

  //! Assign section style.
  Standard_EXPORT void SetAspect (const Handle(Graphic3d_AspectFillCapping)& theAspect);

  //! Returns section style parameters.
  const Handle(Graphic3d_AspectFillCapping)& Aspect() const { return myAspect; }

  //! Release any allocated GL resources.
  Standard_EXPORT virtual void Release (OpenGl_Context* theContext);

  //! Returns true if capping should draw hatch layer.
  Standard_Boolean ToDrawHatch() const 
  {
    return myAspect->ToDrawHatch() 
       && (myAspect->IsStippleHatch()
        || myAspect->IsTextureHatch());
  }

  //! Returns the shading aspect for drawing face of a clipping section itself.
  //! @param theObjectAspect [in] the aspect of an object if it requires combining.
  Standard_EXPORT  const OpenGl_AspectFace* CappingFaceAspect (const OpenGl_AspectFace* theObjectAspect) const;

  //! Returns the shading aspect for drawing hatch layer of a section.
  Standard_EXPORT const OpenGl_AspectFace* HatchingFaceAspect() const;

protected:

  Handle(Graphic3d_AspectFillCapping) myAspect;         //!< Section style settings from application's level.
  mutable OpenGl_AspectFace           myCappingAspect;  //!< GL aspect for shading base layer of a capping section.
  mutable OpenGl_AspectFace           myHatchingAspect; //!< GL aspect for shading hatching layer (additional to base) of a capping section.
  mutable Standard_Size               myHatchingState;

public:

  DEFINE_STANDARD_ALLOC
};

#endif //_OpenGl_AspectFillCapping_Header
