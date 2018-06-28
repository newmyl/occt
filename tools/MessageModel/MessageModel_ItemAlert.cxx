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

#include <inspector/MessageModel_ItemAlert.hxx>

#include <inspector/MessageModel_ItemRoot.hxx>
#include <inspector/MessageModel_ItemReport.hxx>
#include <inspector/MessageModel_Tools.hxx>
#include <inspector/TreeModel_Tools.hxx>

#include <Message_AlertExtended.hxx>
#include <Message_AttributeVectorOfReal.hxx>
#include <Message_AttributeVectorOfRealVec3.hxx>
#include <Message_CompositeAlerts.hxx>

#include <TCollection_AsciiString.hxx>
#include <TopoDS_AlertWithShape.hxx>

#include <QColor>
#include <QIcon>

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
QVariant MessageModel_ItemAlert::initValue (const int theRole) const
{
  if (theRole == Qt::ForegroundRole)
  {
    MessageModel_ItemReportPtr aReportItem = MessageModel_ItemReport::FindReportItem (Parent());
    if (!aReportItem)
      return QVariant();

    Handle(Message_Report) aReport = aReportItem->GetReport();
    if (aReport.IsNull())
      return QVariant();

    if (!aReport->GetAlerts (Message_Fail).IsEmpty())
      return QColor(Qt::darkRed);

    for (int aGravityId = (int)Message_Trace; aGravityId <= (int)Message_Fail; aGravityId++)
    {
      if (!aReport->IsActive ((Message_Gravity)aGravityId))
        return QColor(Qt::darkGray);
    }
    return QVariant();
  }

  Handle(Message_Alert) anAlert = getAlert();
  if (anAlert.IsNull())
    return QVariant();

  // if the alert is composite, process the real alert
  if (theRole == Qt::DecorationRole && Column() == 0)
  {
    if (anAlert->IsKind (STANDARD_TYPE (TopoDS_AlertWithShape)))
      return QIcon (":/icons/item_shape.png");
    else if (anAlert->IsKind (STANDARD_TYPE (Message_AttributeVectorOfReal)))
      return QIcon (":/icons/item_vectorOfReal.png");
    else if (anAlert->IsKind (STANDARD_TYPE (Message_AttributeVectorOfRealVec3)))
      return QIcon (":/icons/item_vectorOfRealVec3.png");
    else
      return QVariant();
  }

  if (theRole != Qt::DisplayRole && theRole != Qt::ToolTipRole)
    return QVariant();

  const Message_ListOfAlert& aUnitedAlerts = GetUnitedAlerts();
  if (anAlert.IsNull() && !aUnitedAlerts.IsEmpty()) // give information about last united alert
    anAlert = !isReversed() ? aUnitedAlerts.Last() : aUnitedAlerts.First();
  if (anAlert.IsNull())
    return QVariant();

  switch (Column())
  {
    case 0:
      return theRole == Qt::DisplayRole ? anAlert->GetMessageKey() : anAlert->DynamicType()->Name();
    case 1: return QVariant(); // visibility state
    case 2: return rowCount() > 0 ? QVariant (rowCount()) : QVariant();
    //case 3: return anAlert->ElapsedTime() < 0 ? QVariant() : anAlert->ElapsedTime();
    case 3: return AmountElapsedTime (anAlert);
    case 4:
    {
      TreeModel_ItemBasePtr aParentItem = Parent();
      MessageModel_ItemReportPtr aReportItem = itemDynamicCast<MessageModel_ItemReport> (aParentItem);
      while (!aReportItem)
      {
        aParentItem = aParentItem->Parent();
        aReportItem = itemDynamicCast<MessageModel_ItemReport> (aParentItem);
      }
      double aDivideTo = MessageModel_ItemReport::AmountElapsedTime (aReportItem->GetReport());
      return aDivideTo == 0 ? QVariant() : 100. * AmountElapsedTime (anAlert) / aDivideTo;
    }
    case 5: return MessageModel_Tools::GetPointerAlertInfo (anAlert).ToCString();
    case 6: return MessageModel_Tools::GetShapeTypeAlertInfo (anAlert).ToCString();
    case 7: return MessageModel_Tools::GetStandardTypeAlertInfo (anAlert).ToCString();
    case 8:
    {
      Handle(Message_AlertExtended) anExtendedAlert = Handle(Message_AlertExtended)::DownCast(anAlert);
      if (!anExtendedAlert.IsNull() && !anExtendedAlert->Attribute().IsNull())
      {
        TCollection_AsciiString aDescription = anExtendedAlert->Attribute()->GetDescription();
        return theRole == Qt::DisplayRole ? TreeModel_Tools::CutString (aDescription.ToCString())
                                          : aDescription.ToCString();
      }
    }
    default: break;
  }
  return QVariant();
}

