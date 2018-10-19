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

#include <inspector/ViewControl_TableProperty.hxx>
#include <inspector/ViewControl_Table.hxx>
#include <inspector/ViewControl_TableModel.hxx>
#include <inspector/ViewControl_TableModelFilter.hxx>
#include <inspector/ViewControl_TableModelValues.hxx>

#include <inspector/TreeModel_Tools.hxx>

#include <QAbstractItemModel>
#include <QAction>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>
#include <QWidget>

#include <limits>

const int DEFAULT_COLUMN_WIDTH = 60;

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================

ViewControl_TableProperty::ViewControl_TableProperty (QWidget* theParent, ViewControl_Table* theTable)
: QObject (theParent), myTable (theTable), myIsActive (false)
{
  myFilter = new ViewControl_TableModelFilter();

  myMainWidget = new QWidget (theParent);
  QGridLayout* aMainLayout = new QGridLayout (myMainWidget);

  QWidget* aProperties = new QWidget (myMainWidget);
  aMainLayout->addWidget (aProperties);
  QHBoxLayout* aPropertiesLayout = new QHBoxLayout (aProperties);
  aPropertiesLayout->setContentsMargins (0, 0, 0, 0);

  mySize = new QLabel ("", myMainWidget);
  myDivideSize = new QLabel("Divide:", myMainWidget);
  mySeparateSize = new QSpinBox (myMainWidget);
  mySeparateSize->setRange (0, 100000);
  myVisualizedSize = new QLabel ("", myMainWidget);

  aPropertiesLayout->addWidget (mySize);
  aPropertiesLayout->addWidget (myDivideSize);
  aPropertiesLayout->addWidget (mySeparateSize);
  aPropertiesLayout->addWidget (myVisualizedSize);

  myXStepLabel = new QLabel ("XStep:");
  myXStep = new QDoubleSpinBox (myMainWidget);
  myXStep->setRange(1.e-12, 1.e+7);
  myXStep->setDecimals (8);
  myXStep->setValue(1.);

  setXStepActive (false);

  connect (myXStep, SIGNAL(editingFinished()), this, SIGNAL (propertyChanged()));

  aPropertiesLayout->addWidget (myXStepLabel);
  aPropertiesLayout->addWidget (myXStep);

  myInformationWidget = new QWidget (theParent);
  QGridLayout* anInfoLayout = new QGridLayout (myInformationWidget);

  myModelInformation = new QPlainTextEdit (myInformationWidget);
  myModelInformation->setVisible (false);
  anInfoLayout->addWidget (myModelInformation, 2, 0, 1, 4);

  myMinValue = new QPushButton ("Min", myInformationWidget);
  myMaxValue = new QPushButton ("Max", myInformationWidget);
  myMinSelectedValue = new QPushButton ("Min of selected", myInformationWidget);
  myMaxSelectedValue = new QPushButton ("Max of selected", myInformationWidget);

  anInfoLayout->addWidget (myMinValue, 3, 0);
  anInfoLayout->addWidget (myMinSelectedValue, 3, 1);
  anInfoLayout->addWidget (myMaxValue, 4, 0);
  anInfoLayout->addWidget (myMaxSelectedValue, 4, 1);

  connect (mySeparateSize, SIGNAL (valueChanged (int)),
           this, SLOT (onSeparateSizeChanged (int)));
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================

void ViewControl_TableProperty::Init()
{
  ViewControl_TableModel* aViewModel = dynamic_cast<ViewControl_TableModel*>(myTable->GetTableView()->model());
  ViewControl_TableModelValues* aModelValues = aViewModel->GetModelValues();

  bool aUseXStep;
  double aXStep = aModelValues->UseTablePropertiesXStep (aUseXStep);
  setXStepActive (aUseXStep, aXStep);

  QString aMinValue = QString::number (DBL_MIN), aMaxValue = QString::number (DBL_MAX);
  aModelValues->GetRangeValues (aMinValue, aMaxValue, QModelIndexList());
  QString anInfo = aModelValues->AdditionalInformation();

  bool aUseSeparateSize = aModelValues->UseTableSeparateSize();
  mySize->setVisible (aUseSeparateSize);
  myDivideSize->setVisible (aUseSeparateSize);
  mySeparateSize->setVisible (aUseSeparateSize);
  myVisualizedSize->setVisible (aUseSeparateSize);
  myFilter->SetActive (aUseSeparateSize);
  if (aUseSeparateSize && mySeparateSize->value() != 0)
    myFilter->SetColumnCount(mySeparateSize->value());
  
  mySize->setText (QString("[ %1, %2 ]").arg (aViewModel->rowCount()).arg (aViewModel->columnCount()));

  myVisualizedSize->setText (mySize->text());
  myVisualizedSize->setToolTip (QString ("sqrt: (%1, %2)").arg (sqrt (aViewModel->rowCount())).arg (sqrt (aViewModel->columnCount())));
  myModelInformation->setVisible(!anInfo.isEmpty());
  if (!anInfo.isEmpty())
    myModelInformation->setPlainText (anInfo);

  myMinValue->setText (QString ("Min: ") + aMinValue);
  myMaxValue->setText (QString ("Max: ") + aMaxValue);
  myMinSelectedValue->setText (QString("Min of selected: ") + QString::number(0));
  myMaxSelectedValue->setText (QString("Max of selected: ") + QString::number(0));
}

// =======================================================================
// function : SetActive
// purpose :
// =======================================================================

void ViewControl_TableProperty::SetActive (const bool theActive)
{
  GetControl()->setVisible (theActive);
  GetInformationControl()->setVisible (theActive);

  ViewControl_TableModel* aModel = dynamic_cast<ViewControl_TableModel*> (myTable->GetTableView()->model());
  aModel->SetFilter (theActive ? GetFilter() : 0);
}

// =======================================================================
// function : onSeparateSizeChanged
// purpose :
// =======================================================================

void ViewControl_TableProperty::onSeparateSizeChanged (int theValue)
{
  myFilter->SetColumnCount(theValue);

  myTable->GetTableView()->selectionModel()->clearSelection();

  ViewControl_TableModel* aViewModel = dynamic_cast<ViewControl_TableModel*>(myTable->GetTableView()->model());
  aViewModel->EmitLayoutChanged();

  myVisualizedSize->setText (QString ("[ %1, %2 ]").arg (aViewModel->rowCount()).arg (aViewModel->columnCount()));
}

// =======================================================================
// function : onToleranceSizeChanged
// purpose :
// =======================================================================

void ViewControl_TableProperty::onToleranceSizeChanged (int theValue)
{
  (void)theValue;
}

// =======================================================================
// function : onSeparateSizeChanged
// purpose :
// =======================================================================

void ViewControl_TableProperty::UpdateOnTableSelectionChanged()
{
  QModelIndexList aSelected = myTable->GetTableView()->selectionModel()->selectedIndexes();
  if (aSelected.isEmpty())
  {
    myMinSelectedValue->setText(QString("Min of selected:") + QString::number(0));
    myMaxSelectedValue->setText(QString("Max of selected:") + QString::number(0));
    return;
  }

  ViewControl_TableModel* aViewModel = dynamic_cast<ViewControl_TableModel*>(myTable->GetTableView()->model());
  ViewControl_TableModelValues* aModelValues = aViewModel->GetModelValues();

  QString aMinValue = QString::number (DBL_MIN), aMaxValue = QString::number (DBL_MAX);
  aModelValues->GetRangeValues (aMinValue, aMaxValue, aSelected);

  myMinSelectedValue->setText (QString ("Min of selected:") + aMinValue);
  myMaxSelectedValue->setText (QString ("Max of selected:") + aMaxValue);
}

// =======================================================================
// function : createAction
// purpose :
// =======================================================================
void ViewControl_TableProperty::setXStepActive (const bool theState, const double theValue)
{
  myXStepActive = theState;

  myXStepLabel->setVisible (myXStepActive);
  myXStep->setVisible (myXStepActive);

  if (theValue > 0)
    myXStep->setValue (theValue);
}
