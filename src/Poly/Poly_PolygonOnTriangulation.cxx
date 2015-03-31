// Created on: 1996-02-20
// Created by: Laurent PAINNOT
// Copyright (c) 1996-1999 Matra Datavision
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

#include <Poly_PolygonOnTriangulation.ixx>

//=======================================================================
//function : Poly_PolygonOnTriangulation
//purpose  : 
//=======================================================================

Poly_PolygonOnTriangulation::Poly_PolygonOnTriangulation
(const TColStd_Array1OfInteger&    Nodes) :
    myDeflection(0.0),
    myNodes(1, Nodes.Length())
{
  myNodes = Nodes;
}

//=======================================================================
//function : Poly_PolygonOnTriangulation
//purpose  : 
//=======================================================================

Poly_PolygonOnTriangulation::Poly_PolygonOnTriangulation
   (const TColStd_Array1OfInteger&    Nodes, 
    const TColStd_Array1OfReal&       Parameters):
    myDeflection(0.0),
    myNodes(1, Nodes.Length())
{
  myNodes = Nodes;
  myParameters = new TColStd_HArray1OfReal(1, Parameters.Length());
  myParameters->ChangeArray1() = Parameters;
}

//=======================================================================
//function : Deflection
//purpose  : 
//=======================================================================

Standard_Real Poly_PolygonOnTriangulation::Deflection() const 
{
  return myDeflection;
}

//=======================================================================
//function : Deflection
//purpose  : 
//=======================================================================

void Poly_PolygonOnTriangulation::Deflection(const Standard_Real D)
{
  myDeflection  = D;
}

//=======================================================================
//function : Nodes
//purpose  : 
//=======================================================================

const TColStd_Array1OfInteger& Poly_PolygonOnTriangulation::Nodes() const 
{
  return myNodes;
}

//=======================================================================
//function : Node
//purpose  : 
//=======================================================================

Standard_Integer Poly_PolygonOnTriangulation::Node (const Standard_Integer theIndex) const
{
  Standard_OutOfRange_Raise_if ((theIndex < 1 || theIndex > myNodes.Length()),
                                "Poly_PolygonOnTriangulation::Node : index out of range");
  return myNodes.Value (theIndex);
}

//=======================================================================
//function : SetNode
//purpose  : 
//=======================================================================

void Poly_PolygonOnTriangulation::SetNode (const Standard_Integer theIndex, const Standard_Integer theNode)
{
  Standard_OutOfRange_Raise_if ((theIndex < 1 || theIndex > myNodes.Length()),
                                "Poly_PolygonOnTriangulation::SetNode : index out of range");
  myNodes.SetValue (theIndex, theNode);
}

//=======================================================================
//function : HasParameters
//purpose  : 
//=======================================================================

Standard_Boolean Poly_PolygonOnTriangulation::HasParameters() const 
{
  return (!myParameters.IsNull());
}

//=======================================================================
//function : Parameters
//purpose  : 
//=======================================================================

Handle(TColStd_HArray1OfReal) Poly_PolygonOnTriangulation::Parameters() const 
{
  return myParameters;
}

//=======================================================================
//function : Parameter
//purpose  : 
//=======================================================================

Standard_Real Poly_PolygonOnTriangulation::Parameter (const Standard_Integer theIndex) const 
{
  Standard_NullObject_Raise_if (myParameters.IsNull(),
                                "Poly_PolygonOnTriangulation::Parameter : parameters is NULL");
  Standard_OutOfRange_Raise_if ((theIndex < 1 || theIndex > myParameters->Length()),
                                "Poly_PolygonOnTriangulation::Parameter : index out of range");
  return myParameters->Value (theIndex);
}

//=======================================================================
//function : SetParameter
//purpose  : 
//=======================================================================

void Poly_PolygonOnTriangulation::SetParameter (const Standard_Integer theIndex, const Standard_Real theValue)
{
  Standard_NullObject_Raise_if (myParameters.IsNull(),
                                "Poly_PolygonOnTriangulation::Parameter : parameters is NULL");
  Standard_OutOfRange_Raise_if ((theIndex < 1 || theIndex > myParameters->Length()),
                                "Poly_PolygonOnTriangulation::Parameter : index out of range");
  myParameters->SetValue (theIndex, theValue);
}