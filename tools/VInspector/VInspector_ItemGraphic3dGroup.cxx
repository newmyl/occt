// Created on: 2018-08-16
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2018 OPEN CASCADE SAS
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

#include <inspector/VInspector_ItemGraphic3dGroup.hxx>

#include <inspector/VInspector_ItemGraphic3dCStructure.hxx>
#include <inspector/VInspector_ItemOpenGlElement.hxx>
#include <inspector/VInspector_Tools.hxx>
#include <inspector/ViewControl_Tools.hxx>

#include <AIS.hxx>
#include <OpenGl_Group.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QStringList>
#include <Standard_WarningsRestore.hxx>

// =======================================================================
// function : GetGroup
// purpose :
// =======================================================================
Handle(Graphic3d_Group) VInspector_ItemGraphic3dGroup::GetGroup() const
{
  initItem();
  return myGroup;
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void VInspector_ItemGraphic3dGroup::Init()
{
  VInspector_ItemGraphic3dCStructurePtr aParentItem = itemDynamicCast<VInspector_ItemGraphic3dCStructure>(Parent());
  myGroup = aParentItem->GetGroup (Row());

  TreeModel_ItemBase::Init();
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================
void VInspector_ItemGraphic3dGroup::Reset()
{
  VInspector_ItemBase::Reset();
  myGroup = NULL;
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================
void VInspector_ItemGraphic3dGroup::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<VInspector_ItemGraphic3dGroup*>(this)->Init();
}

// =======================================================================
// function : initRowCount
// purpose :
// =======================================================================
int VInspector_ItemGraphic3dGroup::initRowCount() const
{
  if (Column() != 0)
    return 0;

  Handle(Graphic3d_Group) aGroup = GetGroup();
  Handle(OpenGl_Group) anOpenGlGroup = Handle(OpenGl_Group)::DownCast(aGroup);
  if (anOpenGlGroup.IsNull())
    return 0;

  int aNbElements = 0;
  for (const OpenGl_ElementNode* aNodeIter = anOpenGlGroup->FirstNode(); aNodeIter != NULL; aNodeIter = aNodeIter->next)
  {
    aNbElements++;
  }
  return aNbElements;
}

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
QVariant VInspector_ItemGraphic3dGroup::initValue (const int theItemRole) const
{
  if (theItemRole != Qt::DisplayRole && theItemRole != Qt::EditRole && theItemRole != Qt::ToolTipRole)
    return QVariant();

  Handle(Graphic3d_Group) aGroup = GetGroup();
  if (aGroup.IsNull())
    return Column() == 0 ? "Empty group" : "";

  switch (Column())
  {
    case 0: return theItemRole == Qt::DisplayRole ? aGroup->DynamicType()->Name()
                                                  : STANDARD_TYPE (Graphic3d_Group)->Name();
    case 1:
      return rowCount();
    case 2:
      return ViewControl_Tools::GetPointerInfo (aGroup, true).ToCString();
    default:
      break;
  }
  return QVariant();
}

// =======================================================================
// function : GetElementNode
// purpose :
// =======================================================================
OpenGl_Element* VInspector_ItemGraphic3dGroup::GetElementNode (const int theRowId) const
{
  int aCurrentIndex = 0;

  Handle(Graphic3d_Group) aGroup = GetGroup();
  Handle(OpenGl_Group) anOpenGlGroup = Handle(OpenGl_Group)::DownCast(aGroup);
  if (anOpenGlGroup.IsNull())
    return 0;
  for (const OpenGl_ElementNode* aNodeIter = anOpenGlGroup->FirstNode(); aNodeIter != NULL; aNodeIter = aNodeIter->next)
  {
    if (theRowId == aCurrentIndex)
      return aNodeIter->elem;
    aCurrentIndex++;
  }
  return 0;
}

// =======================================================================
// function : GetTableRowCount
// purpose :
// =======================================================================
int VInspector_ItemGraphic3dGroup::GetTableRowCount() const
{
  return 40;
}

// =======================================================================
// function : GetTableData
// purpose :
// =======================================================================
QVariant VInspector_ItemGraphic3dGroup::GetTableData (const int theRow, const int theColumn, const int theRole) const
{
  if (theRole != Qt::DisplayRole)
    return QVariant();

  Handle(Graphic3d_Group) aGroup = GetGroup();
  if (aGroup.IsNull())
    return QVariant();

  bool isFirstColumn = theColumn == 0;
  //switch (theRow)
  //{
  //  case 0: return isFirstColumn ? QVariant ("PixelTolerance") : QVariant (myContext->PixelTolerance());
  //  case 1: return isFirstColumn ? QVariant ("PickingStrategy") : QVariant (SelectMgr::PickingStrategyToString (myContext->PickingStrategy()));
  //  case 2: return isFirstColumn ? QVariant ("AutomaticHilight") : QVariant (myContext->AutomaticHilight());
  //  case 3: return isFirstColumn ? QVariant ("ToHilightSelected") : QVariant (myContext->ToHilightSelected());
  //  case 4: return isFirstColumn ? QVariant ("AutoActivateSelection") : QVariant (myContext->GetAutoActivateSelection());
  //  case 5:
  //  case 6:
  //  case 7:
  //  {
  //    AIS_DisplayStatus aDisplayStatus = (AIS_DisplayStatus)(theRow - 5);
  //    if (isFirstColumn)
  //      return QString ("ObjectsByDisplayStatus: %1").arg (AIS::DisplayStatusToString (aDisplayStatus));
  //    AIS_ListOfInteractive anObjects;
  //    myContext->ObjectsByDisplayStatus(aDisplayStatus, anObjects);
  //    return QVariant (anObjects.Extent());
  //  }
  //  break;
  //  case 8: return isFirstColumn ? QVariant ("DetectedOwner") : QVariant (VInspector_Tools::GetPointerInfo (myContext->DetectedOwner()).ToCString());
  //  case 9:
  //  {
  //    if (isFirstColumn)
  //      return QVariant ("DetectedOwners");
  //    int aNbOfDetected = 0;
  //    for (myContext->InitDetected(); myContext->MoreDetected(); myContext->NextDetected())
  //      aNbOfDetected++;
  //    return aNbOfDetected;
  //  }
  //  case 10: return isFirstColumn ? QVariant ("NbSelected") : QVariant (myContext->NbSelected());
  //  default: return QVariant();
  //}
  return QVariant();
}

// =======================================================================
// function : createChild
// purpose :
// =======================================================================
TreeModel_ItemBasePtr VInspector_ItemGraphic3dGroup::createChild (int theRow, int theColumn)
{
  return VInspector_ItemOpenGlElement::CreateItem (currentItem(), theRow, theColumn);
}
