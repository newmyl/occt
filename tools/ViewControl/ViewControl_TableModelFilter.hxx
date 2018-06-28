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

#ifndef ViewControl_TableModelFilter_H
#define ViewControl_TableModelFilter_H

#include <Standard.hxx>

#include <inspector/ViewControl_TableModelValues.hxx>

//! \class ViewControl_TableModelFilter
//! \brief It provides separation of 1D container of values in 2D presentation depending on number of column value
class ViewControl_TableModelFilter
{
public:

  //! Constructor
  Standard_EXPORT ViewControl_TableModelFilter() : myModel (0), myColumnCount (-1) {}

  //! Destructor
  virtual ~ViewControl_TableModelFilter() {}

  //! Sets the model of values
  //! \param theModelValues table view model
  void SetModel (ViewControl_TableModelValues* theModel) { myModel = theModel; }

  //! Sets number of columns
  //! \param theColumnCount a column count
  void SetColumnCount (const int theColumnCount) { myColumnCount = theColumnCount; }

  //! Returns number of columns: parameter value
  //! \param theSourceColumnCount number of columns in the source table
  //! \return an integer value
  Standard_EXPORT int ColumnCount (const int theSourceColumnCount) const;

  //! Returns number of rows: whole number of columns is divided to the current column value
  //! \param theSourceColumnCount number of columns in the source table
  //! \return an integer value
  Standard_EXPORT int RowCount (const int theSourceColumnCount) const;

  //! Returns source row/column indices for the filtered model index for the given role
  //! \param theIndex a model index
  //! \param theRow [out] row number value
  //! \param theColumn [out] column value
  Standard_EXPORT void GetSourcePosition (const QModelIndex& theIndex, int& theRow, int& theColumn) const;

private:
  ViewControl_TableModelValues* myModel; //!< table model
  int myColumnCount; //!< number of table columns
};

#endif
