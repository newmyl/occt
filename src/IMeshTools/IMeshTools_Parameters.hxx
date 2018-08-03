// Created on: 2016-04-07
// Copyright (c) 2016 OPEN CASCADE SAS
// Created by: Oleg AGASHIN
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

#ifndef _IMeshTools_Parameters_HeaderFile
#define _IMeshTools_Parameters_HeaderFile

#include <Precision.hxx>

//! Structure storing meshing parameters
struct IMeshTools_Parameters {

  //! Default constructor
  IMeshTools_Parameters ()
    :
    AngleBorder(0.5),
    DeflectionBorder(0.001),
    AngleInterior(-1.0),
    DeflectionInterior(-1.0),
    MinSize (-1.0),
    InParallel (Standard_False),
    Relative (Standard_False),
    InternalVerticesMode (Standard_True),
    ControlSurfaceDeflection (Standard_True),
    CleanModel(Standard_True)
  {
  }

  //! Returns default relation between MinSize and Deflection
  static Standard_Real RelMinSize()
  {
    return 0.1;
  }

  //! Angular deflection distributed on the nodes,
  //! which split edges of the face.
  Standard_Real                                    AngleBorder;

  //! Linear deflection applied to the edges (excluding internal edges)
  Standard_Real                                    DeflectionBorder;
  
  //! Angular deflection distributed on the nodes
  //! placed inside of the face (including internal edges).
  Standard_Real                                    AngleInterior;

  //! Linear deflection for triangles inside of faces
  Standard_Real                                    DeflectionInterior;
  
  //! Minimal allowed size of mesh element
  Standard_Real                                    MinSize;

  //! Switches on/off multi-thread computation
  Standard_Boolean                                 InParallel;

  //! Switches on/off relative computation of edge tolerance<br>
  //! If true, deflection used for the polygonalisation of each edge will be 
  //! <defle> * Size of Edge. The deflection used for the faces will be the 
  //! maximum deflection of their edges.
  Standard_Boolean                                 Relative;

  //! Mode to take or not to take internal face vertices into account
  //! in triangulation process
  Standard_Boolean                                 InternalVerticesMode;

  //! Parameter to check the deviation of triangulation and interior of
  //! the face
  Standard_Boolean                                 ControlSurfaceDeflection;

  //! Cleans temporary data model when algorithm is finished.
  Standard_Boolean                                 CleanModel;
};

#endif