// =======================================================================
// function : initRowCount
// purpose :
// =======================================================================
int MessageModel_ItemAlert::initRowCount() const
{
  const Handle(Message_Alert)& anAlert = getAlert();
  if (anAlert.IsNull())
    return GetUnitedAlerts().Size();

  Handle(Message_AlertExtended) anExtendedAlert = Handle(Message_AlertExtended)::DownCast(anAlert);
  if (anExtendedAlert.IsNull())
    return GetUnitedAlerts().Size();

  Handle(Message_CompositeAlerts) aCompositeAlert = anExtendedAlert->GetCompositeAlerts();
  if (aCompositeAlert.IsNull())
    return GetUnitedAlerts().Size();

  int aRowCount = 0;
  NCollection_Vector<Message_ListOfAlert> aUnitedAlerts;
  for (int aGravityId = Message_Trace; aGravityId <= Message_Fail; aGravityId++)
  {
    const Message_ListOfAlert& anAlerts  = aCompositeAlert->GetAlerts ((Message_Gravity)aGravityId);
    if (isUniteAlerts())
      GetUnitedAlerts (anAlerts, aUnitedAlerts);
    else
      aRowCount += anAlerts.Size();
  }
  return isUniteAlerts() ? aUnitedAlerts.Size() : aRowCount;
}

