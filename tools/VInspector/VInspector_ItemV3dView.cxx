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

#include <inspector/VInspector_ItemV3dView.hxx>

#include <AIS.hxx>
#include <AIS_InteractiveContext.hxx>
#include <inspector/VInspector_ItemContext.hxx>
#include <inspector/VInspector_ItemGraphic3dClipPlane.hxx>
#include <inspector/VInspector_ItemV3dViewer.hxx>
#include <inspector/VInspector_Tools.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QStringList>
#include <Standard_WarningsRestore.hxx>

// =======================================================================
// function : initRowCount
// purpose :
// =======================================================================
Handle(Graphic3d_ClipPlane) VInspector_ItemV3dView::GetClipPlane(const int theRow)
{
  Handle(V3d_View) aView = GetView();

  const Handle(Graphic3d_SequenceOfHClipPlane)& aClipPlanes = aView->ClipPlanes();

  Standard_Integer aPlaneId = 0;
  for (Graphic3d_SequenceOfHClipPlane::Iterator aPlaneIt (*aClipPlanes); aPlaneIt.More(); aPlaneIt.Next(), ++aPlaneId)
  {
    if (aPlaneId == theRow)
      return aPlaneIt.Value();
  }
  return 0;
}

// =======================================================================
// function : initRowCount
// purpose :
// =======================================================================
int VInspector_ItemV3dView::initRowCount() const
{
  if (Column() != 0)
    return 0;

  return 1; // ClipPlanes
}

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
QVariant VInspector_ItemV3dView::initValue (const int theItemRole) const
{
  if (theItemRole != Qt::DisplayRole && theItemRole != Qt::EditRole && theItemRole != Qt::ToolTipRole)
    return QVariant();

  if (GetView().IsNull())
    return Column() == 0 ? "Empty view" : "";

  switch (Column())
  {
    case 0: return GetView()->DynamicType()->Name();
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

void VInspector_ItemV3dView::Init()
{
  VInspector_ItemV3dViewerPtr aParentItem = itemDynamicCast<VInspector_ItemV3dViewer>(Parent());
  Handle(V3d_View) aView;
  if (aParentItem)
  {
    Handle(V3d_Viewer) aViewer = aParentItem->GetViewer();
    aViewer->InitActiveViews();
    aView = aViewer->ActiveView();
  }
  setView (aView);
  TreeModel_ItemBase::Init(); // to use getIO() without circling initialization
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================

void VInspector_ItemV3dView::Reset()
{
  VInspector_ItemBase::Reset();

  setView (NULL);
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================

void VInspector_ItemV3dView::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<VInspector_ItemV3dView*>(this)->Init();
}

// =======================================================================
// function : GetView
// purpose :
// =======================================================================

Handle(V3d_View) VInspector_ItemV3dView::GetView() const
{
  initItem();
  return myView;
}

// =======================================================================
// function : GetTableRowCount
// purpose :
// =======================================================================
int VInspector_ItemV3dView::GetTableRowCount() const
{
  return 0;
}

// =======================================================================
// function : GetTableEditType
// purpose :
// =======================================================================
ViewControl_EditType VInspector_ItemV3dView::GetTableEditType (const int theRow, const int) const
{
  return ViewControl_EditType_None;
}

// =======================================================================
// function : GetTableEnumValues
// purpose :
// =======================================================================
QList<QVariant> VInspector_ItemV3dView::GetTableEnumValues (const int theRow, const int) const
{
  QList<QVariant> aValues;
  return aValues;
}

// =======================================================================
// function : GetTableData
// purpose :
// =======================================================================
QVariant VInspector_ItemV3dView::GetTableData (const int theRow, const int theColumn, const int theRole) const
{
  if (theRole != Qt::DisplayRole)
    return QVariant();
}

// =======================================================================
// function : SetTableData
// purpose :
// =======================================================================
bool VInspector_ItemV3dView::SetTableData (const int theRow, const int, const QVariant& theValue)
{
  return true;
}

// =======================================================================
// function : createChild
// purpose :
// =======================================================================
TreeModel_ItemBasePtr VInspector_ItemV3dView::createChild (int theRow, int theColumn)
{
  if (theRow == 0)
    return VInspector_ItemGraphic3dClipPlane::CreateItem (currentItem(), theRow, theColumn);

  return TreeModel_ItemBasePtr();
}
