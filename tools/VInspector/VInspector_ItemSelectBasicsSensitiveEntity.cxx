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


#include <inspector/VInspector_ItemSelectBasicsSensitiveEntity.hxx>

#include <AIS_ListOfInteractive.hxx>
#include <AIS_Shape.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <SelectBasics_SensitiveEntity.hxx>
#include <Select3D_SensitiveBox.hxx>
#include <Select3D_SensitiveCircle.hxx>
#include <Select3D_SensitiveCurve.hxx>
#include <Select3D_SensitiveFace.hxx>
#include <Select3D_SensitivePoint.hxx>
#include <Select3D_SensitiveSegment.hxx>
#include <Select3D_SensitiveSet.hxx>
#include <Select3D_SensitiveTriangle.hxx>
#include <Select3D_InteriorSensitivePointSet.hxx> // child of Select3D_SensitiveSet
#include <Select3D_SensitiveGroup.hxx>
#include <Select3D_SensitivePoly.hxx>
#include <Select3D_SensitivePrimitiveArray.hxx>
#include <Select3D_SensitiveTriangulation.hxx>
#include <Select3D_SensitiveWire.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_Selection.hxx>
#include <SelectMgr_SensitiveEntity.hxx>
#include <Standard_Version.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>

#include <inspector/VInspector_ItemContext.hxx>
#include <inspector/VInspector_ItemSelectBasicsEntityOwner.hxx>
#include <inspector/VInspector_ItemSelectMgrSensitiveEntity.hxx>
#include <inspector/VInspector_Tools.hxx>
#include <inspector/ViewControl_Table.hxx>
#include <inspector/ViewControl_Tools.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QStringList>
#include <QColor>
#include <Standard_WarningsRestore.hxx>

// =======================================================================
// function : GetSensitiveEntity
// purpose :
// =======================================================================
Handle(SelectBasics_SensitiveEntity) VInspector_ItemSelectBasicsSensitiveEntity::GetSensitiveEntity() const
{
  initItem();
  return myEntity;
}

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
int VInspector_ItemSelectBasicsSensitiveEntity::initRowCount() const
{
  return 0;
}

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
QVariant VInspector_ItemSelectBasicsSensitiveEntity::initValue (int theItemRole) const
{
  Handle(SelectBasics_SensitiveEntity) anEntity = GetSensitiveEntity();
  Handle(SelectBasics_EntityOwner) anOwner = anEntity->OwnerId();

  switch (theItemRole)
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    {
      switch (Column())
      {
        case 0: return anEntity->DynamicType()->Name();
        default:
          break;
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
            return QVariant ((theItemRole == Qt::BackgroundRole) ? QColor (Qt::darkBlue) : QColor (Qt::white));
        }
      }
      VInspector_ItemSelectMgrSensitiveEntityPtr aParentItem = itemDynamicCast<VInspector_ItemSelectMgrSensitiveEntity>(Parent());
      if (aParentItem)
        return aParentItem->data(QModelIndex(), theItemRole);
      break;
    }
    default:
    break;
  }
  return QVariant();
}

