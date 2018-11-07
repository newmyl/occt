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

#include <inspector/VInspector_ItemGraphic3dCStructure.hxx>
#include <inspector/VInspector_ItemGraphic3dGroup.hxx>

#include <inspector/VInspector_ItemPrs3dPresentation.hxx>
#include <inspector/VInspector_Tools.hxx>
#include <inspector/ViewControl_Tools.hxx>

#include <Graphic3d.hxx>
#include <Graphic3d_CStructure.hxx>
#include <Graphic3d_GraphicDriver.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QStringList>
#include <Standard_WarningsRestore.hxx>

// =======================================================================
// function : GetCStructure
// purpose :
// =======================================================================
Handle(Graphic3d_CStructure) VInspector_ItemGraphic3dCStructure::GetCStructure() const
{
  initItem();
  return myCStructure;
}

// =======================================================================
// function : GetGroup
// purpose :
// =======================================================================
Handle(Graphic3d_Group) VInspector_ItemGraphic3dCStructure::GetGroup (const int theRowId) const
{
  Handle(Graphic3d_CStructure) aCStructure = GetCStructure();
  if (aCStructure.IsNull())
    return Handle(Graphic3d_Group)();

  const Graphic3d_SequenceOfGroup& aGroups = aCStructure->Groups();
  int aCurrentIndex = 0;
  for (Graphic3d_SequenceOfGroup::Iterator aGroupIter (aGroups); aGroupIter.More(); aGroupIter.Next())
  {
    if (theRowId == aCurrentIndex)
      return aGroupIter.Value();

    aCurrentIndex++;
  }
  return Handle(Graphic3d_Group)();
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void VInspector_ItemGraphic3dCStructure::Init()
{
  VInspector_ItemPrs3dPresentationPtr aParentItem = itemDynamicCast<VInspector_ItemPrs3dPresentation>(Parent());

  Handle(Prs3d_Presentation) aPresentation = aParentItem->GetPresentation();
  myCStructure = aPresentation->CStructure();

  TreeModel_ItemBase::Init();
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================
void VInspector_ItemGraphic3dCStructure::Reset()
{
  VInspector_ItemBase::Reset();
  myCStructure = NULL;
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================
void VInspector_ItemGraphic3dCStructure::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<VInspector_ItemGraphic3dCStructure*>(this)->Init();
}

// =======================================================================
// function : initRowCount
// purpose :
// =======================================================================
int VInspector_ItemGraphic3dCStructure::initRowCount() const
{
  if (Column() != 0)
    return 0;

  Handle(Graphic3d_CStructure) aCStructure = GetCStructure();
  return !aCStructure.IsNull() ? aCStructure->Groups().Length() : 0;
}

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
QVariant VInspector_ItemGraphic3dCStructure::initValue (const int theItemRole) const
{
  if (theItemRole != Qt::DisplayRole && theItemRole != Qt::EditRole && theItemRole != Qt::ToolTipRole)
    return QVariant();

  Handle(Graphic3d_CStructure) aCStructure = GetCStructure();
  if (aCStructure.IsNull())
    return Column() == 0 ? "Empty CStructure" : "";

  switch (Column())
  {
    case 0: return theItemRole == Qt::DisplayRole ? aCStructure->DynamicType()->Name()
                                                  : STANDARD_TYPE (Graphic3d_CStructure)->Name();
    case 1: return rowCount();
    default:
      break;
  }
  return QVariant();
}

// =======================================================================
// function : GetTableRowCount
// purpose :
// =======================================================================
int VInspector_ItemGraphic3dCStructure::GetTableRowCount() const
{
  return 40;
}

// =======================================================================
// function : GetTableData
// purpose :
// =======================================================================
QVariant VInspector_ItemGraphic3dCStructure::GetTableData (const int theRow, const int theColumn, const int theRole) const
{
  if (theRole != Qt::DisplayRole)
    return QVariant();

  bool isFirstColumn = theColumn == 0;

  Handle(Graphic3d_CStructure) aCStructure = GetCStructure();
  if (aCStructure.IsNull())
    return QVariant();

  switch (theRow)
  {
    case 0: return isFirstColumn ? QVariant ("GraphicDriver")
      : QVariant (ViewControl_Tools::GetPointerInfo (aCStructure->GraphicDriver()).ToCString());
    case 1: return isFirstColumn ? QVariant ("Groups") : QVariant (aCStructure->Groups().Length());
    case 2: return isFirstColumn ? QVariant ("Transformation")
      : (!aCStructure->Transformation().IsNull() ?
         QVariant (ViewControl_Tools::ToString (aCStructure->Transformation()->Trsf()).ToCString()) : QVariant());
    case 3: return isFirstColumn ? QVariant ("TransformPersistence")
                                 : QVariant (ViewControl_Tools::GetPointerInfo (aCStructure->TransformPersistence()).ToCString());
    case 4: return isFirstColumn ? QVariant ("ClipPlanes")
      : (!aCStructure->ClipPlanes().IsNull() ? QVariant (aCStructure->ClipPlanes()->Size()) : QVariant());
    //case 5: return isFirstColumn ? QVariant ("BoundingBox")
    //                             : QVariant (ViewControl_Tools::ToString (aCStructure->BoundingBox()).ToCString());
    case 6: return isFirstColumn ? QVariant ("HighlightStyle")
                                 : QVariant (ViewControl_Tools::GetPointerInfo (aCStructure->HighlightStyle()).ToCString());

    case 7: return isFirstColumn ? QVariant ("Id") : QVariant (aCStructure->Id);
    case 8: return isFirstColumn ? QVariant ("ZLayer")
                                 : QVariant (Graphic3d::ZLayerIdToString(aCStructure->ZLayer()));
    case 9: return isFirstColumn ? QVariant ("Priority") : QVariant (aCStructure->Priority);
    case 10: return isFirstColumn ? QVariant ("PreviousPriority") : QVariant (aCStructure->PreviousPriority);
    case 11: return isFirstColumn ? QVariant ("ContainsFacet") : QVariant (aCStructure->ContainsFacet);
    case 12: return isFirstColumn ? QVariant ("IsInfinite") : QVariant (aCStructure->IsInfinite);
    
    case 13: return isFirstColumn ? QVariant ("stick") : QVariant (aCStructure->stick);
    case 14: return isFirstColumn ? QVariant ("highlight") : QVariant (aCStructure->highlight);
    case 15: return isFirstColumn ? QVariant ("visible") : QVariant (aCStructure->visible);
    case 16: return isFirstColumn ? QVariant ("HLRValidation") : QVariant (aCStructure->HLRValidation);
    case 17: return isFirstColumn ? QVariant ("IsForHighlight") : QVariant (aCStructure->IsForHighlight);
    case 18: return isFirstColumn ? QVariant ("IsMutable") : QVariant (aCStructure->IsMutable);
    case 19: return isFirstColumn ? QVariant ("Is2dText") : QVariant (aCStructure->Is2dText);

    default: return QVariant();
  }
}

// =======================================================================
// function : createChild
// purpose :
// =======================================================================
TreeModel_ItemBasePtr VInspector_ItemGraphic3dCStructure::createChild (int theRow, int theColumn)
{
  Handle(Graphic3d_CStructure) aCStructure = GetCStructure();
  //Handle(Prs3d_Presentation) aPresentation = GetPresentation();
  //Standard_EXPORT const Handle(Graphic3d_SequenceOfHClipPlane)& ClipPlanes() const;

  if (theRow >= 0 && theRow < GetCStructure()->Groups().Length())
    return VInspector_ItemGraphic3dGroup::CreateItem (currentItem(), theRow, theColumn);
  //else
  //  return VInspector_ItemPresentableObject::CreateItem (currentItem(), theRow, theColumn);

  return TreeModel_ItemBasePtr();
}
