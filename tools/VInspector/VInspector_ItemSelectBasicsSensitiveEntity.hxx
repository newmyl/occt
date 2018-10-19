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

#ifndef VInspector_ItemSelectBasicsSensitiveEntity_H
#define VInspector_ItemSelectBasicsSensitiveEntity_H

#include <AIS_InteractiveObject.hxx>
#include <SelectBasics_SensitiveEntity.hxx>
#include <SelectBasics_SensitiveEntity.hxx>
#include <Standard.hxx>
#include <inspector/VInspector_ItemBase.hxx>

class QItemSelectionModel;
class SelectBasics_EntityOwner;
class VInspector_ItemSelectBasicsSensitiveEntity;

typedef QExplicitlySharedDataPointer<VInspector_ItemSelectBasicsSensitiveEntity> VInspector_ItemSelectBasicsSensitiveEntityPtr;

//! \class VInspector_ItemSelectBasicsSensitiveEntity
//! The item shows information about SelectBasics_EntityOwner.
//! The parent is item selection, children are item entity owners
class VInspector_ItemSelectBasicsSensitiveEntity : public VInspector_ItemBase
{

public:

  //! Creates an item wrapped by a shared pointer
  static VInspector_ItemSelectBasicsSensitiveEntityPtr CreateItem (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  { return VInspector_ItemSelectBasicsSensitiveEntityPtr (new VInspector_ItemSelectBasicsSensitiveEntity (theParent, theRow, theColumn)); }

  //! Destructor
  virtual ~VInspector_ItemSelectBasicsSensitiveEntity() Standard_OVERRIDE {};

  //! \return the current sensitive entity
  Standard_EXPORT Handle(SelectBasics_SensitiveEntity) GetSensitiveEntity() const;

  //! Inits the item, fills internal containers
  Standard_EXPORT virtual void Init() Standard_OVERRIDE;

  //! Resets cached values
  Standard_EXPORT virtual void Reset() Standard_OVERRIDE;

  //! Returns presentation of the attribute to be visualized in the view
  //! \thePresentations [out] container of presentation handles to be visualized
  Standard_EXPORT virtual void GetPresentations (NCollection_List<Handle(Standard_Transient)>& thePresentations);

  //! Returns number of table rows
  //! \return an integer value
  virtual int GetTableRowCount() const Standard_OVERRIDE;

  //! Returns type of edit control for the model index. By default, it is an empty control
  //! \param theRow a model index row
  //! \param theColumn a model index column
  //! \return edit type
  virtual ViewControl_EditType GetTableEditType (const int theRow, const int theColumn) const Standard_OVERRIDE;

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

  //! Initialize the current item. It is empty because Reset() is also empty.
  virtual void initItem() const Standard_OVERRIDE;

  //! \return number of children.
  virtual int initRowCount() const Standard_OVERRIDE;

  //! Returns item information for the given role. Fills internal container if it was not filled yet
  //! \param theItemRole a value role
  //! \return the value
  virtual QVariant initValue (const int theItemRole) const Standard_OVERRIDE;

protected:

  //! Creates a child item in the given position.
  //! \param theRow the child row position
  //! \param theColumn the child column position
  //! \return the created item
  virtual TreeModel_ItemBasePtr createChild (int theRow, int theColumn) Standard_OVERRIDE;

  //! Returns owner of the current sensitive entity
  //! \return owner
  Handle(SelectBasics_EntityOwner) getEntityOwner() const;

  //! Returns table value for the row in form: <function name> <function value> depending on the aspect kind
  //! \param theRow a model index row
  //! \param theColumn a model index column
  //! \param theEntityKind kind or kind of entity
  QVariant getTableData (const int theRow,
                         const int theColumn,
                         const int theRole,
                         const TCollection_AsciiString& theEntityKind) const;

  //! Creates shape depending on the entity kind and parameters
  //! \param theEntity current sensitive entity
  //! \return shape or NULL
  static TopoDS_Shape buildPresentationShape (const Handle(SelectBasics_SensitiveEntity)& theEntity);

private:

  //! Constructor
  //! param theParent a parent item
  VInspector_ItemSelectBasicsSensitiveEntity(TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  : VInspector_ItemBase(theParent, theRow, theColumn) {}

private:

  Handle(SelectBasics_SensitiveEntity) myEntity; //!< the current sensitive entity
  Handle(AIS_InteractiveObject) myPresentation; //!< the current presentation
};

#endif
