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

#ifndef ViewControl_TableModel_H
#define ViewControl_TableModel_H

#include <Standard.hxx>

#include <inspector/ViewControl_TableModelFilter.hxx>
#include <inspector/ViewControl_TableModelValues.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QAbstractTableModel>
#include <QList>
#include <QVariant>
#include <Standard_WarningsRestore.hxx>

#include <vector>

//! \class ViewControl_TableModel
//! \brief This is an extension of table model to visualize a container of values
//! It is possible to:
//! - set orientation to interpretate the values.
//! - set table view header values.
//! Items of the view are enabled and selectable.
class ViewControl_TableModel : public QAbstractTableModel
{
public:

  //! Constructor
  ViewControl_TableModel (QObject* theParent = 0) : myModelValues (0), myFilter (0) { (void)theParent; }

  //! Destructor
  virtual ~ViewControl_TableModel() {}

  //! Sets interface to table values
  //! \theModelValues instance of values
  void SetModelValues (ViewControl_TableModelValues* theModelValues) { myModelValues = theModelValues; SetFilter(0); }

  //! Returns instance of interface for access totable values
  //! \return interface or NULL
  ViewControl_TableModelValues* GetModelValues() const { return myModelValues; }

  //! Sets table values filter to rearrange values presentation
  //! \param filter instance
  void SetFilter (ViewControl_TableModelFilter* theFilter) { myFilter = theFilter; }

  //! Emits the layoutChanged signal from outside of this class
  void EmitLayoutChanged() { emit layoutChanged(); }

  //! Returns number of columns, depending on orientation: myColumnCount or size of values container
  //! \param theParent an index of the parent item
  //! \return an integer value
  virtual int columnCount(const QModelIndex& theParent = QModelIndex()) const Standard_OVERRIDE
  { return myModelValues ? (myFilter ? myFilter->ColumnCount (myModelValues->ColumnCount (theParent)) : myModelValues->ColumnCount (theParent)) : 0; }

  //! Returns number of rows, depending on orientation: myColumnCount or size of values container
  //! \param theParent an index of the parent item
  //! \return an integer value
  virtual int rowCount(const QModelIndex& theParent = QModelIndex()) const Standard_OVERRIDE
  { return myModelValues ? (myFilter ? myFilter->RowCount (myModelValues->ColumnCount (theParent)) : myModelValues->RowCount (theParent)) : 0; }

  //! Returns content of the model index for the given role, it is obtained from internal container of values
  //! It returns value only for DisplayRole.
  //! \param theIndex a model index
  //! \param theRole a view role
  //! \return value intepreted depending on the given role
  virtual QVariant data (const QModelIndex& theIndex, int theRole = Qt::DisplayRole) const Standard_OVERRIDE
  {
    if (!myModelValues)
      return QVariant();
    int aRow = theIndex.row(), aColumn = theIndex.column();
    if (myFilter)
      myFilter->GetSourcePosition (theIndex, aRow, aColumn);
    return myModelValues->Data (aRow, aColumn, theRole);
  }
  //! Returns content of the model index for the given role, it is obtainer from internal container of header values
  //! It returns value only for DisplayRole.
  //! \param theSection an index of value in the container 
  //! \param theIndex a model index
  //! \param theRole a view role
  //! \return value intepreted depending on the given role
  virtual QVariant headerData (int theSection, Qt::Orientation theOrientation, int theRole = Qt::DisplayRole) const Standard_OVERRIDE
  { return myModelValues ? myModelValues->HeaderData (theSection, theOrientation, theRole) : QVariant(); }

  //! Returns flags for the item: ItemIsEnabled | Qt::ItemIsSelectable
  //! \param theIndex a model index
  //! \return flags
  Qt::ItemFlags flags (const QModelIndex& theIndex) const
  { return myModelValues ? myModelValues->Flags (theIndex) : Qt::NoItemFlags; }

  //! Returns source row and column values peforming conversion back from filter
  //! \param theIndex a model index
  //! \param theRow a model row
  //! \param theColumn a model column
  void GetSourcePosition (const QModelIndex& theIndex, int& theRow, int& theColumn)
  { if (myFilter) myFilter->GetSourcePosition (theIndex, theRow, theColumn); else { theRow = theIndex.row(); theColumn = theIndex.column();} }

private:
  ViewControl_TableModelValues* myModelValues; //! interface to table values
  ViewControl_TableModelFilter* myFilter; //! filter of values
};

#endif
