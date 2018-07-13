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

#ifndef _BRepMesh_MeshBuilder_HeaderFile
#define _BRepMesh_MeshBuilder_HeaderFile

#include <IMeshTools_MeshBuilder.hxx>

//! Builds mesh for each face of shape without triangulation.
//! In case if some faces of shape have already been triangulated
//! checks deflection of existing polygonal model and re-uses it
//! if deflection satisfies the specified parameter. Otherwise
//! nullifies existing triangulation and build triangulation anew.
//!
//! The following statuses are used:
//! Message_Done1 - algorithm has finished without errors.
//! Message_Fail1 - invalid context.
//! Message_Fail2 - algorithm has faced unexpected error.
//! Message_Fail3 - can't heal discrete model.
//! Message_Warn1 - shape contains no objects to mesh.
class BRepMesh_MeshBuilder : public IMeshTools_MeshBuilder
{
public: //! @name mesher API

  //! Default constructor
  Standard_EXPORT BRepMesh_MeshBuilder ();

  //! Default constructor
  Standard_EXPORT BRepMesh_MeshBuilder (const Handle (IMeshTools_Context)& theContext);

  //! Destructor
  Standard_EXPORT virtual ~BRepMesh_MeshBuilder ();

  //! Performs meshing ot the shape.
  Standard_EXPORT virtual void Perform () Standard_OVERRIDE;

  DEFINE_STANDARD_RTTI_INLINE(BRepMesh_MeshBuilder, IMeshTools_MeshBuilder)
};

#endif
