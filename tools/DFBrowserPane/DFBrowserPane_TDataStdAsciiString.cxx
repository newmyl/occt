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

#include <inspector/DFBrowserPane_TDataStdAsciiString.hxx>

#include <TDataStd_AsciiString.hxx>

#include <QVariant>
#include <QIcon>

// =======================================================================
// function : GetValues
// purpose :
// =======================================================================
void DFBrowserPane_TDataStdAsciiString::GetValues (const Handle(TDF_Attribute)& theAttribute,
                                                   QList<QVariant>& theValues)
{
  Handle(TDataStd_AsciiString) anAttribute = Handle(TDataStd_AsciiString)::DownCast (theAttribute);
  if (anAttribute.IsNull())
    return;

  theValues.append ("Get");
  theValues.append (anAttribute->Get().ToCString());
}
