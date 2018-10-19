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

#include <inspector/VInspector_ItemV3dViewer.hxx>

#include <AIS.hxx>
#include <AIS_InteractiveContext.hxx>
#include <inspector/VInspector_ItemContext.hxx>
#include <inspector/VInspector_ItemFolderObject.hxx>
#include <inspector/VInspector_ItemV3dView.hxx>
#include <inspector/VInspector_Tools.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QStringList>
#include <Standard_WarningsRestore.hxx>

// =======================================================================
// function : initRowCount
// purpose :
// =======================================================================
int VInspector_ItemV3dViewer::initRowCount() const
{
  if (Column() != 0)
    return 0;

  return 1;
}

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
QVariant VInspector_ItemV3dViewer::initValue (const int theItemRole) const
{
  if (theItemRole != Qt::DisplayRole && theItemRole != Qt::EditRole && theItemRole != Qt::ToolTipRole)
    return QVariant();

  if (GetViewer().IsNull())
    return Column() == 0 ? "Empty viewer" : "";

  switch (Column())
  {
    case 0: return GetViewer()->DynamicType()->Name();
    case 1: return rowCount();
    default:
      break;
  }
  return QVariant();
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================

void VInspector_ItemV3dViewer::Init()
{
  VInspector_ItemFolderObjectPtr aParentItem = itemDynamicCast<VInspector_ItemFolderObject>(Parent());
  Handle(V3d_Viewer) aViewer;
  if (aParentItem)
  {
    VInspector_ItemContextPtr aParentContextItem = itemDynamicCast<VInspector_ItemContext>(aParentItem->Parent());
    if (aParentContextItem)
    {
      Handle(AIS_InteractiveContext) aContext = aParentContextItem->GetContext();
      aViewer = aContext->CurrentViewer();
    }
  }
  setViewer (aViewer);
  TreeModel_ItemBase::Init(); // to use getIO() without circling initialization
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================

void VInspector_ItemV3dViewer::Reset()
{
  VInspector_ItemBase::Reset();

  setViewer (NULL);
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================

void VInspector_ItemV3dViewer::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<VInspector_ItemV3dViewer*>(this)->Init();
}

// =======================================================================
// function : GetViewer
// purpose :
// =======================================================================

Handle(V3d_Viewer) VInspector_ItemV3dViewer::GetViewer() const
{
  initItem();
  return myViewer;
}

// =======================================================================
// function : GetTableRowCount
// purpose :
// =======================================================================
int VInspector_ItemV3dViewer::GetTableRowCount() const
{
  return 0;
}

// =======================================================================
// function : GetTableEditType
// purpose :
// =======================================================================
ViewControl_EditType VInspector_ItemV3dViewer::GetTableEditType (const int theRow, const int) const
{
  return ViewControl_EditType_None;
}

// =======================================================================
// function : GetTableEnumValues
// purpose :
// =======================================================================
QList<QVariant> VInspector_ItemV3dViewer::GetTableEnumValues (const int theRow, const int) const
{
  QList<QVariant> aValues;
  return aValues;
}

// =======================================================================
// function : GetTableData
// purpose :
// =======================================================================
QVariant VInspector_ItemV3dViewer::GetTableData (const int theRow, const int theColumn, const int theRole) const
{
  if (theRole != Qt::DisplayRole)
    return QVariant();
}

// =======================================================================
// function : SetTableData
// purpose :
// =======================================================================
bool VInspector_ItemV3dViewer::SetTableData (const int theRow, const int, const QVariant& theValue)
{
  return true;
}

// =======================================================================
// function : createChild
// purpose :
// =======================================================================
TreeModel_ItemBasePtr VInspector_ItemV3dViewer::createChild (int theRow, int theColumn)
{
  return VInspector_ItemV3dView::CreateItem (currentItem(), theRow, theColumn);
}
