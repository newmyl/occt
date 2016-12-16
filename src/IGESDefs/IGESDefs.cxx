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


#include <IGESDefs.hxx>
#include <IGESDefs_GeneralModule.hxx>
#include <IGESDefs_Protocol.hxx>
#include <IGESGraph.hxx>
#include <Interface_GeneralLib.hxx>


/*void IGESDefs::Init ()
{
  IGESGraph::Init();
  IGESDefs::Protocol();
}*/

const Handle(IGESDefs_Protocol) & IGESDefs::Protocol ()
{
  static Handle(IGESDefs_Protocol) protocol;
  if (protocol.IsNull()) {
    protocol = new IGESDefs_Protocol;
    Interface_GeneralLib::SetGlobal (new IGESDefs_GeneralModule,  protocol);
  }
  return protocol;
}
