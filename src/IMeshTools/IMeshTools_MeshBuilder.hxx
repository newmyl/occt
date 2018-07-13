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

#ifndef _IMeshTools_MeshBuilder_HeaderFile
#define _IMeshTools_MeshBuilder_HeaderFile

#include <Message_Algorithm.hxx>
#include <IMeshTools_Context.hxx>
#include <Standard_Type.hxx>

//! Interface class providing API for mesh builder tool.
class IMeshTools_MeshBuilder : public Message_Algorithm
{
public:

  //! Destructor.
  Standard_EXPORT virtual ~IMeshTools_MeshBuilder()
  {
  }

  //! Sets context for algorithm.
  inline void SetContext (const Handle (IMeshTools_Context)& theContext)
  {
    myContext = theContext;
  }

  //! Gets context of algorithm.
  inline const Handle (IMeshTools_Context)& GetContext () const
  {
    return myContext;
  }

  //! Performs meshing ot the shape using current context.
  Standard_EXPORT virtual void Perform () = 0;

  DEFINE_STANDARD_RTTI_INLINE(IMeshTools_MeshBuilder, Message_Algorithm)

protected:

  //! Constructor.
  Standard_EXPORT IMeshTools_MeshBuilder()
  {
  }

  //! Constructor.
  Standard_EXPORT IMeshTools_MeshBuilder (const Handle (IMeshTools_Context)& theContext)
    : myContext(theContext)
  {
  }

private:

  Handle (IMeshTools_Context) myContext;
};

#endif