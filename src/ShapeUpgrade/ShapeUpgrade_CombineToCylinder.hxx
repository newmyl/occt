//-Copyright: Open CASCADE 2016
// Created on: 2016-08-01
// Created by: Mikhail Sazonov
// Copyright (c) 2016 OPEN CASCADE SAS
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

#ifndef _ShapeUpgrade_CombineToCylinder_HeaderFile
#define _ShapeUpgrade_CombineToCylinder_HeaderFile

#include <Standard_TypeDef.hxx>
#include <TopoDS_Shape.hxx>

class BRepTools_ReShape;

//! This tool tries to merge planar faces constituting a cylindrical surface into one face.
//! The algorithm checks that all merged faces have smooth change of normal within the given
//! angular tolerance.
class ShapeUpgrade_CombineToCylinder
{
public:
  
  //! empty constructor
  Standard_EXPORT ShapeUpgrade_CombineToCylinder();
  
  //! Sets input shape
  void SetShape(const TopoDS_Shape& theShape)
  {
    myInitShape = theShape;
  }
  
  //! Sets the linear tolerance. Default value is Precision::Confusion().
  void SetLinearTolerance(const Standard_Real theValue)
  {
    myLinTol = theValue;
  }

  //! Sets the angular tolerance. Default value is Precision::Angular().
  void SetAngularTolerance(const Standard_Real theValue)
  {
    myAngTol = theValue;
  }

  //! Builds the resulting shape
  Standard_EXPORT void Build();
  
  //! Returns true if the result shape was successfully built.
  Standard_Boolean IsDone() const
  {
    return myDone;
  }

  //! Returns the result shape.
  const TopoDS_Shape& Shape() const
  {
    return myShape;
  }

  //! Returns the number of new faces built by merging
  Standard_Integer NbNewFaces() const
  {
    return myNbNewFaces;
  }

  //! Returns the number of replaced faces
  Standard_Integer NbReplacedFaces() const
  {
    return myNbReplacedFaces;
  }

private:

  TopoDS_Shape myInitShape;
  Standard_Real myLinTol;
  Standard_Real myAngTol;
  TopoDS_Shape myShape;
  Handle(BRepTools_ReShape) myContext;
  Standard_Integer myNbNewFaces;
  Standard_Integer myNbReplacedFaces;
  Standard_Boolean myDone;

};

#endif // _ShapeUpgrade_CombineToCylinder_HeaderFile
