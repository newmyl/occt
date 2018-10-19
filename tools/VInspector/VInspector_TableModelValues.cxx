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

#include <inspector/VInspector_TableModelValues.hxx>

#include <inspector/ViewControl_TableModel.hxx>
#include <inspector/ViewControl_TableModelFilter.hxx>
#include <inspector/VInspector_Tools.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QFont>
#include <Standard_WarningsRestore.hxx>

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================

VInspector_TableModelValues::VInspector_TableModelValues (const TreeModel_ItemBasePtr& theItem)
 : ViewControl_TableModelValues(), myItem (theItem)
{
  QList<TreeModel_HeaderSection> aHeaderValues;
  aHeaderValues.append(TreeModel_HeaderSection ("Function", 190));
  aHeaderValues.append(TreeModel_HeaderSection ("Value", -2));
  SetHeaderValues(aHeaderValues, Qt::Horizontal);

  SetHeaderVisible(Qt::Horizontal, Standard_False);
  SetHeaderVisible(Qt::Vertical, Standard_False);

  SetUseTableSeparateSize (false);
  SetUseTableProperties (false);
  SetUseTablePropertiesXStep (false, -1);
}

// =======================================================================
// function : Data
// purpose :
// =======================================================================

QVariant VInspector_TableModelValues::Data (const int theRow, const int theColumn, int theRole) const
{
  switch (theRole)
  {
    case Qt::FontRole:
    {
      if (theColumn == 0)
      {
        QFont aFont = qApp->font();
        aFont.setItalic (true);
        return aFont;
      }
    }
    case Qt::ForegroundRole:
    {
      if (theColumn == 0)
        return QColor (Qt::darkGray).darker(150);
      else if (GetEditType (theRow, theColumn) != ViewControl_EditType_None)
        return ViewControl_TableModelValues::EditCellColor();
    }
    default:
      return GetItem()->GetTableData (theRow, theColumn, theRole);
  }
}

// =======================================================================
// function : GetRangeValues
// purpose :
// =======================================================================

bool VInspector_TableModelValues::SetData (const int theRow, const int theColumn, const QVariant& theValue, int theRole)
{
  if (theRole != Qt::EditRole)
    return false;

  return GetItem()->SetTableData (theRow, theColumn, theValue);
}

// =======================================================================
// function : Flags
// purpose :
// =======================================================================

Qt::ItemFlags VInspector_TableModelValues::Flags (const QModelIndex& theIndex) const
{
  Qt::ItemFlags aFlags = ViewControl_TableModelValues::Flags (theIndex);

  if (theIndex.column() == 1)
    aFlags = aFlags | Qt::ItemIsEditable;

  return aFlags;
}

// =======================================================================
// function : GetRangeValues
// purpose :
// =======================================================================

int VInspector_TableModelValues::GetValuesCount () const
{
  return GetItem()->GetTableRowCount() * 2;
}

// =======================================================================
// function : GetEditType
// purpose :
// =======================================================================

ViewControl_EditType VInspector_TableModelValues::GetEditType (const int theRow, const int theColumn) const
{
  if (theColumn == 0)
    return ViewControl_EditType_None;

  return GetItem()->GetTableEditType (theRow, theColumn);
}

// =======================================================================
// function : GetEnumValues
// purpose :
// =======================================================================

QList<QVariant> VInspector_TableModelValues::GetEnumValues (const int theRow, const int theColumn) const
{
  if (theColumn != 1)
    return QList<QVariant>();

  return GetItem()->GetTableEnumValues (theRow, theColumn);
}

// =======================================================================
// function : GetItem
// purpose :
// =======================================================================

VInspector_ItemBasePtr VInspector_TableModelValues::GetItem() const
{
  return itemDynamicCast<VInspector_ItemBase>(myItem);
}
