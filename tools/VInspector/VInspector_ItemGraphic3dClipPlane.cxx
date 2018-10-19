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

#include <inspector/VInspector_ItemGraphic3dClipPlane.hxx>

#include <AIS.hxx>
#include <AIS_InteractiveContext.hxx>
#include <inspector/VInspector_ItemContext.hxx>
#include <inspector/VInspector_ItemGraphic3dClipPlane.hxx>
#include <inspector/VInspector_ItemV3dView.hxx>
#include <inspector/VInspector_Tools.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QStringList>
#include <Standard_WarningsRestore.hxx>

// =======================================================================
// function : initRowCount
// purpose :
// =======================================================================
int VInspector_ItemGraphic3dClipPlane::initRowCount() const
{
  if (Column() != 0)
    return 0;

  VInspector_ItemV3dViewPtr aParentViewItem = itemDynamicCast<VInspector_ItemV3dView>(Parent());
  if (aParentViewItem)
    return aParentViewItem->GetView()->ClipPlanes()->Size();

  return 0;
}

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
QVariant VInspector_ItemGraphic3dClipPlane::initValue (const int theItemRole) const
{
  if (theItemRole != Qt::DisplayRole && theItemRole != Qt::EditRole && theItemRole != Qt::ToolTipRole)
    return QVariant();

  if (GetClipPlane().IsNull())
    return Column() == 0 ? "Clip Planes" : "";

  switch (Column())
  {
    case 0: return GetClipPlane()->DynamicType()->Name();
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

void VInspector_ItemGraphic3dClipPlane::Init()
{
  VInspector_ItemV3dViewPtr aParentViewItem = itemDynamicCast<VInspector_ItemV3dView>(Parent());
  Handle(Graphic3d_ClipPlane) aClipPlane;
  if (!aParentViewItem) // ClipPlanes
  {
    aParentViewItem = itemDynamicCast<VInspector_ItemV3dView>(Parent()->Parent());
    aClipPlane = aParentViewItem->GetClipPlane(Row());
  }
  setClipPlane (aClipPlane);
  TreeModel_ItemBase::Init();
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================

void VInspector_ItemGraphic3dClipPlane::Reset()
{
  VInspector_ItemBase::Reset();

  setClipPlane (NULL);
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================

void VInspector_ItemGraphic3dClipPlane::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<VInspector_ItemGraphic3dClipPlane*>(this)->Init();
}

// =======================================================================
// function : GetView
// purpose :
// =======================================================================

Handle(Graphic3d_ClipPlane) VInspector_ItemGraphic3dClipPlane::GetClipPlane() const
{
  initItem();
  return myClipPlane;
}

// =======================================================================
// function : GetTableRowCount
// purpose :
// =======================================================================
int VInspector_ItemGraphic3dClipPlane::GetTableRowCount() const
{
  return 0;
}

// =======================================================================
// function : GetTableEditType
// purpose :
// =======================================================================
ViewControl_EditType VInspector_ItemGraphic3dClipPlane::GetTableEditType (const int theRow, const int) const
{
  return ViewControl_EditType_None;
}

// =======================================================================
// function : GetTableEnumValues
// purpose :
// =======================================================================
QList<QVariant> VInspector_ItemGraphic3dClipPlane::GetTableEnumValues (const int theRow, const int) const
{
  QList<QVariant> aValues;
  return aValues;
}

// =======================================================================
// function : GetTableData
// purpose :
// =======================================================================
QVariant VInspector_ItemGraphic3dClipPlane::GetTableData (const int theRow, const int theColumn, const int theRole) const
{
  if (theRole != Qt::DisplayRole)
    return QVariant();

  return QVariant();
}

// =======================================================================
// function : SetTableData
// purpose :
// =======================================================================
bool VInspector_ItemGraphic3dClipPlane::SetTableData (const int theRow, const int, const QVariant& theValue)
{
  return true;
}

// =======================================================================
// function : createChild
// purpose :
// =======================================================================
TreeModel_ItemBasePtr VInspector_ItemGraphic3dClipPlane::createChild (int theRow, int theColumn)
{
  if (theRow == 0)
    return VInspector_ItemGraphic3dClipPlane::CreateItem (currentItem(), theRow, theColumn);

  return TreeModel_ItemBasePtr();
}
