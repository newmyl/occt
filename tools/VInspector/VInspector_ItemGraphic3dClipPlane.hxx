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

#ifndef VInspector_ItemGraphic3dClipPlane_H
#define VInspector_ItemGraphic3dClipPlane_H

#include <Standard.hxx>
#include <inspector/VInspector_ItemBase.hxx>

#include <Graphic3d_ClipPlane.hxx>

class VInspector_ItemGraphic3dClipPlane;
typedef QExplicitlySharedDataPointer<VInspector_ItemGraphic3dClipPlane> VInspector_ItemGraphic3dClipPlanePtr;

//! \class VInspector_ItemGraphic3dClipPlane
//! Parent item, that corresponds to AIS_InteractiveContext
//! Children of the item are:
//! - "Property" item to show context attributes such as selection filters and drawer properties
//! - presentation items to show all interactive elements displayed/erased in the context
class VInspector_ItemGraphic3dClipPlane : public VInspector_ItemBase
{
public:

  //! Creates an item wrapped by a shared pointer
  static VInspector_ItemGraphic3dClipPlanePtr CreateItem (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  { return VInspector_ItemGraphic3dClipPlanePtr (new VInspector_ItemGraphic3dClipPlane (theParent, theRow, theColumn)); }

  //! Destructor
  virtual ~VInspector_ItemGraphic3dClipPlane() Standard_OVERRIDE {};

  //! Inits the item, fills internal containers
  Standard_EXPORT virtual void Init() Standard_OVERRIDE;

  //! Resets cached values
  Standard_EXPORT virtual void Reset() Standard_OVERRIDE;

  //! Returns current clip plane, initialize if it was not initialized yet
  Standard_EXPORT Handle(Graphic3d_ClipPlane) GetClipPlane() const;

protected:
  //! Initialize the current item. It is empty because Reset() is also empty.
  virtual void initItem() const Standard_OVERRIDE;

  //! Returns number of displayed presentations
  //! \return rows count
  Standard_EXPORT virtual int initRowCount() const Standard_OVERRIDE;

  //! Returns item information for the given role. Fills internal container if it was not filled yet
  //! \param theItemRole a value role
  //! \return the value
  Standard_EXPORT virtual QVariant initValue (const int theItemRole) const Standard_OVERRIDE;

  //! Returns number of table rows
  //! \return an integer value
  virtual int GetTableRowCount() const Standard_OVERRIDE;

  //! Returns type of edit control for the model index. By default, it is an empty control
  //! \param theRow a model index row
  //! \param theColumn a model index column
  //! \return edit type
  virtual ViewControl_EditType GetTableEditType (const int theRow, const int theColumn) const Standard_OVERRIDE;

  //! Returns container of string values for enumeration in the model row
  //! \param theRow table model row index
  //! \param theColumn a model index column
  //! \return string values for the enumeration presented in the row or an empty container
  virtual QList<QVariant> GetTableEnumValues (const int theRow, const int theColumn) const Standard_OVERRIDE;

  //! Returns table value for the row in form: <function name> <function value>
  //! \param theRow a model index row
  //! \param theColumn a model index column
  virtual QVariant GetTableData (const int theRow, const int theColumn, const int theRole) const Standard_OVERRIDE;

  //! Sets the value into the table cell. Only 1st column value might be modified.
  //! \param theRow a model index row
  //! \param theColumn a model index column
  //! \param theValue a new cell value
  virtual bool SetTableData (const int theRow, const int theColumn, const QVariant& theValue) Standard_OVERRIDE;

protected:

  //! Creates a child item in the given position.
  //! \param theRow the child row position
  //! \param theColumn the child column position
  //! \return the created item
  virtual TreeModel_ItemBasePtr createChild (int theRow, int theColumn) Standard_OVERRIDE;

private:

  //! Set Clip Plane into the current field
  //! \param theViewer a viewer
  void setClipPlane (const Handle(Graphic3d_ClipPlane)& theClipPlane) { myClipPlane = theClipPlane; }

private:

  //! Constructor
  //! param theParent a parent item
  //! \param theRow the item row positition in the parent item
  //! \param theColumn the item column positition in the parent item
  VInspector_ItemGraphic3dClipPlane(TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
    : VInspector_ItemBase(theParent, theRow, theColumn) {}

protected:

  Handle(Graphic3d_ClipPlane) myClipPlane; //!< the current ClipPlane
};

#endif