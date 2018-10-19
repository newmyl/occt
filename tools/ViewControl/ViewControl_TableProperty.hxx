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

#ifndef ViewControl_TableProperty_H
#define ViewControl_TableProperty_H

#include <Standard.hxx>
#include <Standard_Macro.hxx>

#ifdef _MSC_VER
#pragma warning(disable : 4127) // conditional expression is constant
#endif

#include <QDoubleSpinBox>
#include <QObject>
#include <QSpinBox>

class ViewControl_Table;
class ViewControl_TableModelFilter;

class QContextMenuEvent;
class QLabel;
class QItemSelection;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QWidget;

//! \class ViewControl_TableProperty
//! \brief View to display table values with possibility to change table columns
//! if the table has 1D dimension and Horizontal orientation
class ViewControl_TableProperty : public QObject
{
  Q_OBJECT
public:

  //! Constructor
  Standard_EXPORT ViewControl_TableProperty (QWidget* theParent, ViewControl_Table* theTable);

  //! Destructor
  virtual ~ViewControl_TableProperty() Standard_OVERRIDE {}

  //! Fills table view and table size control by the model
  Standard_EXPORT void Init();

  //! Sets whether the properties pane is visible or not
  //! \param theUseProperty boolean state
  Standard_EXPORT void SetActive (const bool theActive);

  //! Returns main control
  //! \return widget
  QWidget* GetControl() const { return myMainWidget; }

  //! Returns information control
  //! \return widget
  QWidget* GetInformationControl() { return myInformationWidget; }

  //! Returns table values filter to apply separation size
  //! \return filter instance
  ViewControl_TableModelFilter* GetFilter() { return myFilter; }

  //! Returns X step or -1 if it is not used
  //! \return double value
  double GetXStep() const { return myXStepActive ? myXStep->value() : -1;};

  //! Sets X step
  //! \param theValue value
  void SetXStep(const double theValue) const { myXStep->setValue (theValue); };

  //! Returns divide valid
  //! \return control value
  int GetDivideSize() const { return mySeparateSize->value(); }

  //! Sets separate size value
  //! \param theValue new value
  void SetDivideSize (const int theValue) { mySeparateSize->setValue (theValue); }

signals:
  //! Signals about header cell is clicked
  //! \param theEvent context menu event
  void headerContextMenuRequsted (QContextMenuEvent* theEvent);

  //! Signals about the following properties are changed: XStep.
  void propertyChanged();

protected slots:
  //! Reacts to the spin box value change, it divides table model values to has given number of columns
  //! \param theValue a new value of spin box
  void onSeparateSizeChanged (int theValue);

  //! Reacts to the tolerance value change, it sets it into table model
  //! \param theValue a new value of spin box
  void onToleranceSizeChanged (int theValue);

public:
  //! Updates controls by selection change in table view
  //! \param theSelected container of selected table cells
  //! \param theDeselected container of selected table cells
  void UpdateOnTableSelectionChanged();

private:
  //! Changes visibility of XStep control, if visible, set parameter value
  //! \param theState if true, the control is visible
  //! \param theValue the current for the control
  void setXStepActive (const bool theState, const double theValue = -1);

private:
  bool myIsActive; //!< state whether the control is visible and used
  QWidget* myMainWidget; //!< parent of all controls
  QLabel* mySize; //!< control to visualize initial values size
  QLabel* myDivideSize; //!< size of division table values to rows
  QSpinBox* mySeparateSize; //!< control to divide table by given size if thable has only 1 dimension
  QLabel* myVisualizedSize; //!< control to visualize current table size

  bool myXStepActive; //!< state whether the XStep control is visible
  QLabel* myXStepLabel; //!< label of X step
  QDoubleSpinBox* myXStep; //!< control to enter X step

  QWidget* myInformationWidget; //!< parent of all controls
  QPlainTextEdit* myModelInformation; //!< control to visualize current table size
  QPushButton* myMinValue; //!< minimum table value
  QPushButton* myMaxValue; //!< maximum table value
  QPushButton* myMinSelectedValue; //!< minimum value of selected table cells
  QPushButton* myMaxSelectedValue; //!< maximum value of selected table cells

  ViewControl_Table* myTable; //!< table control
  ViewControl_TableModelFilter* myFilter; //!< table values filter to apply separation size
};
#endif
