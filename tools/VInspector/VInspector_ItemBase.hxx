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

#ifndef VInspector_ItemBase_H
#define VInspector_ItemBase_H

#include <AIS_InteractiveContext.hxx>
#include <Standard.hxx>
#include <TopoDS_Shape.hxx>

#include <inspector/TreeModel_ItemBase.hxx>
#include <inspector/ViewControl_EditType.hxx>

class VInspector_ItemBase;
typedef QExplicitlySharedDataPointer<VInspector_ItemBase> VInspector_ItemBasePtr;

//! \class VInspector_ItemBase
//! Parent item for all ShapeView items
class VInspector_ItemBase : public TreeModel_ItemBase
{
public:
  //! Resets cached values
  virtual void Reset() Standard_OVERRIDE;

  //! Sets the context 
  //! \param theLabel an object where the child items structure is found
  void SetContext (const Handle(AIS_InteractiveContext)& theContext) { myContext = theContext; }

  //! Returns true if the current context is not null
  //! \return a boolean value
  bool HasContext() const { return !GetContext().IsNull(); }

  //! Returns the current contex. It iterates up by list of parents to found context item and return context
  //! \return a context
  Standard_EXPORT Handle(AIS_InteractiveContext) GetContext() const;

  //! Returns presentation of the attribute to be visualized in the view
  //! \thePresentations [out] container of presentation handles to be visualized
  virtual void GetPresentations (NCollection_List<Handle(Standard_Transient)>& thePresentations)
  { (void)thePresentations; }

  //! Returns shape of the item parameters
  //! \return generated shape of the item parameters
  virtual TopoDS_Shape GetPresentationShape() const { initItem(); return myPresentationShape; }

  //! Returns number of table rows
  //! \return an integer value
  virtual int GetTableRowCount() const { return 0; }

  //! Returns type of edit control for the model index. By default, it is an empty control
  //! \param theRow a model index row
  //! \param theColumn a model index column
  //! \return edit type
  virtual ViewControl_EditType GetTableEditType (const int theRow, const int theColumn) const
    { (void)theRow; (void)theColumn; return ViewControl_EditType_None; }

  //! Returns container of string values for enumeration in the model row
  //! \param theRow table model row index
  //! \param theColumn a model index column
  //! \return string values for the enumeration presented in the row or an empty container
  virtual QList<QVariant> GetTableEnumValues (const int theRow, const int theColumn) const
    { (void)theRow; (void)theColumn; return QList<QVariant>(); }

  //! Returns table value for the row in form: <function name> <function value>
  //! \param theRow a model index row
  //! \param theColumn a model index column
  virtual QVariant GetTableData (const int theRow, const int theColumn, const int theRole) const
  {  (void)theRow; (void)theColumn; (void)theRole; return QVariant(); }

  //! Sets the value into the table cell. Only 1st column value might be modified.
  //! \param theRow a model index row
  //! \param theColumn a model index column
  //! \param theValue a new cell value
  virtual bool SetTableData (const int theRow, const int theColumn, const QVariant& theValue)
    { (void)theRow; (void)theColumn; (void)theValue; return false; }

protected:

  //! Initialize the current item. It creates a backup of the specific item information
  virtual void initItem() const {}

protected:

  //! Constructor
  //! param theParent a parent item
  //! \param theRow the item row positition in the parent item
  //! \param theColumn the item column positition in the parent item
  VInspector_ItemBase (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
    : TreeModel_ItemBase (theParent, theRow, theColumn), myContext (0) {}

protected:

  Handle(AIS_InteractiveContext) myContext; //!< the current context
  TopoDS_Shape myPresentationShape; //!< item presentation shape
};

#endif