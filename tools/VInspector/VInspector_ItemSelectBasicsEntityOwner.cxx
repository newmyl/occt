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

#include <inspector/VInspector_ItemSelectBasicsEntityOwner.hxx>
#include <inspector/VInspector_ItemSelectMgrSensitiveEntity.hxx>
#include <inspector/VInspector_ItemPresentableObject.hxx>
#include <inspector/VInspector_Tools.hxx>
#include <inspector/ViewControl_Tools.hxx>

#include <SelectMgr_SensitiveEntity.hxx>
#include <SelectBasics_SensitiveEntity.hxx>
#include <Standard_Version.hxx>
#include <StdSelect_BRepOwner.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QItemSelectionModel>
#include <QColor>
#include <Standard_WarningsRestore.hxx>

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
QVariant VInspector_ItemSelectBasicsEntityOwner::initValue(int theItemRole) const
{
  switch (theItemRole)
  { 
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    {
      Handle(SelectBasics_EntityOwner) anOwner = getEntityOwner();
      if (anOwner.IsNull())
        return QVariant();

      switch (Column())
      {
        case 0: return anOwner->DynamicType()->Name();
        case 2: return VInspector_Tools::GetPointerInfo (anOwner, true).ToCString();
        case 3:
        {
          Handle(StdSelect_BRepOwner) BROwnr = Handle(StdSelect_BRepOwner)::DownCast (anOwner);
          if (BROwnr.IsNull())
            return QVariant();

          const TopoDS_Shape& aShape = BROwnr->Shape();
          if (aShape.IsNull())
            return QVariant();

          return VInspector_Tools::GetShapeTypeInfo (aShape.ShapeType()).ToCString();
        }
        case 17:
        case 18:
        case 19:
          {
          Handle(StdSelect_BRepOwner) BROwnr = Handle(StdSelect_BRepOwner)::DownCast (anOwner);
          if (BROwnr.IsNull())
            return QVariant();

          const TopoDS_Shape& aShape = BROwnr->Shape();
          if (aShape.IsNull())
            return QVariant();

          return Column() == 17 ? VInspector_Tools::GetPointerInfo (aShape.TShape(), true).ToCString()
               : Column() == 18 ? VInspector_Tools::OrientationToName (aShape.Orientation()).ToCString()
               :           /*19*/ ViewControl_Tools::ToString (aShape.Location()).ToCString();
        }
        case 21:
          {
          Handle(StdSelect_BRepOwner) BROwnr = Handle(StdSelect_BRepOwner)::DownCast (anOwner);
          if (BROwnr.IsNull())
            return QVariant();

          //const TopoDS_Shape& aShape = BROwnr->Shape();
          //if (aShape.IsNull())
          //  return QVariant();

          return ViewControl_Tools::ToString (BROwnr->Location()).ToCString();
        }
        default: break;
      }
      break;
    }
    case Qt::BackgroundRole:
    case Qt::ForegroundRole:
    {
      if (Column() == 2)
      {
        Handle(AIS_InteractiveContext) aContext = GetContext();
        if (!aContext.IsNull())
        {
          if (VInspector_Tools::IsOwnerSelected(aContext, getEntityOwner()))
            return (theItemRole == Qt::BackgroundRole) ? QColor (Qt::darkBlue) : QColor (Qt::white);
        }
      }
      VInspector_ItemSelectMgrSensitiveEntityPtr aParentItem = itemDynamicCast<VInspector_ItemSelectMgrSensitiveEntity>(Parent());
      if (aParentItem)
        return aParentItem->data(QModelIndex(), theItemRole);
      break;
    }
  }
  return QVariant();
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void VInspector_ItemSelectBasicsEntityOwner::Init()
{
  Handle(SelectBasics_EntityOwner) anOwner;

  VInspector_ItemSelectMgrSensitiveEntityPtr aParentItem = itemDynamicCast<VInspector_ItemSelectMgrSensitiveEntity>(Parent());
  if (aParentItem)
  {
    Handle(SelectMgr_SensitiveEntity) anEntity = aParentItem->GetSensitiveEntity();
    anOwner = anEntity->BaseSensitive()->OwnerId();
  }
  else
  {
    VInspector_ItemPresentableObjectPtr aPOItem = itemDynamicCast<VInspector_ItemPresentableObject>(Parent());
    if (aPOItem)
    {
      Handle(AIS_InteractiveObject) anIO = aPOItem->GetInteractiveObject();

      int aRowId = Row();
      int aCurrentIndex = 0;
#if OCC_VERSION_HEX < 0x070201
      for (anIO->Init(); anIO->More() && anOwner.IsNull(); anIO->Next())
      {
        const Handle(SelectMgr_Selection)& aSelection = anIO->CurrentSelection();
        for (aSelection->Init(); aSelection->More() && anOwner.IsNull(); aSelection->Next())
        {
          Handle(SelectMgr_SensitiveEntity) anEntity = aSelection->Sensitive();
#else
      for (SelectMgr_SequenceOfSelection::Iterator aSelIter (anIO->Selections()); aSelIter.More() && anOwner.IsNull(); aSelIter.Next())
      {
        const Handle(SelectMgr_Selection)& aSelection = aSelIter.Value();
        for (NCollection_Vector<Handle(SelectMgr_SensitiveEntity)>::Iterator aSelEntIter (aSelection->Entities()); aSelEntIter.More() && anOwner.IsNull(); aSelEntIter.Next())
        {
          Handle(SelectMgr_SensitiveEntity) anEntity = aSelEntIter.Value();
#endif
          const Handle(SelectBasics_SensitiveEntity)& aBase = anEntity->BaseSensitive();
          if (!aBase.IsNull())
          {
            if (aRowId == aCurrentIndex)
              anOwner = aBase->OwnerId();
            aCurrentIndex++;
          }
        }
      }
    }
  }
  myOwner = anOwner;
  TreeModel_ItemBase::Init();
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================
void VInspector_ItemSelectBasicsEntityOwner::Reset()
{
  VInspector_ItemBase::Reset();
  SetContext (NULL);
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================
void VInspector_ItemSelectBasicsEntityOwner::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<VInspector_ItemSelectBasicsEntityOwner*>(this)->Init();
}

// =======================================================================
// function : getEntityOwner
// purpose :
// =======================================================================
Handle(SelectBasics_EntityOwner) VInspector_ItemSelectBasicsEntityOwner::getEntityOwner() const
{
  initItem();
  return myOwner;
}

// =======================================================================
// function : GetTableRowCount
// purpose :
// =======================================================================
int VInspector_ItemSelectBasicsEntityOwner::GetTableRowCount() const
{
  return 20;
}

// =======================================================================
// function : GetTableEditType
// purpose :
// =======================================================================
ViewControl_EditType VInspector_ItemSelectBasicsEntityOwner::GetTableEditType(const int theRow, const int) const
{
  /*switch (theRow)
  {
  case 4: return ViewControl_EditType_Line;
  case 5: return ViewControl_EditType_Combo;
  case 6: return ViewControl_EditType_Bool;
  case 12: return ViewControl_EditType_Bool;
  case 17: return ViewControl_EditType_Combo;
  case 18: return ViewControl_EditType_Bool;
  default: return ViewControl_EditType_None;
  }*/
  return ViewControl_EditType_None;
}

// =======================================================================
// function : GetTableEnumValues
// purpose :
// =======================================================================
QList<QVariant> VInspector_ItemSelectBasicsEntityOwner::GetTableEnumValues(const int theRow, const int) const
{
  QList<QVariant> aValues;
  /*switch (theRow)
  {
  case 5:
  {
  for (int i = 0; i <= Aspect_TOFM_FRONT_SIDE; i++)
  aValues.append (Aspect::TypeOfFacingModelToString((Aspect_TypeOfFacingModel)i));
  }
  break;
  case 17:
  {
  for (int i = 0; i <= PrsMgr_TOP_ProjectorDependant; i++)
  aValues.append (PrsMgr::TypeOfPresentation3dToString ((PrsMgr_TypeOfPresentation3d)i));
  }
  break;
  default: break;
  }*/
  return aValues;
}

// =======================================================================
// function : GetTableData
// purpose :
// =======================================================================
QVariant VInspector_ItemSelectBasicsEntityOwner::GetTableData(const int theRow, const int theColumn, const int theRole) const
{
  if (theRole != Qt::DisplayRole)
    return QVariant();

  bool isFirstColumn = theColumn == 0;

  /*Handle(AIS_InteractiveObject) aPrs = GetInteractiveObject();
  switch (theRow)
  {
  case 0: return ViewControl_Table::SeparatorData();
  case 1: return isFirstColumn ? QVariant (STANDARD_TYPE (AIS_InteractiveObject)->Name())
  : ViewControl_Tools::GetPointerInfo (aPrs).ToCString();
  case 2: return ViewControl_Table::SeparatorData();
  case 3: return isFirstColumn ? QVariant ("HasWidth") : QVariant (aPrs->HasWidth());
  case 4: return isFirstColumn ? QVariant ("Width") : QVariant (aPrs->Width());
  case 5: return isFirstColumn ? QVariant ("CurrentFacingModel")
  : QVariant (Aspect::TypeOfFacingModelToString (aPrs->CurrentFacingModel()));
  case 6: return isFirstColumn ? QVariant ("IsInfinite") : QVariant (aPrs->IsInfinite());
  case 7: return isFirstColumn ? QVariant ("HasColor") : QVariant (aPrs->HasColor());
  case 8: return isFirstColumn ? QVariant ("HasMaterial") : QVariant (aPrs->HasMaterial());

  case 9: return ViewControl_Table::SeparatorData();
  case 10: return isFirstColumn ? QVariant (STANDARD_TYPE (SelectMgr_SelectableObject)->Name())
  : ViewControl_Tools::GetPointerInfo (aPrs).ToCString();
  case 11: return ViewControl_Table::SeparatorData();
  case 12: return isFirstColumn ? QVariant ("IsAutoHilight") : QVariant (aPrs->IsAutoHilight());
  case 13: return isFirstColumn ? QVariant ("GlobalSelectionMode") : QVariant (aPrs->GlobalSelectionMode());

  case 14: return ViewControl_Table::SeparatorData();
  case 15: return isFirstColumn ? QVariant (STANDARD_TYPE (PrsMgr_PresentableObject)->Name())
  : ViewControl_Tools::GetPointerInfo (aPrs).ToCString();
  case 16: return ViewControl_Table::SeparatorData();
  case 17: return isFirstColumn ? QVariant ("TypeOfPresentation3d")
  : QVariant (PrsMgr::TypeOfPresentation3dToString (aPrs->TypeOfPresentation3d()));
  case 18: return isFirstColumn ? QVariant ("IsMutable") : QVariant (aPrs->IsMutable());
  case 19: return isFirstColumn ? QVariant ("HasOwnPresentations") : QVariant (aPrs->HasOwnPresentations());
  default: return QVariant();
  }*/
  return QVariant();
}

// =======================================================================
// function : SetTableData
// purpose :
// =======================================================================
bool VInspector_ItemSelectBasicsEntityOwner::SetTableData(const int theRow, const int, const QVariant& theValue)
{
  /*Handle(AIS_InteractiveObject) aPrs = GetInteractiveObject();
  switch (theRow)
  {
  case 4:
  {
  double aValue = theValue.toDouble();
  if (aValue > 0) aPrs->SetWidth (aValue);
  else aPrs->UnsetWidth();
  }
  break;
  case 5: aPrs->SetCurrentFacingModel (Aspect::TypeOfFacingModelFromString (theValue.toString().toStdString().c_str()));
  case 6: aPrs->SetInfiniteState (theValue.toBool());
  case 12: aPrs->SetAutoHilight(theValue.toBool());
  case 17: aPrs->SetTypeOfPresentation (PrsMgr::TypeOfPresentation3dFromString (theValue.toString().toStdString().c_str()));
  case 18: aPrs->SetMutable (theValue.toBool());
  default: return false;
  }*/
  return true;
}
