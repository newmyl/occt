// Created on: 2018-09-27
// Created by: Alexander SOLOVYOV
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

#include <QANCollection.hxx>
#include <Draw_Interpretor.hxx>
#include <TColStd_BitField.hxx>
#define PERF_ENABLE_METERS
#include <OSD_PerfMeter.hxx>
#include <TColStd_HPackedMapOfInteger.hxx>
#include <set>

static void printAllMeters(Draw_Interpretor& theDI)
{
  char buffer[25600];
  perf_sprint_all_meters(buffer, 25600 - 1, 1);
  theDI << buffer;
}


//=======================================================================
//function : QANBitField
//purpose  :
//=======================================================================
static Standard_Integer QANBitField(Draw_Interpretor& theDI, Standard_Integer, const char**)
{
#ifdef NDEBUG
  const Standard_Integer NB = 12000000;
#else
  const Standard_Integer NB = 1000000;
#endif

  TColStd_BitField aField;
  aField.Reserve(NB);
  PERF_START_METER("Bit field fill 12 mlns");
  for (Standard_Integer i = 0; i < NB; i++)
  {
    aField.Add(i);
  }
  PERF_STOP_METER("Bit field fill 12 mlns");

  /*std::vector<bool> aVector;
  aVector.resize(NB);
  PERF_START_METER("Vector<bool> 12 mlns");
  for (Standard_Integer i = 0; i < NB; i++)
  {
    aVector[i] = true;
  }
  PERF_STOP_METER("Vector<bool> 12 mlns");*/

  /*std::set<int> aSet;
  PERF_START_METER("set<int> 12 mlns");
  for (Standard_Integer i = 0; i < NB; i++)
  {
    aSet.insert(i);
  }
  PERF_STOP_METER("set<int> 12 mlns");*/

  Handle(TColStd_HPackedMapOfInteger) aMap = new TColStd_HPackedMapOfInteger();
  PERF_START_METER("Packed map fill 12 mlns");
  for (Standard_Integer j = 0; j < NB; j++)
  {
    aMap->ChangeMap().Add(j);
  }
  PERF_STOP_METER("Packed map fill 12 mlns");


  TColStd_BitField aField1, aField2;
  aField1.Reserve(NB);
  aField2.Reserve(NB);
  for (Standard_Integer i = 0; i < NB; i++)
  {
    if (i % 2 == 0)
      aField1.Add(i);
    else
      aField2.Add(i);
  }
  PERF_START_METER("Bit field unite 12 mlns");
  aField1.Unite(aField2);
  PERF_STOP_METER("Bit field unite 12 mlns");

  /*std::vector<bool> aVector1, aVector2;
  aVector1.resize(NB);
  aVector2.resize(NB);
  for (Standard_Integer i = 0; i < NB; i++)
  {
    if (i % 2 == 0)
      aVector1[i] = true;
    else
      aVector2[i] = true;
  }
  PERF_START_METER("Vector unite 12 mlns");
  for (Standard_Integer i = 0; i < NB; i++)
  {
    aVector1[i] = aVector1[i] || aVector2[i];
  }
  PERF_STOP_METER("Vector unite 12 mlns");

  TColStd_BitField aField3;
  aField3.Reserve(NB);
  PERF_START_METER("Bit field access 12 mlns");
  int c = 0;
  for (Standard_Integer i = 0; i < NB; i++)
  {
    if (aField3.Has(i))
      c++;
  }
  PERF_STOP_METER("Bit field access 12 mlns");

  std::vector<bool> aVector3;
  aVector3.resize(NB);
  PERF_START_METER("Vector<bool> access 12 mlns");
  for (Standard_Integer i = 0; i < NB; i++)
  {
    if (aVector3[i])
      c++;
  }
  PERF_STOP_METER("Vector<bool> access 12 mlns");*/




  printAllMeters(theDI);

  return 0;
}

//=======================================================================
//function : CommandsBitField
//purpose  :
//=======================================================================
void QANCollection::CommandsBitField(Draw_Interpretor& theCommands)
{
  const char* aGroup = "QANCollection";

  theCommands.Add("QANBitField",
    "QANBitField",
    __FILE__,
    QANBitField,
    aGroup);
}

