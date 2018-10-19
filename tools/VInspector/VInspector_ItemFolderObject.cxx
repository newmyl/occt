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

#include <inspector/VInspector_ItemFolderObject.hxx>

#include <inspector/VInspector_ItemContext.hxx>
#include <inspector/VInspector_ItemPresentableObject.hxx>
#include <inspector/VInspector_ItemPrs3dDrawer.hxx>
#include <inspector/VInspector_ItemSelectMgrFilter.hxx>
#include <inspector/VInspector_ItemV3dViewer.hxx>

#include <AIS_InteractiveObject.hxx>
#include <Prs3d.hxx>

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
QVariant VInspector_ItemFolderObject::initValue (int theItemRole) const
{
  if (Column() != 0 || (theItemRole != Qt::DisplayRole && theItemRole != Qt::ToolTipRole))
    return QVariant();

  ParentKind aParentKind = GetParentItemKind();
  switch (aParentKind)
  {
    case ParentKind_ContextItem: return "Properties";
    case ParentKind_PresentationItem: return "Properties";
    case ParentKind_FolderItem: return Row() == 0 ? "Filters" : QVariant();
    default: return QVariant();
  }
}

// =======================================================================
// function : initRowCount
// purpose :
// =======================================================================
int VInspector_ItemFolderObject::initRowCount() const
{
  ParentKind aParentKind = GetParentItemKind();
  switch (aParentKind)
  {
    case ParentKind_ContextItem:
    {
      int aNbChildren = 2; // Filters, Viewer
      aNbChildren++; // DefaultDrawer
      for (int aTypeId = 0; aTypeId < Prs3d_TypeOfHighlight_NB; aTypeId++)
      {
        const Handle(Prs3d_Drawer)& aStyle = GetContext()->HighlightStyle ((Prs3d_TypeOfHighlight)aTypeId);
        if (!aStyle.IsNull())
          aNbChildren++;
      }
      return aNbChildren;
    }
    case ParentKind_PresentationItem:
    {
      return 3; // Attributes, HilightAttributes and DynamicHilightAttributes
    }
    case ParentKind_FolderItem:
      return (GetContext().IsNull() ? 0 : GetContext()->Filters().Extent());
    default:
      return 0;
  }
}

// =======================================================================
// function : createChild
// purpose :
// =======================================================================
TreeModel_ItemBasePtr VInspector_ItemFolderObject::createChild (int theRow, int theColumn)
{
  ParentKind aParentKind = GetParentItemKind();
  switch (aParentKind)
  {
    case ParentKind_ContextItem:
    {
      if (theRow == 0)
        return VInspector_ItemFolderObject::CreateItem (currentItem(), theRow, theColumn);
      else if (theRow == 1)
        return VInspector_ItemV3dViewer::CreateItem (currentItem(), theRow, theColumn);
      else
        return VInspector_ItemPrs3dDrawer::CreateItem (currentItem(), theRow, theColumn);
    }
    case ParentKind_PresentationItem:
      return VInspector_ItemPrs3dDrawer::CreateItem (currentItem(), theRow, theColumn);
    case ParentKind_FolderItem:
      return VInspector_ItemSelectMgrFilter::CreateItem (currentItem(), theRow, theColumn);
    default: return TreeModel_ItemBasePtr();
  }
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void VInspector_ItemFolderObject::Init()
{
  TreeModel_ItemBase::Init(); // to use getIO() without circling initialization
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================
void VInspector_ItemFolderObject::Reset()
{
  VInspector_ItemBase::Reset();
}

// =======================================================================
// function : GetPrs3dDrawer
// purpose :
// =======================================================================
Handle(Prs3d_Drawer) VInspector_ItemFolderObject::GetPrs3dDrawer (const int theRow,
                                                                  TCollection_AsciiString& theName) const
{
  ParentKind aParentKind = GetParentItemKind();
  switch (aParentKind)
  {
    case ParentKind_ContextItem:
    {
      if (theRow == 0 || theRow == 1) // "Filters", "Viewer"
        return 0;

      if (theRow == 2)
      {
        theName = "DefaultDrawer";
        return GetContext()->DefaultDrawer();
      }

      for (int aTypeId = 0, aCurId = 0; aTypeId < Prs3d_TypeOfHighlight_NB; aTypeId++)
      {
        Prs3d_TypeOfHighlight aType = (Prs3d_TypeOfHighlight)aTypeId;
        const Handle(Prs3d_Drawer)& aDrawer = GetContext()->HighlightStyle (aType);
        if (aDrawer.IsNull())
          continue;
        if (aCurId == theRow - 3)
        {
          theName = TCollection_AsciiString ("HighlightStyle: ") + Prs3d::TypeOfHighlightToString (aType);
          return aDrawer;
        }
        aCurId++;
      }
    }
    case ParentKind_PresentationItem:
    {
      VInspector_ItemPresentableObjectPtr aParentPrsItem = itemDynamicCast<VInspector_ItemPresentableObject>(Parent());
      Handle(AIS_InteractiveObject) aPrs = aParentPrsItem->GetInteractiveObject();
      switch (theRow)
      {
        case 0: theName = "Attributes"; return aPrs->Attributes();
        case 1: theName = "HilightAttributes"; return aPrs->HilightAttributes();
        case 2: theName = "DynamicHilightAttributes"; return aPrs->DynamicHilightAttributes();
        default: break;
      }
    }
    case ParentKind_FolderItem:
    default: break;
  }

  theName = "None";
  return Handle(Prs3d_Drawer)();
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================
void VInspector_ItemFolderObject::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<VInspector_ItemFolderObject*> (this)->Init();
}

// =======================================================================
// function : GetParentItemKind
// purpose :
// =======================================================================
VInspector_ItemFolderObject::ParentKind VInspector_ItemFolderObject::GetParentItemKind() const
{
  VInspector_ItemPresentableObjectPtr aParentPrsItem = itemDynamicCast<VInspector_ItemPresentableObject>(Parent());
  if (aParentPrsItem)
    return ParentKind_PresentationItem;

  VInspector_ItemContextPtr aParentContextItem = itemDynamicCast<VInspector_ItemContext>(Parent());
  if (aParentContextItem)
    return ParentKind_ContextItem;

  return ParentKind_FolderItem;
}
