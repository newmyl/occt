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

#ifndef TColStd_BitField_HeaderFile
#define TColStd_BitField_HeaderFile

#include <vector>

class TColStd_BitField
{
public:
  TColStd_BitField(): myIsEmpty(Standard_True) { }

  inline void Clear()
  {
    myData.clear();
    myIsEmpty = Standard_True;
  }

  inline void Add(const Standard_Integer theIndex)
  {
    myData[theIndex] = true;
    myIsEmpty = Standard_False;
  }

  inline Standard_Boolean Has(const Standard_Integer theIndex) const
  {
    return myData[theIndex];
  }

  inline void Reserve(const Standard_Integer theMax)
  {
    myData.resize(theMax+1, false);
  }

  inline void Unite(const TColStd_BitField& theBitField)
  {
    size_t aSize = myData.size();
    if (theBitField.myData.size() > aSize)
      aSize = theBitField.myData.size();

    myData.resize(aSize);

    for (int i = 0; i < aSize; i++)
      myData[i] = myData[i] || theBitField.myData[i];

    myIsEmpty |= theBitField.myIsEmpty;
  }

  inline Standard_Boolean IsEmpty() { return myIsEmpty; }

private:
  std::vector<bool> myData;
  Standard_Boolean myIsEmpty;
};

#endif
