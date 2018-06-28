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

#include <inspector/ViewControl_PropertyView.hxx>
#include <inspector/ViewControl_Table.hxx>
#include <inspector/ViewControl_TableModel.hxx>
#include <inspector/ViewControl_TableModelValues.hxx>
#include <inspector/ViewControl_Tools.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QAbstractTableModel>
#include <QStackedWidget>
#include <QScrollArea>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>
#include <Standard_WarningsRestore.hxx>

class ViewControl_Widget : public QWidget
{
public:
  //! Constructor
  ViewControl_Widget (QWidget* theParent, const QSize& theSize) : QWidget (theParent) { SetPredefinedSize (theSize); }

  //! Destructor
  virtual ~ViewControl_Widget() {}

  //! Sets default size of control, that is used by the first control show
  //! \param theDefaultWidth the width value
  //! \param theDefaultHeight the height value
  void SetPredefinedSize (const QSize& theSize) { myDefaultSize = theSize;}

  //! Returns predefined size if both values are positive, otherwise parent size hint
  virtual QSize sizeHint() const Standard_OVERRIDE { return myDefaultSize.isValid() ? myDefaultSize : QWidget::sizeHint(); }

private:
  QSize myDefaultSize; //! default size, empty isze if it should not be used
};

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================
ViewControl_PropertyView::ViewControl_PropertyView (QWidget* theParent)
: QObject (theParent), myOwnSelectionChangeBlocked (false)
{
  myMainWidget = new ViewControl_Widget (theParent, QSize (1, 100));

  QVBoxLayout* aLayout = new QVBoxLayout (myMainWidget);
  aLayout->setContentsMargins (0, 0, 0, 0);

  QScrollArea* anArea = new QScrollArea (myMainWidget);

  myAttributesStack = new QStackedWidget (myMainWidget);
  anArea->setWidget (myAttributesStack);
  anArea->setWidgetResizable( true );
  aLayout->addWidget (anArea);

  myEmptyWidget = new QWidget (myAttributesStack);
  myAttributesStack->addWidget (myEmptyWidget);

  myTableWidget = new QWidget (myAttributesStack);
  myTableWidgetLayout = new QVBoxLayout (myTableWidget);
  myTableWidgetLayout->setContentsMargins (0, 0, 0, 0);
  myAttributesStack->addWidget (myTableWidget);

  myAttributesStack->setCurrentWidget (myEmptyWidget);
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void ViewControl_PropertyView::Init (const QList<ViewControl_TableModelValues*>& theTableValues)
{
  for (int aTableId = 0; aTableId < theTableValues.size(); aTableId++)
  {
    ViewControl_TableModelValues* aValues = theTableValues[aTableId];

    ViewControl_Table* aTable = findTable (aTableId);

    aTable->Init (aValues);
    ViewControl_Tools::SetDefaultHeaderSections (aTable->GetTableView(), Qt::Horizontal);

    aTable->SetActive (true);
  }
  // hide not used tables
  for (int aTableId = theTableValues.size(); aTableId < myTables.size(); aTableId++)
  {
    ViewControl_Table* aTable = findTable (aTableId, false);
    if (!aTable)
      continue;

    ViewControl_TableModel* aModel = dynamic_cast<ViewControl_TableModel*> (aTable->GetTableView()->model());
    aModel->SetModelValues (0);

    aTable->SetActive (false);
  }

  if (theTableValues.size() > 0)
    myAttributesStack->setCurrentWidget (myTableWidget);
  else
    myAttributesStack->setCurrentWidget (myEmptyWidget);
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void ViewControl_PropertyView::Init (QWidget*)
{
}

// =======================================================================
// function : GetActiveTables
// purpose :
// =======================================================================
void ViewControl_PropertyView::GetActiveTables (QList<ViewControl_Table*>& theTables)
{
  for (int aTableId = 0; aTableId < myTables.size(); aTableId++)
  {
    ViewControl_Table* aTable = findTable (aTableId, false);
    if (aTable && aTable->IsActive())
      theTables.append (aTable);
  }
}

// =======================================================================
// function : ClearActiveTablesSelection
// purpose :
// =======================================================================
void ViewControl_PropertyView::ClearActiveTablesSelection()
{
  bool aWasBlocked = myOwnSelectionChangeBlocked;
  myOwnSelectionChangeBlocked = true;

  QList<ViewControl_Table*> aTables;
  for (int aTableId = 0; aTableId < myTables.size(); aTableId++)
  {
    ViewControl_Table* aTable = findTable (aTableId, false);
    if (aTable && aTable->IsActive())
      aTable->GetTableView()->selectionModel()->clearSelection();
  }
  myOwnSelectionChangeBlocked = aWasBlocked;
}

// =======================================================================
// function : Clear
// purpose :
// =======================================================================
void ViewControl_PropertyView::Clear()
{
  for (int aTableId = 0; aTableId < myTables.size(); aTableId++)
  {
    ViewControl_Table* aTable = findTable (aTableId, false);
    if (!aTable)
      continue;

    ViewControl_TableModel* aModel = dynamic_cast<ViewControl_TableModel*> (aTable->GetTableView()->model());
    aModel->SetModelValues (0);

    aTable->SetActive (true);
  }
  myAttributesStack->setCurrentWidget (myEmptyWidget);
}

// =======================================================================
// function : findTable
// purpose :
// =======================================================================
ViewControl_Table* ViewControl_PropertyView::findTable (const int theTableId, const bool isToCreate)
{
  if (!isToCreate && theTableId >= myTables.size())
    return 0;

  if (theTableId < myTables.size())
    return myTables[theTableId];

  ViewControl_Table* aTable = new ViewControl_Table (myMainWidget);
  ViewControl_TableModel* aModel = new ViewControl_TableModel(aTable->GetTableView());
  aTable->SetModel (aModel);


  connect (aTable->GetTableView()->selectionModel(),
          SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
          this, SLOT(onTableSelectionChanged (const QItemSelection&, const QItemSelection&)));
  connect (aTable->GetProperty(), SIGNAL (propertyChanged()), this, SIGNAL (propertyViewSelectionChanged()));

  if (myXStepValues.size() > theTableId)
    aTable->GetProperty()->SetXStep (myXStepValues[theTableId]);

  if (myDivideValues.size() > theTableId)
    aTable->GetProperty()->SetDivideSize (myDivideValues[theTableId]);

  myTableWidgetLayout->addWidget (aTable->GetControl());

  myTables.insert (theTableId, aTable);

  return myTables[theTableId];
}

// =======================================================================
// function : onTableSelectionChanged
// purpose :
// =======================================================================
void ViewControl_PropertyView::onTableSelectionChanged (const QItemSelection&, const QItemSelection&)
{
  if (myOwnSelectionChangeBlocked)
    return;

  emit propertyViewSelectionChanged();
}

// =======================================================================
// function : SaveState
// purpose :
// =======================================================================
void ViewControl_PropertyView::SaveState (ViewControl_PropertyView* theView,
                                          QMap<QString, QString>& theItems,
                                          const QString& thePrefix)
{
  QList<ViewControl_Table*> anActiveTables;
  theView->GetActiveTables(anActiveTables);

  if (anActiveTables.size() == 0)
    return;

  anActiveTables[0]->GetProperty()->GetXStep();
  QStringList aDivideSizes, aXSteps;
  for (int i = 0; i < anActiveTables.size(); i++)
  {
    aXSteps.append (QString::number (anActiveTables[i]->GetProperty()->GetXStep()));
    aDivideSizes.append (QString::number (anActiveTables[i]->GetProperty()->GetDivideSize()));
  }
  theItems[thePrefix + "property_view_xstep_value"] = aXSteps.join (",");
  theItems[thePrefix + "property_view_divide_value"] = aDivideSizes.join (",");
}

// =======================================================================
// function : RestoreState
// purpose :
// =======================================================================
bool ViewControl_PropertyView::RestoreState (ViewControl_PropertyView* theView,
                                             const QString& theKey,
                                             const QString& theValue,
                                             const QString& thePrefix)
{
  if (theKey == thePrefix + "property_view_xstep_value")
  {
    QList<double> aXStepValues;
    QStringList aValues = theValue.split (",", QString::SkipEmptyParts);
    for (int aValueId = 0; aValueId < aValues.size(); aValueId++)
      aXStepValues.append (aValues.at (aValueId).toDouble());
    theView->SetXSteps (aXStepValues);
  }
  else if (theKey == thePrefix + "property_view_divide_value")
  {
    QList<int> aDivideValues;
    QStringList aValues = theValue.split (",", QString::SkipEmptyParts);
    for (int aValueId = 0; aValueId < aValues.size(); aValueId++)
      aDivideValues.append (aValues.at (aValueId).toInt());
    theView->SetDivideValues (aDivideValues);
  }
  else
    return false;
  return true;
}
