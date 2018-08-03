// Copyright (c) 2013 OPEN CASCADE SAS
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

#ifndef _BRepMesh_FastDiscret_HeaderFile
#define _BRepMesh_FastDiscret_HeaderFile

#include <Standard.hxx>
#include <Standard_Assert.hxx>

#include <IMeshTools_Parameters.hxx>

class Standard_DEPRECATED("The class BRepMesh_FastDiscret is deprecated. Please use IMeshTools package") BRepMesh_FastDiscret
{
public:
  struct Parameters
  {

    //! Default constructor
    Parameters()
      :
      Angle(0.1),
      Deflection(0.001),
      ReservedField1(-1.0),
      ReservedField2(-1.0),
      MinSize(Precision::Confusion()),
      InParallel(Standard_False),
      Relative(Standard_False),
      InternalVerticesMode(Standard_True),
      ControlSurfaceDeflection(Standard_True),
      AdaptiveMin(Standard_False)
    {
      Standard_STATIC_ASSERT (sizeof(IMeshTools_Parameters) == sizeof(Parameters));
    }

    //! Returns default relation between MinSize and Deflection
    static Standard_Real RelMinSize()
    {
      return 0.1;
    }

    //! Angular deflection distributed on the nodes
    //! placed inside of the face (including internal edges).
    Standard_Real                                    Angle;

    //! Angular deflection distributed on the nodes,
    //! which split edges of the face.
    Standard_Real                                    Deflection;

    //! Linear deflection for triangles inside of faces
    Standard_Real                                    ReservedField1;

    //! Linear deflection applied to the edges (excluding internal edges)
    Standard_Real                                    ReservedField2;

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

    //! Adaptive parametric tolerance flag. <br>
    //! If this flag is set to true the minimal parametric tolerance
    //! is computed taking minimal parametric distance between vertices
    //! into account
    Standard_Boolean                                 AdaptiveMin;

  };
};

#endif
