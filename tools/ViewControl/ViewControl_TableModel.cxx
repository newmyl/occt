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

#include <inspector/ViewControl_TableModel.hxx>

// =======================================================================
// function : SetModelValues
// purpose :
// =======================================================================
void ViewControl_TableModel::SetModelValues (ViewControl_TableModelValues* theModelValues)
{
  if (myModelValues)
    delete myModelValues;

  myModelValues = theModelValues;
  SetFilter(0);
}

// =======================================================================
// function : columnCount
// purpose :
// =======================================================================
int ViewControl_TableModel::columnCount(const QModelIndex& theParent) const
{
  if (!myModelValues)
    return 0;

  int aColumnCount = myModelValues->ColumnCount (theParent);
  return isFilterActive() ? myFilter->ColumnCount (aColumnCount) : aColumnCount;
}

// =======================================================================
// function : rowCount
// purpose :
// =======================================================================
int ViewControl_TableModel::rowCount(const QModelIndex& theParent ) const
{
  if (!myModelValues)
    return 0;

  return isFilterActive() ? myFilter->RowCount (myModelValues->ColumnCount (theParent))
                          : myModelValues->RowCount (theParent);
}

// =======================================================================
// function : data
// purpose :
// =======================================================================
QVariant ViewControl_TableModel::data (const QModelIndex& theIndex, int theRole) const
{
  if (!myModelValues)
    return QVariant();

  int aRow = theIndex.row(), aColumn = theIndex.column();
  if (isFilterActive())
    myFilter->GetSourcePosition (theIndex, aRow, aColumn);

  return myModelValues->Data (aRow, aColumn, theRole);
}

// =======================================================================
// function : setData
// purpose :
// =======================================================================
bool ViewControl_TableModel::setData (const QModelIndex& theIndex, const QVariant& theValue, int theRole)
{
  if (!myModelValues)
    return false;

  int aRow = theIndex.row(), aColumn = theIndex.column();
  if (isFilterActive())
    myFilter->GetSourcePosition (theIndex, aRow, aColumn);

  return myModelValues->SetData (aRow, aColumn, theValue, theRole);
}

// =======================================================================
// function : GetSourcePosition
// purpose :
// =======================================================================
void ViewControl_TableModel::GetSourcePosition (const QModelIndex& theIndex, int& theRow, int& theColumn)
{
  if (isFilterActive())
    myFilter->GetSourcePosition (theIndex, theRow, theColumn);
  else
    myModelValues->GetSourcePosition (theIndex.row(), theIndex.column(), theRow, theColumn);
}