// =======================================================================
// function : createChild
// purpose :
// =======================================================================
TreeModel_ItemBasePtr VInspector_ItemSelectBasicsSensitiveEntity::createChild (int theRow, int theColumn)
{
  return VInspector_ItemSelectBasicsEntityOwner::CreateItem (currentItem(), theRow, theColumn);
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void VInspector_ItemSelectBasicsSensitiveEntity::Init()
{
  VInspector_ItemSelectMgrSensitiveEntityPtr aParentItem = itemDynamicCast<VInspector_ItemSelectMgrSensitiveEntity>(Parent());

  Handle(SelectMgr_SensitiveEntity) anEntity = aParentItem->GetSensitiveEntity();

  myEntity = anEntity->BaseSensitive();
  myPresentationShape = buildPresentationShape (myEntity);

  TreeModel_ItemBase::Init();
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================
void VInspector_ItemSelectBasicsSensitiveEntity::Reset()
{
  // an empty method to don't clear the main label, otherwise the model will be empty
  TreeModel_ItemBase::Reset();
  myEntity = NULL;
  //if (!myPresentation.IsNull() && !myPresentation->GetContext().IsNull())
  //{
  //  myPresentation->GetContext()->Erase (myPresentation, Standard_False);
  //  myPresentation = NULL;
  //}
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================
void VInspector_ItemSelectBasicsSensitiveEntity::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<VInspector_ItemSelectBasicsSensitiveEntity*>(this)->Init();
}

// =======================================================================
// function : getEntityOwner
// purpose :
// =======================================================================
Handle(SelectBasics_EntityOwner) VInspector_ItemSelectBasicsSensitiveEntity::getEntityOwner() const
{
  initItem();

  Handle(SelectBasics_EntityOwner) anOwner;
  const Handle(SelectBasics_SensitiveEntity)& aBase = GetSensitiveEntity();
  if (aBase.IsNull())
    return anOwner;
  return aBase->OwnerId();
}

// =======================================================================
// function : GetPresentations
// purpose :
// =======================================================================
void VInspector_ItemSelectBasicsSensitiveEntity::GetPresentations(NCollection_List<Handle(Standard_Transient)>& thePresentations)
{
  if (Column() != 0)
    return;

  if (!myPresentation.IsNull())
    return;

  Handle(Select3D_SensitiveEntity) aBaseEntity = Handle(Select3D_SensitiveEntity)::DownCast (GetSensitiveEntity());
  if (aBaseEntity.IsNull())
    return;

  myPresentation = new AIS_Shape (myPresentationShape);
  myPresentation->SetColor (Quantity_Color (Quantity_NOC_BLUE1));
  thePresentations.Append (myPresentation);
}

// =======================================================================
// function : GetTableRowCount
// purpose :
// =======================================================================
int VInspector_ItemSelectBasicsSensitiveEntity::GetTableRowCount() const
{
  return 40;
}

// =======================================================================
// function : GetTableEditType
// purpose :
// =======================================================================
ViewControl_EditType VInspector_ItemSelectBasicsSensitiveEntity::GetTableEditType(const int theRow, const int) const
{
  switch (theRow)
  {
    case 3: return ViewControl_EditType_Spin;
    default: return ViewControl_EditType_None;
  }
}

// =======================================================================
// function : GetTableData
// purpose :
// =======================================================================
QVariant VInspector_ItemSelectBasicsSensitiveEntity::GetTableData (const int theRow, const int theColumn, const int theRole) const
{
  if (theRole != Qt::DisplayRole)
    return QVariant();

  bool isFirstColumn = theColumn == 0;

  Handle(SelectBasics_SensitiveEntity) anEntity = GetSensitiveEntity();
  switch (theRow)
  {
    case 0: return ViewControl_Table::SeparatorData();
    case 1: return isFirstColumn ? QVariant(STANDARD_TYPE(SelectBasics_SensitiveEntity)->Name())
                                 : ViewControl_Tools::GetPointerInfo (anEntity).ToCString();
    case 2: return ViewControl_Table::SeparatorData();

    case 3: return isFirstColumn ? QVariant ("SensitivityFactor") : QVariant(anEntity->SensitivityFactor());
    case 4: return isFirstColumn ? QVariant ("NbSubElements") : QVariant (anEntity->NbSubElements());
    case 5: return isFirstColumn ? QVariant ("BoundingBox") : QVariant (VInspector_Tools::ToVariant (anEntity->BoundingBox()));

    case 6: return isFirstColumn ? QVariant ("HasInitLocation") : QVariant (anEntity->HasInitLocation());
    case 7: return isFirstColumn ? QVariant ("InvInitLocation")
                                 : QVariant (ViewControl_Tools::ToString (anEntity->InvInitLocation().Trsf()).ToCString());

    case 8: return ViewControl_Table::SeparatorData();
    case 9: return isFirstColumn ? QVariant (anEntity->DynamicType()->Name())
                                 : ViewControl_Tools::GetPointerInfo (anEntity).ToCString();
    case 10: return ViewControl_Table::SeparatorData();
    default: return getTableData (theRow, theColumn, theRole, anEntity->DynamicType()->Name());
  }
  return QVariant();
}

// =======================================================================
// function : SetTableData
// purpose :
// =======================================================================
bool VInspector_ItemSelectBasicsSensitiveEntity::SetTableData(const int theRow, const int, const QVariant& theValue)
{
  Handle(SelectBasics_SensitiveEntity) anEntity = GetSensitiveEntity();
  switch (theRow)
  {
    case 3: anEntity->SetSensitivityFactor(theValue.toInt());
    //case 5: aPrs->SetCurrentFacingModel (Aspect::TypeOfFacingModelFromString (theValue.toString().toStdString().c_str()));
    //case 6: aPrs->SetInfiniteState (theValue.toBool());
    //case 12: aPrs->SetAutoHilight(theValue.toBool());
    //case 17: aPrs->SetTypeOfPresentation (PrsMgr::TypeOfPresentation3dFromString (theValue.toString().toStdString().c_str()));
    //case 18: aPrs->SetMutable (theValue.toBool());
    default: return false;
  }
  return true;
}

// =======================================================================
// function : getTableData
// purpose :
// =======================================================================
QVariant VInspector_ItemSelectBasicsSensitiveEntity::getTableData (const int theRow,
                                                                   const int theColumn,
                                                                   const int,
                                                                   const TCollection_AsciiString& theEntityKind) const
{
  Handle(Select3D_SensitiveEntity) aBaseEntity = Handle(Select3D_SensitiveEntity)::DownCast (GetSensitiveEntity());
  if (aBaseEntity.IsNull())
    return QVariant();

  int aRow = theRow - 11;
  bool isFirstColumn = theColumn == 0;

  if (aRow == 0)
    return isFirstColumn ? QVariant ("CenterOfGeometry")
                         : QVariant (ViewControl_Tools::ToString (aBaseEntity->CenterOfGeometry()).ToCString());
  aRow = aRow - 1; // for Select3D_SensitiveEntity common value

  Handle(Select3D_SensitiveSet) aSetEntity = Handle(Select3D_SensitiveSet)::DownCast (aBaseEntity);
  if (!aSetEntity.IsNull())
  {
    if (aRow == 0)
    {
      return isFirstColumn ? QVariant ("Size") : QVariant (aSetEntity->Size());
    }
    aRow = aRow - 1; // for Select3D_SensitiveSet common value
  }
  //if (theEntityKind == STANDARD_TYPE (Select3D_SensitiveBox)->Name())
  //{
    //Handle(Select3D_SensitiveBox) anEntity = Handle(Select3D_SensitiveBox)::DownCast (aBaseEntity);
  //}
  if (theEntityKind == STANDARD_TYPE (Select3D_SensitiveCircle)->Name())
  {
    Handle(Select3D_SensitiveCircle) anEntity = Handle(Select3D_SensitiveCircle)::DownCast (aBaseEntity);
    //if (aRow == 0)
  }
  if (theEntityKind == STANDARD_TYPE (Select3D_SensitiveCurve)->Name())
  {
    Handle(Select3D_SensitiveCurve) anEntity = Handle(Select3D_SensitiveCurve)::DownCast (aBaseEntity);
    //if (aRow == 0)
  }
  if (theEntityKind == STANDARD_TYPE (Select3D_SensitiveFace)->Name())
  {
    Handle(Select3D_SensitiveFace) anEntity = Handle(Select3D_SensitiveFace)::DownCast (aBaseEntity);
    if (aRow == 0)
    {
      if (isFirstColumn)
        return "GetPoints";
      else
      {
        Handle(TColgp_HArray1OfPnt) aPoints;
        anEntity->GetPoints (aPoints);
        return ViewControl_Tools::ToString (aPoints).ToCString();
      }
    }
  }
  else if (theEntityKind == STANDARD_TYPE (Select3D_SensitivePoint)->Name())
  {
    Handle(Select3D_SensitivePoint) anEntity = Handle(Select3D_SensitivePoint)::DownCast (aBaseEntity);
    if (aRow == 0)
      return isFirstColumn ? QVariant ("Point") : QVariant (ViewControl_Tools::ToString (anEntity->Point()).ToCString());
  }
  else if (theEntityKind == STANDARD_TYPE (Select3D_SensitiveSegment)->Name())
  {
    Handle(Select3D_SensitiveSegment) anEntity = Handle(Select3D_SensitiveSegment)::DownCast (aBaseEntity);
    switch (aRow)
    {
      case 0: return isFirstColumn ? QVariant ("StartPoint") : QVariant (ViewControl_Tools::ToString (anEntity->StartPoint()).ToCString());
      case 1: return isFirstColumn ? QVariant ("EndPoint") : QVariant (ViewControl_Tools::ToString (anEntity->EndPoint()).ToCString());
      default: break;
    }
  }
  //Select3D_SensitiveSet
  else if (theEntityKind == STANDARD_TYPE (Select3D_SensitiveTriangle)->Name())
  {
    Handle(Select3D_SensitiveTriangle) anEntity = Handle(Select3D_SensitiveTriangle)::DownCast (aBaseEntity);
    gp_Pnt aPnt0, aPnt1, aPnt2;
    anEntity->Points3D (aPnt0, aPnt1, aPnt2);
    switch (aRow)
    {
      case 0: return isFirstColumn ? QVariant ("Points (0)") : QVariant (ViewControl_Tools::ToString (aPnt0).ToCString());
      case 1: return isFirstColumn ? QVariant ("Points (1)") : QVariant (ViewControl_Tools::ToString (aPnt1).ToCString());
      case 2: return isFirstColumn ? QVariant ("Points (2)") : QVariant (ViewControl_Tools::ToString (aPnt2).ToCString());
      default: break;
    }
  }
  //Select3D_InteriorSensitivePointSet
  //Select3D_SensitiveGroup
  else if (theEntityKind == STANDARD_TYPE (Select3D_SensitivePoly)->Name() ||
           theEntityKind == STANDARD_TYPE (Select3D_SensitiveCircle)->Name() ||
           theEntityKind == STANDARD_TYPE (Select3D_SensitiveCurve)->Name())
  {
    if (aRow == 0)
    {
      if (isFirstColumn)
        return "GetPoints";
      else
      {
        Handle(Select3D_SensitivePoly) anEntity = Handle(Select3D_SensitivePoly)::DownCast (aBaseEntity);
        Handle(TColgp_HArray1OfPnt) aPoints;
        anEntity->Points3D (aPoints);
        return ViewControl_Tools::ToString (aPoints).ToCString();
      }
    }
  }
  //Select3D_SensitivePrimitiveArray
  else if (theEntityKind == STANDARD_TYPE (Select3D_SensitiveTriangulation)->Name())
  {
    Handle(Select3D_SensitiveTriangulation) anEntity = Handle(Select3D_SensitiveTriangulation)::DownCast (aBaseEntity);
    if (aRow == 0)
      return isFirstColumn ? QVariant ("Triangulation") : QVariant (ViewControl_Tools::GetPointerInfo (anEntity->Triangulation()).ToCString());
  }
  //Select3D_SensitiveWire
  return QVariant();
}

// =======================================================================
// function : buildPresentationShape
// purpose :
// =======================================================================
TopoDS_Shape VInspector_ItemSelectBasicsSensitiveEntity::buildPresentationShape
  (const Handle(SelectBasics_SensitiveEntity)& theEntity)
{
  Handle(Select3D_SensitiveEntity) aBaseEntity = Handle(Select3D_SensitiveEntity)::DownCast (theEntity);
  if (aBaseEntity.IsNull())
    return TopoDS_Shape();

  BRep_Builder aBuilder;
  TopoDS_Compound aCompound;
  aBuilder.MakeCompound (aCompound);

  aBuilder.Add (aCompound, BRepBuilderAPI_MakeVertex(aBaseEntity->CenterOfGeometry()));

  Standard_CString aTypeName = aBaseEntity->DynamicType()->Name();
  if (aTypeName == STANDARD_TYPE (Select3D_SensitiveBox)->Name())
  {
    Handle(Select3D_SensitiveBox) anEntity = Handle(Select3D_SensitiveBox)::DownCast (aBaseEntity);
    TopoDS_Shape aShape = VInspector_Tools::CreateShape(anEntity->Box());
    aBuilder.Add (aCompound, aShape);
  }
  else if (aTypeName == STANDARD_TYPE (Select3D_SensitiveFace)->Name())
  {
    Handle(Select3D_SensitiveFace) anEntity = Handle(Select3D_SensitiveFace)::DownCast (aBaseEntity);
    Handle(TColgp_HArray1OfPnt) aPoints;
    anEntity->GetPoints (aPoints);
    for (Standard_Integer aPntIter = aPoints->Lower(); aPntIter <= aPoints->Upper(); ++aPntIter)
      aBuilder.Add (aCompound, BRepBuilderAPI_MakeVertex(aPoints->Value (aPntIter)));
  }
  else if (aTypeName == STANDARD_TYPE (Select3D_SensitivePoint)->Name())
  {
    Handle(Select3D_SensitivePoint) anEntity = Handle(Select3D_SensitivePoint)::DownCast (aBaseEntity);
    aBuilder.Add (aCompound, BRepBuilderAPI_MakeVertex(anEntity->Point()));
  }
  else if (aTypeName == STANDARD_TYPE (Select3D_SensitiveSegment)->Name())
  {
    Handle(Select3D_SensitiveSegment) anEntity = Handle(Select3D_SensitiveSegment)::DownCast (aBaseEntity);
    aBuilder.Add (aCompound, BRepBuilderAPI_MakeEdge(anEntity->StartPoint(), anEntity->EndPoint()));
  }
  else if (aTypeName == STANDARD_TYPE (Select3D_SensitiveTriangle)->Name())
  {
    Handle(Select3D_SensitiveTriangle) anEntity = Handle(Select3D_SensitiveTriangle)::DownCast (aBaseEntity);
    gp_Pnt aPnt0, aPnt1, aPnt2;
    anEntity->Points3D (aPnt0, aPnt1, aPnt2);
    aBuilder.Add (aCompound, BRepBuilderAPI_MakeVertex(aPnt0));
    aBuilder.Add (aCompound, BRepBuilderAPI_MakeVertex(aPnt1));
    aBuilder.Add (aCompound, BRepBuilderAPI_MakeVertex(aPnt2));
  }
  else if (aTypeName == STANDARD_TYPE (Select3D_SensitivePoly)->Name() ||
           aTypeName == STANDARD_TYPE (Select3D_SensitiveCircle)->Name() ||
           aTypeName == STANDARD_TYPE (Select3D_SensitiveCurve)->Name())
  {
    Handle(Select3D_SensitivePoly) anEntity = Handle(Select3D_SensitivePoly)::DownCast (aBaseEntity);

    Handle(TColgp_HArray1OfPnt) aPoints;
    anEntity->Points3D (aPoints);
    for (Standard_Integer aPntIter = aPoints->Lower(); aPntIter <= aPoints->Upper(); ++aPntIter)
      aBuilder.Add (aCompound, BRepBuilderAPI_MakeVertex(aPoints->Value (aPntIter)));
  }
  else if (aTypeName == STANDARD_TYPE (Select3D_SensitiveTriangulation)->Name())
  {
    Handle(Select3D_SensitiveTriangulation) anEntity = Handle(Select3D_SensitiveTriangulation)::DownCast (aBaseEntity);
    const Handle(Poly_Triangulation)& aPolyTriangulation = anEntity->Triangulation();
    if (!aPolyTriangulation.IsNull())
    {
      TopoDS_Face aFace;
      aBuilder.MakeFace (aFace, aPolyTriangulation);
      aBuilder.Add (aCompound, aFace);
    }
  }

  return aCompound;
}
