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

#include <inspector/MessageModel_Tools.hxx>
#include <inspector/MessageModel_TableModelRealValues.hxx>
#include <inspector/MessageModel_TableModelRealVec3Values.hxx>

#include <inspector/ViewControl_Table.hxx>
#include <inspector/ViewControl_TableModelValues.hxx>

#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepTools.hxx>
#include <Message_AlertExtended.hxx>
#include <Message_AttributeObject.hxx>
#include <Message_AttributeVectorOfReal.hxx>
#include <Message_AttributeVectorOfRealVec3.hxx>
#include <Precision.hxx>
#include <TopoDS_AlertWithShape.hxx>
#include <TopoDS_Edge.hxx>

// =======================================================================
// function : GetPointerInfo
// purpose :
// =======================================================================
TCollection_AsciiString MessageModel_Tools::GetPointerInfo (const Handle(Standard_Transient)& thePointer, const bool isShortInfo)
{
  if (thePointer.IsNull())
    return TCollection_AsciiString();

  std::ostringstream aPtrStr;
  aPtrStr << thePointer.operator->();
  if (!isShortInfo)
    return aPtrStr.str().c_str();

  TCollection_AsciiString anInfoPtr (aPtrStr.str().c_str());
  for (int aSymbolId = 1; aSymbolId < anInfoPtr.Length(); aSymbolId++)
  {
    if (anInfoPtr.Value(aSymbolId) != '0')
    {
      anInfoPtr = anInfoPtr.SubString (aSymbolId, anInfoPtr.Length());
      anInfoPtr.Prepend("0x");
      return anInfoPtr;
    }
  }
  return aPtrStr.str().c_str();
}

// =======================================================================
// function : GetPointerAlertInfo
// purpose :
// =======================================================================
TCollection_AsciiString MessageModel_Tools::GetPointerAlertInfo (const Handle(Message_Alert)& theAlert)
{
  if (theAlert->IsKind (STANDARD_TYPE (TopoDS_AlertWithShape)))
    return GetPointerInfo (Handle(TopoDS_AlertWithShape)::DownCast (theAlert)->GetShape().TShape());
  else if (theAlert->IsKind (STANDARD_TYPE (Message_AttributeObject)))
    return GetPointerInfo (Handle(Message_AttributeObject)::DownCast (theAlert)->GetObject());
  return TCollection_AsciiString();
}

// =======================================================================
// function : GetShapeTypeAlertInfo
// purpose :
// =======================================================================
TCollection_AsciiString MessageModel_Tools::GetShapeTypeAlertInfo (const Handle(Message_Alert)& theAlert)
{
  TopoDS_Shape aShape;

  if (theAlert->IsKind (STANDARD_TYPE (TopoDS_AlertWithShape)))
    aShape = Handle(TopoDS_AlertWithShape)::DownCast (theAlert)->GetShape();

  if (aShape.IsNull())
    return TCollection_AsciiString();

  TopAbs_ShapeEnum aShapeType = aShape.ShapeType();
  Standard_SStream aSStream;
  TopAbs::Print ((TopAbs_ShapeEnum)aShapeType, aSStream);
  return aSStream.str().c_str();
}

// =======================================================================
// function : GetStandardTypeAlertInfo
// purpose :
// =======================================================================
TCollection_AsciiString MessageModel_Tools::GetStandardTypeAlertInfo (const Handle(Message_Alert)& theAlert)
{
  Handle(Standard_Transient) aPointer;
  if (theAlert->IsKind (STANDARD_TYPE (TopoDS_AlertWithShape)))
    aPointer = Handle(TopoDS_AlertWithShape)::DownCast (theAlert)->GetShape().TShape();
  else if (theAlert->IsKind (STANDARD_TYPE (Message_AttributeObject)))
    aPointer = Handle(Message_AttributeObject)::DownCast (theAlert)->GetObject();

  if (aPointer.IsNull())
    return TCollection_AsciiString();

  return aPointer->DynamicType()->Name();
}

