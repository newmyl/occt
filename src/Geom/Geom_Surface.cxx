// Created on: 1993-03-10
// Created by: JCV
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


#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <Geom_UndefinedDerivative.hxx>
#include <Geom_UndefinedValue.hxx>
#include <gp_GTrsf2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <Standard_NotImplemented.hxx>
#include <Standard_RangeError.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Geom_Surface,Geom_Geometry)

typedef Geom_Surface         Surface;

//=======================================================================
//function : UReversed
//purpose  : 
//=======================================================================

Handle(Geom_Surface) Geom_Surface::UReversed () const
{
  Handle(Geom_Surface) S = Handle(Geom_Surface)::DownCast(Copy());
  S->UReverse();
  return S;
}


//=======================================================================
//function : VReversed
//purpose  : 
//=======================================================================

Handle(Geom_Surface) Geom_Surface::VReversed () const
{
  Handle(Geom_Surface) S = Handle(Geom_Surface)::DownCast(Copy());
  S->VReverse();
  return S;
}


//=======================================================================
//function : TransformParameters
//purpose  : 
//=======================================================================

void Geom_Surface::TransformParameters(Standard_Real& ,
				       Standard_Real& ,
				       const gp_Trsf&  ) const
{
}

//=======================================================================
//function : ParametricTransformation
//purpose  : 
//=======================================================================

gp_GTrsf2d Geom_Surface::ParametricTransformation(const gp_Trsf&) const
{
  gp_GTrsf2d dummy;
  return dummy;
}

//=======================================================================
//function : UPeriod
//purpose  : 
//=======================================================================

Standard_Real Geom_Surface::UPeriod() const
{
  throw Standard_NotImplemented("Geom_Surface::UPeriod");
}


//=======================================================================
//function : VPeriod
//purpose  : 
//=======================================================================

Standard_Real Geom_Surface::VPeriod() const
{
  throw Standard_NotImplemented("Geom_Surface::VPeriod");
}

//=======================================================================
//function : Value
//purpose  : 
//=======================================================================

gp_Pnt  Geom_Surface::Value(const Standard_Real U, 
			    const Standard_Real V)const 
{
  gp_Pnt P;
  D0(U,V,P);
  return P;
}
