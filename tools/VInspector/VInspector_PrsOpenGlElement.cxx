// Created on: 2018-08-16
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2018 OPEN CASCADE SAS
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

#include <inspector/VInspector_PrsOpenGlElement.hxx>

#include <OpenGl_Workspace.hxx>

IMPLEMENT_STANDARD_RTTIEXT(VInspector_PrsOpenGlElement, AIS_InteractiveObject)

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================
VInspector_PrsOpenGlElement::VInspector_PrsOpenGlElement ()
: AIS_InteractiveObject ()
{
}

// =======================================================================
// function : Compute
// purpose :
// =======================================================================
void VInspector_PrsOpenGlElement::Compute (const Handle(PrsMgr_PresentationManager3d)& thePrsMgr,
                                           const Handle(Prs3d_Presentation)& thePrs,
                                           const Standard_Integer /*theMode*/)
{
  thePrs->Clear();

  Handle(OpenGl_Group) aGroup = Handle(OpenGl_Group)::DownCast (thePrs->NewGroup());
  aGroup->SetDestroyElements (Standard_False);
  for (NCollection_List<OpenGl_Element*>::Iterator anElementsIt (myElements); anElementsIt.More(); anElementsIt.Next())
  {
    OpenGl_Element* anElement = anElementsIt.Value();
    aGroup->AddElement (anElement);
  }
  // invalidate bounding box of the scene
  thePrsMgr->StructureManager()->Update();
}
