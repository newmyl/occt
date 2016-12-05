// Copyright (c) 1999-2014 OPEN CASCADE SAS
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


#include <IGESData_FileProtocol.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESData_FileProtocol,IGESData_Protocol)

void IGESData_FileProtocol::Add (const Handle(IGESData_Protocol)& theProtocol)
{
  NCollection_Vector<Handle(IGESData_Protocol)>::Iterator itr(myResources);
  for (; itr.More(); itr.Next())
  {
    if (itr.Value()->IsInstance(theProtocol->DynamicType())) return; // passer
  }
  myResources.Append(theProtocol);
}

Standard_Integer IGESData_FileProtocol::NbResources () const
{
  return myResources.Length();
}

Handle(Interface_Protocol) IGESData_FileProtocol::Resource (const Standard_Integer num) const
{
  return (num > 0 && num <= myResources.Length())? myResources(num-1) : NULL;
}
