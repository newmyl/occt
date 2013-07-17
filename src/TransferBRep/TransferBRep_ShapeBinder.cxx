// Copyright (c) 1999-2012 OPEN CASCADE SAS
//
// The content of this file is subject to the Open CASCADE Technology Public
// License Version 6.5 (the "License"). You may not use the content of this file
// except in compliance with the License. Please obtain a copy of the License
// at http://www.opencascade.org and read it completely before using this file.
//
// The Initial Developer of the Original Code is Open CASCADE S.A.S., having its
// main offices at: 1, place des Freres Montgolfier, 78280 Guyancourt, France.
//
// The Original Code and all software distributed under the License is
// distributed on an "AS IS" basis, without warranty of any kind, and the
// Initial Developer hereby disclaims all such warranties, including without
// limitation, any warranties of merchantability, fitness for a particular
// purpose or non-infringement. Please see the License for the specific terms
// and conditions governing the rights and limitations under the License.

#include <TransferBRep_ShapeBinder.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <Standard_TypeMismatch.hxx>
#include <Standard_OutOfRange.hxx>
#include <Standard_Type.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_TYPE(TransferBRep_ShapeBinder)
IMPLEMENT_STANDARD_SUPERTYPE_ARRAY()
  STANDARD_TYPE(TransferBRep_BinderOfShape),
  STANDARD_TYPE(Transfer_Binder),
  STANDARD_TYPE(MMgt_TShared),
  STANDARD_TYPE(Standard_Transient),

IMPLEMENT_STANDARD_SUPERTYPE_ARRAY_END()
IMPLEMENT_STANDARD_TYPE_END(TransferBRep_ShapeBinder)


IMPLEMENT_DOWNCAST(TransferBRep_ShapeBinder,Standard_Transient)
IMPLEMENT_STANDARD_RTTI(TransferBRep_ShapeBinder)

TransferBRep_ShapeBinder::TransferBRep_ShapeBinder ()    {  }

    TransferBRep_ShapeBinder::TransferBRep_ShapeBinder (const TopoDS_Shape& shape)
    :  TransferBRep_BinderOfShape (shape)    {  }

    TopAbs_ShapeEnum  TransferBRep_ShapeBinder::ShapeType () const
{
  if (!HasResult()) return TopAbs_SHAPE;
  return  Result().ShapeType();
}

    TopoDS_Vertex  TransferBRep_ShapeBinder::Vertex() const
      {  return TopoDS::Vertex(Result());  }

    TopoDS_Edge  TransferBRep_ShapeBinder::Edge() const
      {  return TopoDS::Edge(Result());  }

    TopoDS_Wire  TransferBRep_ShapeBinder::Wire() const
      {  return TopoDS::Wire(Result());  }

    TopoDS_Face  TransferBRep_ShapeBinder::Face() const
      {  return TopoDS::Face(Result());  }

    TopoDS_Shell  TransferBRep_ShapeBinder::Shell() const
      {  return TopoDS::Shell(Result());  }

    TopoDS_Solid  TransferBRep_ShapeBinder::Solid() const
      {  return TopoDS::Solid(Result());  }

    TopoDS_CompSolid  TransferBRep_ShapeBinder::CompSolid() const
      {  return TopoDS::CompSolid(Result());  }

    TopoDS_Compound  TransferBRep_ShapeBinder::Compound() const
      {  return TopoDS::Compound(Result());  }