// =======================================================================
// function : GetPropertyTableValues
// purpose :
// =======================================================================
void MessageModel_Tools::GetPropertyTableValues (const Handle(Message_Alert)& theAlert,
                                                 QList<ViewControl_TableModelValues*>& theTableValues)
{
  ViewControl_TableModelValues* aTableValues = 0;

  Handle(Message_AlertExtended) anExtendedAlert = Handle(Message_AlertExtended)::DownCast(theAlert);
  if (anExtendedAlert.IsNull())
    return;

  Handle(Message_Attribute) anAttribute = anExtendedAlert->Attribute();
  if (anAttribute.IsNull())
    return;

  if (anAttribute->IsKind (STANDARD_TYPE (Message_AttributeVectorOfReal)))
  {
    int aSectionSize = 60;
    aTableValues = new MessageModel_TableModelRealValues (anAttribute, aSectionSize);
    theTableValues.append (aTableValues);
  }
  else if (anAttribute->IsKind (STANDARD_TYPE (Message_AttributeVectorOfRealVec3)))
  {
    int aSectionSize = 160;
    aTableValues = new MessageModel_TableModelRealVec3Values (anAttribute, aSectionSize);
    theTableValues.append (aTableValues);
  }
  else
  {
    if (!anAttribute->GetDescription().IsEmpty())
    {
      aTableValues = new ViewControl_TableModelValues();
      QList<TreeModel_HeaderSection> aHeaderValues;
      QVector<QVariant> aValues;
      aHeaderValues << TreeModel_HeaderSection ("Description", -2);
      aValues << anAttribute->GetDescription().ToCString();
      aTableValues->SetHeaderValues (aHeaderValues, Qt::Horizontal);

      QString aValueStr = anAttribute->GetDescription().ToCString();
      QStringList aValueStrList = aValueStr.split ("\n");
      int aNbRows = aValueStrList.size();

      QFontMetrics aFontMetrics (qApp->font());
      int aHeight = aFontMetrics.boundingRect(aValueStr).height();
      aHeight = (aHeight + TreeModel_Tools::HeaderSectionMargin()) * aNbRows;
      aTableValues->SetValues (aValues);
      aTableValues->SetDefaultSectionSize(Qt::Vertical, aHeight);

      theTableValues.append (aTableValues);
    }
  }
}

// =======================================================================
// function : BuildShape
// purpose :
// =======================================================================
TopoDS_Shape MessageModel_Tools::BuildShape (const Handle(Message_Alert)& theAlert, QList<int> theSelectedIndices, ViewControl_Table* theTable)
{
  if (theAlert.IsNull())
    return TopoDS_Shape();

  BRep_Builder aBuilder;
  TopoDS_Compound aCompound;
  aBuilder.MakeCompound(aCompound);

  if (theAlert->IsKind (STANDARD_TYPE (Message_AttributeVectorOfReal)))
  {
    Handle(Message_AttributeVectorOfReal) aValuesAlert = Handle(Message_AttributeVectorOfReal)::DownCast (theAlert);
    NCollection_Vector<double> aValues = aValuesAlert->GetValues();
    int aValuesSize = aValues.Size();

    gp_Pnt aPreviousPoint(0, 0, 0), aCurrentPoint(0, 0, 0);
    double aXStep = theTable->GetProperty()->GetXStep();
    if (aXStep < 0)
      aXStep = 1;
    double aCurrentXValue = 0;
    for (QList<int>::const_iterator anIt = theSelectedIndices.begin(); anIt != theSelectedIndices.end(); anIt++)
    {
      if (*anIt >= aValuesSize)
        continue;

      if (aCurrentXValue == 0)
      { //just define the previous point
        aPreviousPoint.SetX (aCurrentXValue);
        aPreviousPoint.SetY (aValues.Value (*anIt));
        aCurrentXValue = aCurrentXValue + aXStep;
        continue;
      }
      aCurrentPoint.SetX (aCurrentXValue);
      aCurrentPoint.SetY (aValues.Value (*anIt));
      if (aPreviousPoint.Distance (aCurrentPoint) < Precision::Confusion())
        continue;

      TopoDS_Edge anEdge = BRepBuilderAPI_MakeEdge (aPreviousPoint, aCurrentPoint);
      aBuilder.Add (aCompound, anEdge);
      aPreviousPoint = aCurrentPoint;
      aCurrentXValue = aCurrentXValue + aXStep;
    }
  }
  else if (theAlert->IsKind (STANDARD_TYPE (Message_AttributeVectorOfRealVec3)))
  {
    Handle(Message_AttributeVectorOfRealVec3) aValuesAlert = Handle(Message_AttributeVectorOfRealVec3)::DownCast (theAlert);
    NCollection_Vector<NCollection_Vec3<double>> aValues = aValuesAlert->GetValues();
    int aValuesSize = aValues.Size();

    gp_Pnt aCurrentPoint(0, 0, 0);
    NCollection_Vec3<double> aValue;
    for (QList<int>::const_iterator anIt = theSelectedIndices.begin(); anIt != theSelectedIndices.end(); anIt++)
    {
      if (*anIt >= aValuesSize)
        continue;

      aValue = aValues.Value (*anIt);
      aCurrentPoint.SetX (aValue.x());
      aCurrentPoint.SetY (aValue.y());
      aCurrentPoint.SetZ (aValue.z());

      TopoDS_Vertex aVertex = BRepBuilderAPI_MakeVertex (aCurrentPoint);
      aBuilder.Add (aCompound, aVertex);
    }
  }

  return aCompound;
}
