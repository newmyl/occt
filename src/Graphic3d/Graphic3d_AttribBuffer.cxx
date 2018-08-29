// Copyright (c) 2014 OPEN CASCADE SAS
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

#include <Graphic3d_AttribBuffer.hxx>

Graphic3d_AttribBuffer::Graphic3d_AttribBuffer(const Handle(NCollection_BaseAllocator)& theAlloc,
                                               Standard_Boolean isInterleaved, Standard_Boolean isMutable,
                                               Standard_Integer theMaxNbElements)
  : Graphic3d_Buffer(theAlloc, isInterleaved, isMutable, theMaxNbElements)
{
}

Standard_Boolean Graphic3d_AttribBuffer::Invalidate(int theAttributeIndex)
{
  if (!IsMutable() || IsInterleaved())
    return false;

  Graphic3d_Range aRange;
  aRange.Start = AttributeOffset(theAttributeIndex);
  aRange.Length = AttributeOffset(theAttributeIndex + 1) - aRange.Start;
  Graphic3d_Buffer::Invalidate(aRange);

  return true;
}

Standard_Boolean Graphic3d_AttribBuffer::Invalidate(int theAttributeIndex, int theStartVertexIndex, int theEndVertexIndex)
{
  if (!IsMutable() || IsInterleaved())
    return false;

  Graphic3d_Range aRange;
  Standard_Integer anAttrOffset = AttributeOffset(theAttributeIndex);
  Standard_Integer aStride = AttributesArray()[theAttributeIndex].Stride();
  aRange.Start = anAttrOffset + aStride * theStartVertexIndex;
  aRange.Length = aStride * ( theEndVertexIndex - theStartVertexIndex + 1 );
  Graphic3d_Buffer::Invalidate(aRange);

  return true;
}

Standard_Boolean Graphic3d_AttribBuffer::Invalidate(int theStartVertexIndex, int theEndVertexIndex)
{
  if (!IsMutable())
    return false;

  if (IsInterleaved())
  {
    Graphic3d_Range aRange;
    aRange.Start = Stride * theStartVertexIndex;
    aRange.Length = Stride * (theEndVertexIndex - theStartVertexIndex + 1);
    Graphic3d_Buffer::Invalidate(aRange);
    return true;
  }
  else
  {
    Standard_Boolean isOK = true;
    for (Standard_Integer i = 0, n = NbAttributes; i < n; i++)
    {
      bool isLocalOK = Invalidate(i, theStartVertexIndex, theEndVertexIndex);
      isOK = isOK && isLocalOK;
    }
    return isOK;
  }
}


