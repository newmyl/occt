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

#include <inspector/ViewControl_TableModelFilter.hxx>

// =======================================================================
// function : ColumnCount
// purpose :
// =======================================================================

int ViewControl_TableModelFilter::ColumnCount (const int theSourceColumnCount) const
{
  return myColumnCount <= 0 ? theSourceColumnCount : myColumnCount;
}

// =======================================================================
// function : RowCount
// purpose :
// =======================================================================

int ViewControl_TableModelFilter::RowCount (const int theSourceColumnCount) const
{
  if (myColumnCount <= 0)
    return 1;

  int aRows = (int) (theSourceColumnCount / myColumnCount);
  if (myColumnCount > 0 && aRows * myColumnCount < theSourceColumnCount)
    aRows++; /// row with left values, not fully filled

  return aRows;
}

// =======================================================================
// function : GetSourcePosition
// purpose :
// =======================================================================

void ViewControl_TableModelFilter::GetSourcePosition (const QModelIndex& theIndex, int& theRow, int& theColumn) const
{
  GetSourcePosition (theIndex.row(), theIndex.column(), theRow, theColumn);
}

// =======================================================================
// function : GetSourcePosition
// purpose :
// =======================================================================

void ViewControl_TableModelFilter::GetSourcePosition (const int theSourceRow, const int theSourceColumn, int& theRow,
                                                      int& theColumn) const
{
  theRow = 0;
  theColumn = myColumnCount * theSourceRow + theSourceColumn;
}
