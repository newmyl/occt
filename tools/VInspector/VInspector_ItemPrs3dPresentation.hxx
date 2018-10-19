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

#ifndef VInspector_ItemPrs3dPresentation_H
#define VInspector_ItemPrs3dPresentation_H

#include <Standard.hxx>
#include <inspector/VInspector_ItemBase.hxx>

#include <AIS_InteractiveObject.hxx>
#include <NCollection_List.hxx>

class QItemSelectionModel;

class VInspector_ItemPrs3dPresentation;
typedef QExplicitlySharedDataPointer<VInspector_ItemPrs3dPresentation> VInspector_ItemPrs3dPresentationPtr;

//! \class VInspector_ItemPrs3dPresentation
//! Item presents information about AIS_InteractiveObject.
//! Parent is item context, children are item selections.
class VInspector_ItemPrs3dPresentation : public VInspector_ItemBase
{

public:

  //! Creates an item wrapped by a shared pointer
  static VInspector_ItemPrs3dPresentationPtr CreateItem (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  { return VInspector_ItemPrs3dPresentationPtr (new VInspector_ItemPrs3dPresentation (theParent, theRow, theColumn)); }
  //! Destructor
  virtual ~VInspector_ItemPrs3dPresentation() Standard_OVERRIDE {};

  //! Returns the current presentation, init item if it was not initialized yet
  //! \return presentation object
  Standard_EXPORT Handle(Prs3d_Presentation) GetPresentation() const;

  //! Inits the item, fills internal containers
  Standard_EXPORT virtual void Init() Standard_OVERRIDE;

  //! Resets cached values
  Standard_EXPORT virtual void Reset() Standard_OVERRIDE;

  //! Returns number of table rows
  //! \return an integer value
  virtual int GetTableRowCount() const Standard_OVERRIDE;

  //! Returns table value for the row in form: <function name> <function value>
  //! \param theRow a model index row
  //! \param theColumn a model index column
  virtual QVariant GetTableData (const int theRow, const int theColumn, const int theRole) const Standard_OVERRIDE;

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

  //! Set presentation into the current field
  //! \param thePresentation a presentation
  //! \param theName the item name
  void setPresentation (const Handle(Prs3d_Presentation)& thePresentation,
                        const TCollection_AsciiString& theName);

private:

  //! Constructor
  //! param theParent a parent item
  VInspector_ItemPrs3dPresentation(TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  : VInspector_ItemBase(theParent, theRow, theColumn) {}

protected:

  Handle(Prs3d_Presentation) myPresentation; //!< the current presentation
  TCollection_AsciiString myName; //!< the item display data value
};

#endif
