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

                                                          
#include <inspector/MessageModel_Actions.hxx>

#include <inspector/MessageModel_ItemReport.hxx>
#include <inspector/MessageModel_ItemRoot.hxx>
#include <inspector/MessageModel_ItemAlert.hxx>
#include <inspector/MessageModel_Tools.hxx>
#include <inspector/MessageModel_TreeModel.hxx>
#include <inspector/TInspectorAPI_PluginParameters.hxx>
#include <inspector/ViewControl_Tools.hxx>

#include <TCollection_AsciiString.hxx>
#include <TopoDS_AlertWithShape.hxx>

#include <QAction>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMessageBox>
#include <QWidget>

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================
MessageModel_Actions::MessageModel_Actions (QWidget* theParent,
  MessageModel_TreeModel* theTreeModel, QItemSelectionModel* theModel)
: QObject (theParent), myTreeModel (theTreeModel), mySelectionModel (theModel)
{
  myActions.insert (MessageModel_ActionType_Deactivate,
                    ViewControl_Tools::CreateAction ("Deactivate", SLOT (OnDeactivateReport()), parent(), this));
  myActions.insert (MessageModel_ActionType_Activate,
                    ViewControl_Tools::CreateAction ("Activate", SLOT (OnActivateReport()), parent(), this));
  myActions.insert (MessageModel_ActionType_ExportToShapeView,
                    ViewControl_Tools::CreateAction (tr ("Export to ShapeView"), SLOT (OnExportToShapeView()), parent(), this));
}

// =======================================================================
// function : GetAction
// purpose :
// =======================================================================
QAction* MessageModel_Actions::GetAction (const MessageModel_ActionType& theType)
{
  if (myActions.contains (theType))
    return myActions[theType];

  return 0;
}

// =======================================================================
// function : AddMenuActions
// purpose :
// =======================================================================
void MessageModel_Actions::AddMenuActions (const QModelIndexList& theSelectedIndices, QMenu* theMenu)
{
  MessageModel_ItemRootPtr aRootItem;
  MessageModel_ItemReportPtr aReportItem;
  MessageModel_ItemAlertPtr anAlertItem;
  for (QModelIndexList::const_iterator aSelIt = theSelectedIndices.begin(); aSelIt != theSelectedIndices.end(); aSelIt++)
  {
    QModelIndex anIndex = *aSelIt;
    if (anIndex.column() != 0)
      continue;

    TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
    if (!anItemBase)
      continue;

    aRootItem = itemDynamicCast<MessageModel_ItemRoot> (anItemBase);
    if (aRootItem)
      break;

    aReportItem = itemDynamicCast<MessageModel_ItemReport> (anItemBase);
    if (aReportItem)
      break;

    anAlertItem = itemDynamicCast<MessageModel_ItemAlert> (anItemBase);
    if (anAlertItem)
      break;
  }

  if (aReportItem && !aReportItem->GetReport().IsNull())
  {
    theMenu->addAction (myActions[MessageModel_ActionType_Deactivate]);
    theMenu->addAction (myActions[MessageModel_ActionType_Activate]);
  }
  else if (anAlertItem)
    theMenu->addAction (myActions[MessageModel_ActionType_ExportToShapeView]);

  theMenu->addSeparator();
}

// =======================================================================
// function : onImportReport
// purpose :
// =======================================================================
Handle(Message_Report) MessageModel_Actions::getSelectedReport (QModelIndex& theReportIndex) const
{
  MessageModel_ItemReportPtr aReportItem;
  QModelIndexList aSelectedIndices = mySelectionModel->selectedIndexes();
  for (QModelIndexList::const_iterator aSelIt = aSelectedIndices.begin(); aSelIt != aSelectedIndices.end(); aSelIt++)
  {
    QModelIndex anIndex = *aSelIt;
    if (anIndex.column() != 0)
      continue;

    TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
    if (!anItemBase)
      continue;

    aReportItem = itemDynamicCast<MessageModel_ItemReport> (anItemBase);
    theReportIndex = anIndex;
    if (aReportItem)
      break;
  }
  if (!aReportItem)
    return NULL;

  return aReportItem->GetReport();
}

// =======================================================================
// function : OnDeactivateReport
// purpose :
// =======================================================================
void MessageModel_Actions::OnDeactivateReport()
{
  QModelIndex aReportIndex;
  Handle(Message_Report) aReport = getSelectedReport (aReportIndex);
  if (aReport.IsNull())
    return;

  aReport->SetActive (Standard_False);
  ((MessageModel_TreeModel*)mySelectionModel)->EmitDataChanged (aReportIndex, aReportIndex);
}

// =======================================================================
// function : OnActivateReport
// purpose :
// =======================================================================
void MessageModel_Actions::OnActivateReport()
{
  QModelIndex aReportIndex;
  Handle(Message_Report) aReport = getSelectedReport (aReportIndex);
  if (aReport.IsNull())
    return;

  aReport->SetActive (Standard_True);
  ((MessageModel_TreeModel*)mySelectionModel)->EmitDataChanged (aReportIndex, aReportIndex);
}

// =======================================================================
// function : OnExportToShapeView
// purpose :
// =======================================================================
void MessageModel_Actions::OnExportToShapeView()
{
  TCollection_AsciiString aPluginName ("TKShapeView");

  NCollection_List<Handle(Standard_Transient)> aPluginParameters;
  if (myParameters->FindParameters (aPluginName))
    aPluginParameters = myParameters->Parameters (aPluginName);
  NCollection_List<TCollection_AsciiString> anItemNames;
  if (myParameters->FindSelectedNames (aPluginName))
    anItemNames = myParameters->GetSelectedNames (aPluginName);

  QModelIndexList aSelectedIndices = mySelectionModel->selectedIndexes();
  QStringList anExportedPointers;
  for (QModelIndexList::const_iterator aSelIt = aSelectedIndices.begin(); aSelIt != aSelectedIndices.end(); aSelIt++)
  {
    QModelIndex anIndex = *aSelIt;
    if (anIndex.column() != 0)
      continue;

    TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (anIndex);
    if (!anItemBase)
      continue;

    MessageModel_ItemAlertPtr anAlertItem = itemDynamicCast<MessageModel_ItemAlert> (anItemBase);
    if (!anAlertItem)
      continue;

    Handle(Message_Alert) anAlert = anAlertItem->GetAlert();
    if (anAlert.IsNull())
      continue;

    Handle(TopoDS_AlertWithShape) aShapeAlert = Handle(TopoDS_AlertWithShape)::DownCast (anAlert);
    if (aShapeAlert.IsNull() || aShapeAlert->GetShape().IsNull())
      continue;

    const TopoDS_Shape aShape = aShapeAlert->GetShape();
    if (aShape.IsNull())
      continue;
    aPluginParameters.Append (aShape.TShape());
    anItemNames.Append (TInspectorAPI_PluginParameters::ParametersToString (aShape));

    anExportedPointers.append (MessageModel_Tools::GetPointerInfo (aShape.TShape(), true).ToCString());
  }

  if (anExportedPointers.empty())
      return;
  myParameters->SetSelectedNames (aPluginName, anItemNames);
  myParameters->SetParameters (aPluginName, aPluginParameters);
  QMessageBox::information (0, "Information", QString ("TShapes '%1' are sent to %2 tool.")
    .arg (anExportedPointers.join (", ")).arg (QString (aPluginName.ToCString())));
}
