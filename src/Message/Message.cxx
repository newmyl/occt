// Created on: 1999-11-23
// Created by: data exchange team
// Copyright (c) 1999-1999 Matra Datavision
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


#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <TCollection_AsciiString.hxx>

#include <stdio.h>
#include <string.h>
//=======================================================================
//function : DefaultMessenger
//purpose  : 
//=======================================================================
const Handle(Message_Messenger)& Message::DefaultMessenger ()
{
  static Handle(Message_Messenger) aMessenger = new Message_Messenger;
  return aMessenger;
}

//=======================================================================
//function : FillTime
//purpose  : 
//=======================================================================

TCollection_AsciiString Message::FillTime (const Standard_Integer hour,
					       const Standard_Integer minute,
					       const Standard_Real second)
{
  char t [30];
  if (hour > 0)
    Sprintf (t, "%02dh:%02dm:%.2fs", hour, minute, second);
  else if (minute > 0)
    Sprintf (t, "%02dm:%.2fs", minute, second);
  else
    Sprintf (t, "%.2fs", second);
  return TCollection_AsciiString (t);
}

namespace
{
  static Standard_CString Message_Table_PrintGravityEnum[5] =
  {
    "TRACE", "INFO", "WARNING", "ALARM", "FAIL"
  };
}

//=======================================================================
//function : GravityToString
//purpose  :
//=======================================================================

Standard_CString Message::GravityToString (const Message_Gravity theGravity)
{
  return Message_Table_PrintGravityEnum[theGravity];
}

//=======================================================================
//function : GravityFromString
//purpose  :
//=======================================================================

Standard_Boolean Message::GravityFromString (const Standard_CString theGravityString,
                                             Message_Gravity& theGravity)
{
  TCollection_AsciiString aName (theGravityString);
  aName.UpperCase();
  for (Standard_Integer aGravityIter = 0; aGravityIter <= Message_Fail; ++aGravityIter)
  {
    Standard_CString aGravityName = Message_Table_PrintGravityEnum[aGravityIter];
    if (aName == aGravityName)
    {
      theGravity = Message_Gravity (aGravityIter);
      return Standard_True;
    }
  }
  return Standard_False;
}
