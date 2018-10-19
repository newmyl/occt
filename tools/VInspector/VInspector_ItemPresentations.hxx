// Created on: 2017-06-16
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2017 OPEN CASCADE SAS
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

#ifndef VInspector_ItemPresentations_H
#define VInspector_ItemPresentations_H

#include <Standard.hxx>
#include <inspector/VInspector_ItemBase.hxx>

#include <TCollection_AsciiString.hxx>
#include <Prs3d_Presentation.hxx>

class VInspector_ItemPresentations;
typedef QExplicitlySharedDataPointer<VInspector_ItemPresentations> VInspector_ItemPresentationsPtr;

//! \class VInspector_ItemPresentations
//! Item presents information about AIS_InteractiveObject.
//! Parent is item context, children are item selections.
class VInspector_ItemPresentations : public VInspector_ItemBase
{

public:

  //! Creates an item wrapped by a shared pointer
  static VInspector_ItemPresentationsPtr CreateItem (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  { return VInspector_ItemPresentationsPtr (new VInspector_ItemPresentations (theParent, theRow, theColumn)); }
  //! Destructor
  virtual ~VInspector_ItemPresentations() Standard_OVERRIDE {};

  //! Returns presentation of the parent interactive object
  //! \return presentation object
  Standard_EXPORT Handle(Prs3d_Presentation) GetPresentation (const int theRowId, TCollection_AsciiString& theName) const;

  //! Inits the item, fills internal containers
  Standard_EXPORT virtual void Init() Standard_OVERRIDE;

  //! Resets cached values
  Standard_EXPORT virtual void Reset() Standard_OVERRIDE;

protected:

  //! Initialize the current item. It is empty because Reset() is also empty.
  virtual void initItem() const Standard_OVERRIDE;

  //! Returns number of item selected
  //! \return rows count
  virtual int initRowCount() const Standard_OVERRIDE;

  //! Returns item information for the given role. Fills internal container if it was not filled yet
  //! \param theItemRole a value role
  //! \return the value
  virtual QVariant initValue (const int theItemRole) const Standard_OVERRIDE;

  //! Creates a child item in the given position.
  //! \param theRow the child row position
  //! \param theColumn the child column position
  //! \return the created item
  virtual TreeModel_ItemBasePtr createChild (int theRow, int theColumn) Standard_OVERRIDE;

private:

  //! Constructor
  //! param theParent a parent item
  VInspector_ItemPresentations(TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  : VInspector_ItemBase(theParent, theRow, theColumn) {}

};

#endif
