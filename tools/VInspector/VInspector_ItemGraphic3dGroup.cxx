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
  return 10;
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
  switch (theRow)
  {
    case 0: return isFirstColumn ? QVariant ("LineAspect") : QVariant (ViewControl_Tools::GetPointerInfo (aGroup->LineAspect()).ToCString());
    case 1: return isFirstColumn ? QVariant ("FillAreaAspect") : QVariant (ViewControl_Tools::GetPointerInfo (aGroup->FillAreaAspect()).ToCString());
    case 2: return isFirstColumn ? QVariant ("TextAspect") : QVariant (ViewControl_Tools::GetPointerInfo (aGroup->TextAspect()).ToCString());
    case 3: return isFirstColumn ? QVariant ("MarkerAspect") : QVariant (ViewControl_Tools::GetPointerInfo (aGroup->MarkerAspect()).ToCString());
    case 4: return isFirstColumn ? QVariant ("ContainsFacet") : QVariant (aGroup->ContainsFacet());
    case 5: return isFirstColumn ? QVariant ("IsDeleted") : QVariant (aGroup->IsDeleted());
    case 6: return isFirstColumn ? QVariant ("IsEmpty") : QVariant (aGroup->IsEmpty());
    case 7: return isFirstColumn ? QVariant ("IsClosed") : QVariant (aGroup->IsClosed());
    case 8:
    {
      if (isFirstColumn)
        return QVariant ("MinMaxValues");
      Standard_Real aXMin, anYMin, aZMin, aXMax, anYMax, aZMax;
      aGroup->MinMaxValues (aXMin, anYMin, aZMin, aXMax, anYMax, aZMax);
      Bnd_Box aBox;
      aBox.Update(aXMin, anYMin, aZMin, aXMax, anYMax, aZMax);
      return QVariant (ViewControl_Tools::ToString (aBox).ToCString());
    }
    case 9:
    {
      if (isFirstColumn)
        return QVariant ("BoundingBox");
      const Graphic3d_BndBox4f& aBndBox = aGroup->BoundingBox();
      Bnd_Box aBox;
      aBox.Update((Standard_Real )aBndBox.CornerMin().x(),
                  (Standard_Real )aBndBox.CornerMin().y(),
                  (Standard_Real )aBndBox.CornerMin().z(),
                  (Standard_Real )aBndBox.CornerMax().x(),
                  (Standard_Real )aBndBox.CornerMax().y(),
                  (Standard_Real )aBndBox.CornerMax().z());
      return QVariant (ViewControl_Tools::ToString (aBox).ToCString());
    }
  }
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
