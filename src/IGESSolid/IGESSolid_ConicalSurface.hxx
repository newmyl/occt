// Created on: 1993-01-09
// Created by: CKY / Contract Toubro-Larsen ( SIVA )
// Copyright (c) 1993-1999 Matra Datavision
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

#ifndef _IGESSolid_ConicalSurface_HeaderFile
#define _IGESSolid_ConicalSurface_HeaderFile

#include <IGESData_IGESEntity.hxx>
class IGESGeom_Point;
class IGESGeom_Direction;


class IGESSolid_ConicalSurface;
DEFINE_STANDARD_HANDLE(IGESSolid_ConicalSurface, IGESData_IGESEntity)

//! defines ConicalSurface, Type <194> Form Number <0,1>
//! in package IGESSolid
//! The right circular conical surface is defined by a
//! point on the axis on the cone, the direction of the axis
//! of the cone, the radius of the cone at the axis point and
//! the cone semi-angle.
class IGESSolid_ConicalSurface : public IGESData_IGESEntity
{
 public:

  IGESSolid_ConicalSurface() {}
  
  //! This method is used to set the fields of the class
  //! ConicalSurface
  //! - aLocation : Location of the point on axis
  //! - anAxis    : Direction of the axis
  //! - aRadius   : Radius at axis point
  //! - anAngle   : Value of semi-angle in degrees (0<angle<90)
  //! - aRefdir   : Reference direction (parametrised surface)
  //! Null if unparametrised surface.
  Standard_EXPORT void Init (const Handle(IGESGeom_Point)& aLocation, const Handle(IGESGeom_Direction)& anAxis, const Standard_Real aRadius, const Standard_Real anAngle, const Handle(IGESGeom_Direction)& aRefdir);
  
  //! returns the location of the point on the axis
  const Handle(IGESGeom_Point) & LocationPoint() const { return theLocationPoint; }
  
  //! returns the direction of the axis
  const Handle(IGESGeom_Direction) & Axis() const { return theAxis; }
  
  //! returns the radius at the axis point
  Standard_Real Radius() const { return theRadius; }
  
  //! returns the semi-angle value
  Standard_Real SemiAngle() const { return theAngle; }
  
  //! returns the reference direction of the conical surface in case
  //! of parametrised surface. For unparametrised surface it returns NULL.
  const Handle(IGESGeom_Direction) & ReferenceDir() const { return theRefDir; }
  
  //! returns True if Form no is 1 else false
  Standard_Boolean IsParametrised() const { return (!theRefDir.IsNull()); }

  Standard_EXPORT virtual void OwnShared(Interface_EntityIterator &theIter) const Standard_OVERRIDE;

  Standard_EXPORT virtual void OwnCheck (const Interface_ShareTool &, const Handle(Interface_Check) &) const Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(IGESSolid_ConicalSurface,IGESData_IGESEntity)

 private:

  Handle(IGESGeom_Point) theLocationPoint;
  Handle(IGESGeom_Direction) theAxis;
  Standard_Real theRadius;
  Standard_Real theAngle;
  Handle(IGESGeom_Direction) theRefDir;
};

#endif // _IGESSolid_ConicalSurface_HeaderFile
