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

#ifndef _Graphic3d_AttribBuffer_HeaderFile
#define _Graphic3d_AttribBuffer_HeaderFile

#include <Graphic3d_Buffer.hxx>

//! Buffer of vertex attributes.
class Graphic3d_AttribBuffer : public Graphic3d_Buffer
{
public:
  //! Empty constructor.
  Standard_EXPORT Graphic3d_AttribBuffer(const Handle(NCollection_BaseAllocator)& theAlloc,
    Standard_Boolean isInterleaved, Standard_Boolean isMutable);

  Standard_EXPORT Standard_Boolean Invalidate(Standard_Integer theAttributeIndex);
  Standard_EXPORT Standard_Boolean Invalidate(Standard_Integer theAttributeIndex,
                                              Standard_Integer theStartVertexIndex,
                                              Standard_Integer theEndVertexIndex);
  Standard_EXPORT Standard_Boolean Invalidate(Standard_Integer theStartVertexIndex,
                                              Standard_Integer theEndVertexIndex);

public:
  DEFINE_STANDARD_RTTI_INLINE(Graphic3d_AttribBuffer, Graphic3d_Buffer) // Type definition
};

DEFINE_STANDARD_HANDLE(Graphic3d_AttribBuffer, Graphic3d_Buffer)

#endif // _Graphic3d_AttribBuffer_HeaderFile
