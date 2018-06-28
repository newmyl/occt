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

#ifndef TreeModel_PropertyView_H
#define TreeModel_PropertyView_H

#include <Standard.hxx>
#include <Standard_Macro.hxx>

#include <inspector/ViewControl_Table.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QObject>
#include <QList>
#include <Standard_WarningsRestore.hxx>

class QAbstractTableModel;
class QStackedWidget;
class QWidget;
class QVBoxLayout;

class ViewControl_TableModelValues;

//! \class ViewControl_PropertyView
//! \brief View widget where several tables are visualized in vertical layout.
class ViewControl_PropertyView : public QObject
{
  Q_OBJECT
public:

  //! Constructor
  Standard_EXPORT ViewControl_PropertyView (QWidget* theParent);

  //! Destructor
  virtual ~ViewControl_PropertyView() Standard_OVERRIDE {}

  //! Fills the view content with values. Number of visible tables is size of container,
  //! Each element of container is values of the corresponded table
  //! \param theTableValues values
  Standard_EXPORT void Init (const QList<ViewControl_TableModelValues*>& theTableValues);

  //! Fills the view content with the parameter custom widget.
  //! \param theWidget control
  Standard_EXPORT void Init (QWidget* theWidget);

    //! Clears layout of the view and tables models.
  Standard_EXPORT void Clear();

  //! \return the text edit control
  QWidget* GetControl() const { return myMainWidget; }

  //! Returns container of active tables
  //! \param theTables [out] modified container
  Standard_EXPORT void GetActiveTables (QList<ViewControl_Table*>& theTables);

  //! Clears selection in active tables
  Standard_EXPORT void ClearActiveTablesSelection();

  //! Sets X steps values for table properties
  //! \param container of values
  void SetXSteps (const QList<double>& theValues) { myXStepValues = theValues; }

  //! Sets divide values for table properties
  //! \param container of values
  void SetDivideValues (const QList<int>& theValues) { myDivideValues = theValues; }

  //! Save state of three view in a container in form: key, value. It saves:
  //! - XStep of property table
  //! - divide values of property table
  //! \param theTreeView a view instance
  //! \param theItems [out] properties
  //! \param thePrefix peference item prefix
  Standard_EXPORT static void SaveState (ViewControl_PropertyView* theView, QMap<QString, QString>& theItems,
                                         const QString& thePrefix = QString());
  //! Restore state of three view by a container
  //! \param theTreeView a view instance
  //! \param theKey property key
  //! \param theValue property value
  //! \param thePrefix peference item prefix
  //! \return boolean value whether the property is applyed to the tree view
  Standard_EXPORT static bool RestoreState (ViewControl_PropertyView* theView, const QString& theKey, const QString& theValue,
                                            const QString& thePrefix = QString());

signals:
  void propertyViewSelectionChanged();

protected slots:
  //! Emits signal about selection is changed
  //! \param theSelected container of selected table cells
  //! \param theDeselected container of selected table cells
  void onTableSelectionChanged (const QItemSelection& theSelected, const QItemSelection& theDeselected);

protected:
  //! Returns table instance or create if it was not created ealier
  //! \param theTableId an index in internal container of tables
  //! \param isToCreate if true, the table is created if not exists
  ViewControl_Table* findTable (const int theTableId, const bool isToCreate = true);

private:
  bool myOwnSelectionChangeBlocked; //! blocking emit of selection changed signal

  QWidget* myMainWidget; //! parent of all controls

  QStackedWidget* myAttributesStack; //!< container of already created panes
  QWidget* myEmptyWidget; //!< an empty widget when nothing is selected in tree view

  QWidget* myTableWidget; //!< widget of tables in vertical layout
  QVBoxLayout* myTableWidgetLayout; //! main view layout where tables or custom widgets are presented
  QList<ViewControl_Table*> myTables; //!< table view, shown only first tables filled in Init method
  QWidget* myCustomWidget; //!< custom view widget

  QList<double> myXStepValues; //! predefined values for XStep in ViewControl_TableProperty
  QList<int> myDivideValues; //! predefined values for divide value in ViewControl_TableProperty
};
#endif
