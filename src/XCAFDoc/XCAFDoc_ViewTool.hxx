// Created on: 2016-10-19
// Created by: Irina KRYLOVA
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

#ifndef _XCAFDoc_ViewTool_HeaderFile
#define _XCAFDoc_ViewTool_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TDF_Attribute.hxx>
#include <Standard_Boolean.hxx>
#include <TDF_LabelSequence.hxx>
#include <Standard_Integer.hxx>

class XCAFDoc_ShapeTool;
class XCAFDoc_DimTolTool;
class TDF_Label;
class Standard_GUID;
class TDF_Attribute;


class XCAFDoc_ViewTool;
DEFINE_STANDARD_HANDLE(XCAFDoc_ViewTool, TDF_Attribute)

//! Provides tools to store and retrieve Views
//! in and from TDocStd_Document
//! Each View contains parts XCAFDoc_View attribute
//! with all information about camera and view window.
//! Also each view contain information of displayed shapes and GDTs
//! as sets of shape and GDT labels.
class XCAFDoc_ViewTool : public TDF_Attribute
{

public:

  
  Standard_EXPORT XCAFDoc_ViewTool();
  
  //! Creates (if not exist) ViewTool.
  Standard_EXPORT static Handle(XCAFDoc_ViewTool) Set (const TDF_Label& L);
  
  Standard_EXPORT static const Standard_GUID& GetID();
  
  //! Returns the label under which Views are stored
  Standard_EXPORT TDF_Label BaseLabel() const;
  
  //! Returns True if label belongs to a View table and
  //! is a View definition
  Standard_EXPORT Standard_Boolean IsView (const TDF_Label& theLabel) const;
  
  //! Returns a sequence of View labels currently stored
  //! in the View table
  Standard_EXPORT void GetViewLabels (TDF_LabelSequence& theLabels) const;
  
  //! Sets a link with GUID
  Standard_EXPORT void SetView (const TDF_LabelSequence& theShapes,
                                const TDF_LabelSequence& theGDTs,
                                const TDF_LabelSequence& theClippingPlanes,
                                const TDF_Label& theViewL) const;

  //! Sets a link with GUID
  Standard_EXPORT void SetView(const TDF_LabelSequence& theShapes,
                               const TDF_LabelSequence& theGDTs,
                               const TDF_Label& theViewL) const;

  //! Set Clipping planes to  given View
  Standard_EXPORT void SetClippingPlanes(const TDF_LabelSequence& theClippingPlaneLabels,
                                         const TDF_Label& theViewL) const;
  Standard_EXPORT void  SetEnabledShapes(const TDF_LabelSequence& theShapesTransparencyLabels,
                                         const TDF_Label& theViewL) const;

  //! Set Notes to  given View
  Standard_EXPORT void SetNotes(const TDF_LabelSequence& theNoteLabels,
                                const TDF_Label& theViewL) const;

  //! Remove View
  Standard_EXPORT void RemoveView(const TDF_Label& theViewL);

  //! Returns all View labels defined for label ShapeL
  Standard_EXPORT Standard_Boolean GetViewLabelsForShape (const TDF_Label& theShapeL, TDF_LabelSequence& theViews) const;

  //! Returns all View labels defined for label GDTL
  Standard_EXPORT Standard_Boolean GetViewLabelsForGDT (const TDF_Label& theGDTL, TDF_LabelSequence& theViews) const;

  //! Returns all View labels defined for label ClippingPlaneL
  Standard_EXPORT Standard_Boolean GetViewLabelsForClippingPlane(const TDF_Label& theClippingPlaneL, TDF_LabelSequence& theViews) const;
  
  //! Returns all View labels defined for label Shapes transparency
  Standard_EXPORT Standard_Boolean GetViewLabelsForEnabledShapesLabel(const TDF_Label& theShapesTransparencyL, TDF_LabelSequence& theViews) const;
  
  //! Returns all View labels defined for label NotesL
  Standard_EXPORT Standard_Boolean GetViewLabelsForNote(const TDF_Label& theNoteL, TDF_LabelSequence& theViews) const;

  //! Adds a view definition to a View table and returns its label
  Standard_EXPORT TDF_Label AddView() ;
  
  //! Returns shape labels defined for label theViewL
  //! Returns False if the theViewL is not in View table
  Standard_EXPORT Standard_Boolean GetRefShapeLabel (const TDF_Label& theViewL, TDF_LabelSequence& theShapeLabels) const;
  
  //! Returns GDT labels defined for label theViewL
  //! Returns False if the theViewL is not in View table
  Standard_EXPORT Standard_Boolean GetRefGDTLabel (const TDF_Label& theViewL, TDF_LabelSequence& theGDTLabels) const;

  //! Returns ClippingPlane labels defined for label theViewL
  //! Returns False if the theViewL is not in View table
  Standard_EXPORT Standard_Boolean GetRefClippingPlaneLabel(const TDF_Label& theViewL, TDF_LabelSequence& theClippingPlaneLabels) const;

  //! Returns shapes transparency labels defined for label theViewL
  //! Returns False if the theViewL is not in View table
  Standard_EXPORT Standard_Boolean GetRefEnabledShapesLabel(const TDF_Label& theViewL, TDF_LabelSequence& theShapesTranspanencyLabels) const;

  //! Returns Note labels defined for label theViewL
  //! Returns False if the theViewL is not in View table
  Standard_EXPORT Standard_Boolean GetRefNoteLabel(const TDF_Label& theViewL, TDF_LabelSequence& theNoteLabels) const;

  //! Returns true if the given View is marked as locked
  Standard_EXPORT Standard_Boolean IsLocked(const TDF_Label& theViewL) const;

  //! Mark the given View as locked
  Standard_EXPORT void Lock(const TDF_Label& theViewL) const;

  //! Unlock the given View
  Standard_EXPORT void Unlock(const TDF_Label& theViewL) const;
  
  Standard_EXPORT const Standard_GUID& ID() const Standard_OVERRIDE;
  
  Standard_EXPORT void Restore (const Handle(TDF_Attribute)& with) Standard_OVERRIDE;
  
  Standard_EXPORT Handle(TDF_Attribute) NewEmpty() const Standard_OVERRIDE;
  
  Standard_EXPORT void Paste (const Handle(TDF_Attribute)& into, const Handle(TDF_RelocationTable)& RT) const Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(XCAFDoc_ViewTool, TDF_Attribute)

};
#endif // _XCAFDoc_ViewTool_HeaderFile
