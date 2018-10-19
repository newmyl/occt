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

#ifndef VInspector_PrsOpenGlElement_H
#define VInspector_PrsOpenGlElement_H

#include <Standard.hxx>
#include <Standard_Macro.hxx>

#include <AIS_InteractiveObject.hxx>
#include <NCollection_List.hxx>
#include <OpenGl_Element.hxx>

class OpenGl_Workspace;

DEFINE_STANDARD_HANDLE(VInspector_PrsOpenGlElement, AIS_InteractiveObject)

//! \class VInspector_PrsOpenGlElement
//! Presentation to display container of OpenGl_Element
class VInspector_PrsOpenGlElement : public AIS_InteractiveObject
{
public:

  //! Constructor
  Standard_EXPORT VInspector_PrsOpenGlElement();

  //! Destructor
  ~VInspector_PrsOpenGlElement() { myElements.Clear(); }

  //! Fills internal container of elements
  //! \param theElements elements to be displayed
  void Set (const NCollection_List<OpenGl_Element*>& theElements) { myElements = theElements; }

private:
  //! Calculates the 3D view aPresentation and its updates.
  //! \param thePrsMgr processor for the presentation
  //! \param thePrs presentation
  //! \param theMode presentable object display mode
  Standard_EXPORT virtual void Compute (const Handle(PrsMgr_PresentationManager3d)& thePrsMgr,
                                        const Handle(Prs3d_Presentation)& thePrs,
                                        const Standard_Integer theMode) Standard_OVERRIDE;

  //! Recovers and calculates any sensitive primitive,
  //! aSelection, available in Shape mode, specified by
  //! aMode
  Standard_EXPORT virtual void ComputeSelection (const Handle(SelectMgr_Selection)& theSelection,
                                                 const Standard_Integer theMode) {}

  // OCCT RTTI
  DEFINE_STANDARD_RTTIEXT(VInspector_PrsOpenGlElement, AIS_InteractiveObject)

private:
  NCollection_List<OpenGl_Element*> myElements;
};

#endif
