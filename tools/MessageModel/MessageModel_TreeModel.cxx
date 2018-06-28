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

#include <inspector/MessageModel_TreeModel.hxx>

#include <inspector/MessageModel_ItemAlert.hxx>
#include <inspector/MessageModel_ItemRoot.hxx>
#include <inspector/MessageModel_ItemReport.hxx>

const int COLUMN_NAME_WIDTH = 310;
const int COLUMN_SIZE_WIDTH = 30;
const int COLUMN_POINTER_WIDTH = 70;
const int COLUMN_SHAPE_TYPE_WIDTH = 75;
const int COLUMN_STANDARD_TYPE_WIDTH = 120;

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================
MessageModel_TreeModel::MessageModel_TreeModel (QObject* theParent)
: TreeModel_ModelBase (theParent), myIsReversed (Standard_False), myIsUniteAlerts (Standard_False)
{
  SetHeaderItem (0, TreeModel_HeaderSection ("Name", COLUMN_NAME_WIDTH));
  // column 1 is reserved for visiblity state
  SetHeaderItem (2, TreeModel_HeaderSection ("Size", COLUMN_SIZE_WIDTH));

  SetHeaderItem (3, TreeModel_HeaderSection ("Whole Time", -1));
  SetHeaderItem (4, TreeModel_HeaderSection ("Percent", -1));

  SetHeaderItem (5, TreeModel_HeaderSection ("Pointer", COLUMN_POINTER_WIDTH));
  SetHeaderItem (6, TreeModel_HeaderSection ("Shape Type", COLUMN_SHAPE_TYPE_WIDTH));
  SetHeaderItem (7, TreeModel_HeaderSection ("Standard Type", COLUMN_STANDARD_TYPE_WIDTH));
  SetHeaderItem (8, TreeModel_HeaderSection ("Description", -1));
}

// =======================================================================
// function : createRootItem
// purpose :
// =======================================================================
void MessageModel_TreeModel::createRootItem (const int theColumnId)
{
  myRootItems.insert (theColumnId, MessageModel_ItemRoot::CreateItem (TreeModel_ItemBasePtr(), 0, theColumnId));
  if (theColumnId == 0)
      m_pRootItem = myRootItems[0];
}

// =======================================================================
// function : AddShape
// purpose :
// =======================================================================
void MessageModel_TreeModel::AddReport (const Handle(Message_Report)& theReport,
                                        const TCollection_AsciiString& theReportDescription)
{
  for (int aColId = 0, aNbColumns = columnCount(); aColId < aNbColumns; aColId++)
  {
    MessageModel_ItemRootPtr aRootItem = itemDynamicCast<MessageModel_ItemRoot> (RootItem (aColId));
    if (!aRootItem)
      continue;
    aRootItem->AddReport (theReport, theReportDescription);
    aRootItem->SetReversed (myIsReversed);
    aRootItem->SetUniteAlerts (myIsUniteAlerts);
  }

  Reset();
  EmitLayoutChanged();
}

// =======================================================================
// function : SetReport
// purpose :
// =======================================================================
void MessageModel_TreeModel::SetReport (const int theRowId, const Handle(Message_Report)& theReport,
                                        const TCollection_AsciiString& theReportDescription)
{
  for (int aColId = 0, aNbColumns = columnCount(); aColId < aNbColumns; aColId++)
  {
    MessageModel_ItemRootPtr aRootItem = itemDynamicCast<MessageModel_ItemRoot> (RootItem (aColId));
    if (!aRootItem)
      continue;
    aRootItem->SetReport (theRowId, theReport, theReportDescription);
  }
  Reset();
  EmitLayoutChanged();
}

// =======================================================================
// function : SetReversed
// purpose :
// =======================================================================
void MessageModel_TreeModel::SetReversed (const Standard_Boolean& theReversed)
{
  myIsReversed = theReversed;

  for (int aColId = 0, aNbColumns = columnCount(); aColId < aNbColumns; aColId++)
  {
    MessageModel_ItemRootPtr aRootItem = itemDynamicCast<MessageModel_ItemRoot> (RootItem (aColId));
    if (aRootItem)
      aRootItem->SetReversed (myIsReversed);
  }

  Reset();
  EmitLayoutChanged();
}

// =======================================================================
// function : SetUniteAlerts
// purpose :
// =======================================================================
void MessageModel_TreeModel::SetUniteAlerts (const Standard_Boolean& theUniteAlerts)
{
  myIsUniteAlerts = theUniteAlerts;

  for (int aColId = 0, aNbColumns = columnCount(); aColId < aNbColumns; aColId++)
  {
    MessageModel_ItemRootPtr aRootItem = itemDynamicCast<MessageModel_ItemRoot> (RootItem (aColId));
    if (aRootItem)
      aRootItem->SetUniteAlerts (myIsUniteAlerts);
  }

  Reset();
  EmitLayoutChanged();
}

// =======================================================================
// function : UpdateTreeModel
// purpose :
// =======================================================================
void MessageModel_TreeModel::SetRootItemName (const TCollection_AsciiString& theName)
{
  MessageModel_ItemRootPtr aRootItem = itemDynamicCast<MessageModel_ItemRoot> (RootItem (0));
  if (aRootItem)
    aRootItem->SetName (theName);
}

// =======================================================================
// function : UpdateTreeModel
// purpose :
// =======================================================================
void MessageModel_TreeModel::UpdateTreeModel()
{
  Reset();
  EmitLayoutChanged();
}
