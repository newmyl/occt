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

#ifndef ViewControl_Table_H
#define ViewControl_Table_H

#include <Standard.hxx>
#include <Standard_Macro.hxx>

#include <inspector/ViewControl_TableProperty.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QItemSelection>
#include <QObject>
#include <QTableView>
#include <Standard_WarningsRestore.hxx>

class ViewControl_TableModelValues;

class QAbstractTableModel;
class QContextMenuEvent;
class QWidget;

//! \class ViewControl_Table
//! \brief View to display table values with possibility to change table columns
//! if the table has 1D dimension and Horizontal orientation
class ViewControl_Table : public QObject
{
  Q_OBJECT
public:

  //! Constructor
  Standard_EXPORT ViewControl_Table (QWidget* theParent);

  //! Destructor
  virtual ~ViewControl_Table() Standard_OVERRIDE {}

  //! Set model into table view, init selection model by the given model, connect to selection change
  //! \param theModel table values model
  void SetModel (QAbstractTableModel* theModel);

  //! Fills table view and table size control by the model
  //! \param theModel values model
  Standard_EXPORT void Init (ViewControl_TableModelValues* theModelValues);

  //! Sets the table active and show the table
  //! \param theState boolean value
  void SetActive (const bool theState) { myIsActive = theState; GetTableView()->setVisible (theState); }

  //!< true if the table is used in property view and visible
  bool IsActive() const { return myIsActive; }

  //! \return the text edit control
  QWidget* GetControl() const { return myMainWidget; }

  //! \return the table view
  QTableView* GetTableView() const { return myTableView; }

  //! Returns instance of table property control
  //! \return property
  ViewControl_TableProperty* GetProperty() const { return myProperty; }

  //! Retuns model indices of the selected cells in table view
  //! \param theSelectedIndices [out] a container of indices: row to list of columns
  Standard_EXPORT void GetSelectedIndices (QMap<int, QList<int>>& aSelectedIndices);

protected slots:

  //! Updates controls by selection change in table view
  //! \param theSelected container of selected table cells
  //! \param theDeselected container of selected table cells
  void onTableSelectionChanged(const QItemSelection& theSelected, const QItemSelection& theDeselected);

private:
  bool myIsActive; //!< true if the table is used in property view and visible

  QWidget* myMainWidget; //!< parent of all controls
  bool myIsUseProperty; //!< boolean value whether the property control should be shown/hidden
  QTableView* myTableView; //!< table view

  ViewControl_TableProperty* myProperty; //!< modifier of the table visual properties
};
#endif
