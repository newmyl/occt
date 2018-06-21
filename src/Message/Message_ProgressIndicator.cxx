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


#include <Message_ProgressIndicator.hxx>
#include <Message_ProgressScope.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Message_ProgressIndicator,Standard_Transient)

//=======================================================================
//function : Message_ProgressIndicator
//purpose  : 
//=======================================================================

Message_ProgressIndicator::Message_ProgressIndicator()
  : myPosition(0.),
    myRootScope(new Message_ProgressScope(this, 0L, 0., 1., 1.))
{
}

//=======================================================================
//function : ~Message_ProgressIndicator
//purpose  : 
//=======================================================================

Message_ProgressIndicator::~Message_ProgressIndicator()
{
  // Avoid calling of Increment() from myRootScope->Close()
  myRootScope->myProgress = 0L;
  delete myRootScope;
}

//=======================================================================
//function : Increment
//purpose  : 
//=======================================================================

void Message_ProgressIndicator::Increment(const Standard_Real theStep,
                                          const Message_ProgressScope* theScope)
{
  Standard_Mutex::Sentry aSentry(myMutex);
  myPosition = Min(myPosition + theStep, 1.);
  Show(Standard_False, theScope);
}