// =======================================================================
// function : createChild
// purpose :
// =======================================================================
TreeModel_ItemBasePtr MessageModel_ItemAlert::createChild (int theRow, int theColumn)
{
  return MessageModel_ItemAlert::CreateItem (currentItem(), theRow, theColumn);
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void MessageModel_ItemAlert::Init()
{
  MessageModel_ItemReportPtr aReportItem = itemDynamicCast<MessageModel_ItemReport> (Parent());
  MessageModel_ItemAlertPtr anAlertItem;
  Handle(Message_Report) aReport;
  Handle(Message_Alert) anAlert;
  if (aReportItem)
    aReport = aReportItem->GetReport();
  else
  {
    anAlertItem = itemDynamicCast<MessageModel_ItemAlert> (Parent());
    if (anAlertItem)
      anAlert = anAlertItem->GetAlert();
  }
  if (aReport.IsNull() && anAlert.IsNull() && !anAlertItem)
    return;

  if (anAlert.IsNull() && anAlertItem) // union folder item
  {
    int aCurrentSubId = 0;
    for (Message_ListOfAlert::Iterator anAlertsIt (anAlertItem->GetUnitedAlerts()); anAlertsIt.More();
         anAlertsIt.Next(), aCurrentSubId++)
    {
      if (aCurrentSubId != Row())
        continue;
      myAlert = anAlertsIt.Value();
      MessageModel_ItemBase::Init();
      return;
    }
    return;
  }

  // iterates through all gravity types, skip types where report is empty, if report is not empty, increment
  // current index until it equal to the current row index
  Message_ListOfAlert anAlerts;
  NCollection_Vector<Message_ListOfAlert> aUnitedAlerts;
  int aRowId = Row();
  int aPreviousAlertsCount = 0;
  for (int aGravityId = Message_Trace; aGravityId <= Message_Fail; aGravityId++)
  {
    if (!aReport.IsNull())
      anAlerts = aReport->GetAlerts ((Message_Gravity)aGravityId);
    else if (!anAlert.IsNull())
    {
      Handle(Message_AlertExtended) anExtendedAlert = Handle(Message_AlertExtended)::DownCast(anAlert);
      Handle(Message_CompositeAlerts) aCompositeAlert = !anExtendedAlert.IsNull() ? anExtendedAlert->GetCompositeAlerts()
        : Handle(Message_CompositeAlerts)();
      if (!aCompositeAlert.IsNull())
        anAlerts = aCompositeAlert->GetAlerts ((Message_Gravity)aGravityId);
    }

    if (isReversed())
      anAlerts.Reverse();

    if (isUniteAlerts())
    {
      GetUnitedAlerts (anAlerts, aUnitedAlerts);
      if (aRowId < aUnitedAlerts.Size())
      {
        anAlerts = aUnitedAlerts.Value (aRowId);

        if (anAlerts.Size() == 1)
          myAlert = anAlerts.First();
        else
          myUnitedAlerts = anAlerts;

        MessageModel_ItemBase::Init();
        return;
      }
    }
    else
    {
      if (aRowId < aPreviousAlertsCount + anAlerts.Size())
      {
        aRowId = aRowId - aPreviousAlertsCount;
        int aCurrentId = 0;
        for (Message_ListOfAlert::Iterator anAlertsIt (anAlerts); anAlertsIt.More(); anAlertsIt.Next(), aCurrentId++)
        {
          if (aCurrentId != Row())
            continue;
          myAlert = anAlertsIt.Value();
          MessageModel_ItemBase::Init();
          return;
        }
      }
      aPreviousAlertsCount += anAlerts.Size();
    }
  }
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================
void MessageModel_ItemAlert::Reset()
{
  MessageModel_ItemBase::Reset();
  myAlert = Handle(Message_Alert)();
  myUnitedAlerts.Clear();
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================
void MessageModel_ItemAlert::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<MessageModel_ItemAlert*>(this)->Init();
}

// =======================================================================
// function : getAlert
// purpose :
// =======================================================================
const Handle(Message_Alert)& MessageModel_ItemAlert::getAlert() const
{
  initItem();
  return myAlert;
}

// =======================================================================
// function : getUnitedAlerts
// purpose :
// =======================================================================
const Message_ListOfAlert& MessageModel_ItemAlert::getUnitedAlerts() const
{
  initItem();
  return myUnitedAlerts;
}

// =======================================================================
// function : AmountElapsedTime
// purpose :
// =======================================================================
double MessageModel_ItemAlert::AmountElapsedTime(const Handle(Message_Alert)& theAlert)
{
  if (theAlert.IsNull())
    return 0;

  double anAmountTime = 0;
  Handle(Message_AlertExtended) anExtendedAlert = Handle(Message_AlertExtended)::DownCast(theAlert);
  if (anExtendedAlert->GetCumulativeTime() > 0)
    anAmountTime = anExtendedAlert->GetCumulativeTime();

  return anAmountTime;
}

// =======================================================================
// function : GetUnitedAlerts
// purpose :
// =======================================================================
void MessageModel_ItemAlert::GetUnitedAlerts(const Message_ListOfAlert& theAlerts,
                       NCollection_Vector<Message_ListOfAlert>& theUnitedAlerts)
{
  //theUnitedAlerts.Clear();
  TCollection_AsciiString anAlertMessageKey;
  for (Message_ListOfAlert::Iterator anAlertsIt (theAlerts); anAlertsIt.More(); anAlertsIt.Next())
  {
    Handle(Message_Alert) anAlert = anAlertsIt.Value();
    if (anAlertMessageKey.IsEqual (anAlert->GetMessageKey())) {
      Message_ListOfAlert anAlerts = theUnitedAlerts.Last();
      anAlerts.Append (anAlert);
      theUnitedAlerts.SetValue(theUnitedAlerts.Size()-1, anAlerts);
    }
    else {
      Message_ListOfAlert anAlerts;
      anAlerts.Append (anAlert);
      theUnitedAlerts.Append (anAlerts);
      anAlertMessageKey = anAlert->GetMessageKey();
    }
  }
}
