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


#include <inspector/MessageModel_TableModelRealVec3Values.hxx>

#include <inspector/ViewControl_TableModel.hxx>

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================

MessageModel_TableModelRealVec3Values::MessageModel_TableModelRealVec3Values (const Handle(Message_Attribute)& theAttribute,
  const int theSectionWidth)
 : myAttribute (Handle(Message_AttributeVectorOfRealVec3)::DownCast (theAttribute))
{
  SetUseTableProperties (true);

  SetDefaultSectionSize (Qt::Horizontal, theSectionWidth);
}

// =======================================================================
// function : ColumnCount
// purpose :
// =======================================================================

int MessageModel_TableModelRealVec3Values::ColumnCount (const QModelIndex&) const
{
  return myAttribute->GetValues().Length();
}

// =======================================================================
// function : Data
// purpose :
// =======================================================================

QVariant MessageModel_TableModelRealVec3Values::Data (const int, const int theColumn, int theRole) const
{
  if (theRole != Qt::DisplayRole || theColumn > myAttribute->GetValues().Length())
    return QVariant();

  NCollection_Vec3<double> aValue = myAttribute->GetValues().Value(theColumn);
  return QString ("%1, %2, %3").arg (aValue.x()).arg (aValue.y()).arg (aValue.z());
}

// =======================================================================
// function : GetRangeValues
// purpose :
// =======================================================================

void MessageModel_TableModelRealVec3Values::GetRangeValues (QString& theMinValue, QString& theMaxValue,
  const QModelIndexList& theSelected) const
{
  theMinValue = DBL_MAX;
  theMaxValue = DBL_MIN;

  QList<int> aValuePositions;
  if (!theSelected.isEmpty())
  {
    ViewControl_TableModel* aModel = (ViewControl_TableModel*)theSelected[0].model();
    int aRow, aColumn;
    for (QModelIndexList::const_iterator aSelIt = theSelected.begin(); aSelIt != theSelected.end(); aSelIt++)
    {
      aModel->GetSourcePosition (*aSelIt, aRow, aColumn);
      aValuePositions.append (aColumn);
    }
  }

  NCollection_Vector<NCollection_Vec3<double>> aValues = myAttribute->GetValues();
  int aValuesSize = aValues.Length();
  if (aValuesSize < 1)
    return;

  double aXMinValue = aValues.First().x(), aXMaxValue = aValues.First().x();
  NCollection_Vec3<double> aValue;
  for (int i = 1; i < aValuesSize; i++)
  {
    if (!aValuePositions.empty() && !aValuePositions.contains (i))
      continue;

    aValue = aValues.Value (i);
    if (aValue.x() < aXMinValue) aXMinValue = aValue.x();
    if (aValue.x() > aXMaxValue) aXMaxValue = aValue.x();
  }

  double anYMinValue = aValues.First().y(), anYMaxValue = aValues.First().y();
  for (int i = 1; i < aValuesSize; i++)
  {
    if (!aValuePositions.empty() && !aValuePositions.contains (i))
      continue;

    aValue = aValues.Value (i);
    if (aValue.x() == aXMinValue && aValue.y() < anYMinValue) anYMinValue = aValue.y();
    if (aValue.x() == aXMaxValue && aValue.y() > anYMaxValue) anYMaxValue = aValue.y();
  }

  double aZMinValue = aValues.First().z(), aZMaxValue = aValues.First().z();
  for (int i = 1; i < aValuesSize; i++)
  {
    if (!aValuePositions.empty() && !aValuePositions.contains (i))
      continue;
    aValue = aValues.Value (i);
    if (aValue.x() == aXMinValue && aValue.y() == anYMinValue && aValue.z() < aZMinValue) aZMinValue = aValue.z();
    if (aValue.x() == aXMaxValue && aValue.y() == anYMaxValue && aValue.z() > aZMaxValue) aZMaxValue = aValue.z();
  }

  theMinValue = QString ("%1, %2, %3").arg (aXMinValue).arg (anYMinValue).arg (aZMinValue);
  theMaxValue = QString ("%1, %2, %3").arg (aXMaxValue).arg (anYMaxValue).arg (aZMaxValue);
}
