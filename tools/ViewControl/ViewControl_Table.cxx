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

#include <inspector/ViewControl_Table.hxx>
#include <inspector/ViewControl_TableModel.hxx>
#include <inspector/ViewControl_TableProperty.hxx>

#include <inspector/TreeModel_Tools.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QAction>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>
#include <QWidget>
#include <Standard_WarningsRestore.hxx>

#include <limits>

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================

ViewControl_Table::ViewControl_Table (QWidget* theParent)
: QObject (theParent), myIsUseProperty (false)
{
  myMainWidget = new QWidget (theParent);
  QGridLayout* aLayout = new QGridLayout (myMainWidget);
  aLayout->setContentsMargins (0, 0, 0, 0);

  myProperty = new ViewControl_TableProperty(myMainWidget, this);
  aLayout->addWidget (myProperty->GetControl());

  myTableView = new QTableView (myMainWidget);
  myTableView->setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);

  QHeaderView* aVHeader = myTableView->verticalHeader();
  int aDefCellSize = aVHeader->minimumSectionSize() + TreeModel_Tools::HeaderSectionMargin();
  aVHeader->setDefaultSectionSize (aDefCellSize);

  aLayout->addWidget (myTableView, 1, 0, 1, 4);
  aLayout->addWidget (myProperty->GetInformationControl());
}

// =======================================================================
// function : SetModel
// purpose :
// =======================================================================

void ViewControl_Table::SetModel (QAbstractTableModel* theModel)
{
  myTableView->setModel (theModel);

  myTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  QItemSelectionModel* aSelectionModel = new QItemSelectionModel(theModel);
  myTableView->setSelectionModel (aSelectionModel);
  connect(aSelectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
          this, SLOT(onTableSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================

void ViewControl_Table::Init (ViewControl_TableModelValues* theModelValues)
{
  myTableView->selectionModel()->clearSelection();

  ViewControl_TableModel* aModel = dynamic_cast<ViewControl_TableModel*> (myTableView->model());
  aModel->SetModelValues (theModelValues);

  myProperty->SetActive (theModelValues->UseTableProperties());
  myProperty->Init();

  int aSectionSize;
  myTableView->horizontalHeader()->setVisible (theModelValues->IsHeaderVisible (Qt::Horizontal));
  if (theModelValues->GetDefaultSectionSize (Qt::Horizontal, aSectionSize) )
    myTableView->horizontalHeader()->setDefaultSectionSize (aSectionSize);

  myTableView->verticalHeader()->setVisible (theModelValues->IsHeaderVisible (Qt::Vertical));
  if (theModelValues->GetDefaultSectionSize (Qt::Vertical, aSectionSize) )
  {
    myTableView->verticalHeader()->setDefaultSectionSize (aSectionSize);
  }
  else
    myTableView->verticalHeader()->setDefaultSectionSize (myTableView->verticalHeader()->minimumSectionSize() +
                                                          TreeModel_Tools::HeaderSectionMargin());
  aModel->EmitLayoutChanged();
}

// =======================================================================
// function : GetSelectedIndices
// purpose :
// =======================================================================

void ViewControl_Table::GetSelectedIndices (QMap<int, QList<int>>& theSelectedIndices)
{
  QModelIndexList aSelected = myTableView->selectionModel()->selectedIndexes();
  ViewControl_TableModel* aModel = dynamic_cast<ViewControl_TableModel*> (myTableView->model());

  int aRow, aColumn;
  for (QModelIndexList::const_iterator anIt = aSelected.begin(); anIt != aSelected.end(); anIt++)
  {
    aModel->GetSourcePosition (*anIt, aRow, aColumn);

    if (!theSelectedIndices.contains (aRow))
      theSelectedIndices.insert (aRow, QList<int>());
    theSelectedIndices[aRow].append (aColumn);
  }
}

// =======================================================================
// function : onTableSelectionChanged
// purpose :
// =======================================================================

void ViewControl_Table::onTableSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  QModelIndexList aSelected = myTableView->selectionModel()->selectedIndexes();

  myProperty->UpdateOnTableSelectionChanged();
}
