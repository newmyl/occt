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
#include <Standard_ProgramError.hxx>

//=======================================================================
//function : Message_ProgressScope
//purpose  : 
//=======================================================================

Message_ProgressScope::Message_ProgressScope(Message_ProgressIndicator* theProgress,
                                             Standard_CString theName,
                                             Standard_Real theMin, Standard_Real theMax,
                                             Standard_Real theStep, Standard_Boolean isInfinite)
  : myProgress(theProgress), myParent(0),
    myStart(0.), myEnd(1.),
    myMax(theMax), myStep(theStep), myPosition(theMin),
    myName(theName), myNbChild(0),
    myIsInfinite(isInfinite)
{}

//=======================================================================
//function : Message_ProgressScope
//purpose  : 
//=======================================================================

Message_ProgressScope::Message_ProgressScope(Message_ProgressScope* theParent,
                                             Standard_CString theName,
                                             Standard_Real theMin, Standard_Real theMax,
                                             Standard_Real theStep, Standard_Boolean isInfinite)
  : myProgress(theParent ? theParent->myProgress : 0L), myParent(theParent),
    myMax(theMax), myStep(theStep), myPosition(theMin),
    myName(theName), myNbChild(0),
    myIsInfinite(isInfinite)
{
  if (myProgress)
    myParent->addScope(this);
}

//=======================================================================
//function : SetScale
//purpose  : 
//=======================================================================

void Message_ProgressScope::SetScale(double theMin, double theMax, double theStep, bool isInfinite)
{
  myMax = theMax - theMin;
  myStep = theStep;
  myIsInfinite = isInfinite;
  myPosition = 0.;
}

//=======================================================================
//function : Close
//purpose  : 
//=======================================================================

void Message_ProgressScope::Close()
{
  if (!myProgress)
    return;

  // advance to the end of the scope
  Standard_Real aCurr = localToGlobal(myPosition, myIsInfinite);
  myProgress->Increment(myEnd - aCurr, this);

  if (myParent)
    myParent->removeScope();
  myProgress = 0L;

  Standard_ProgramError_Raise_if(myNbChild,
    "Some child sub-scopes remained while parent is being closed");
}

//=======================================================================
//function : UserBreak
//purpose  : 
//=======================================================================

Standard_Boolean Message_ProgressScope::UserBreak() const
{
  return myProgress && myProgress->UserBreak();
}

//=======================================================================
//function : Next
//purpose  : 
//=======================================================================

void Message_ProgressScope::Next(Standard_Real theStep)
{
  if (!myProgress)
    return;

  if (theStep > 0.)
  {
    Standard_Real aValue = Min(myPosition + theStep, myMax);
    Standard_Real aCurr = localToGlobal(myPosition, myIsInfinite);
    Standard_Real aNext = localToGlobal(aValue, myIsInfinite);
    myProgress->Increment(aNext - aCurr, this);
    myPosition = aValue;
  }
}

//=======================================================================
//function : addScope
//purpose  : 
//=======================================================================

void Message_ProgressScope::addScope(Message_ProgressScope* theChild)
{
  // reserve space on the scale for the new child
  Standard_Real aPrevPos = myPosition;
  myPosition = Min(myPosition + myStep, myMax);

  // compute child range on total progress scale
  theChild->myStart = localToGlobal(aPrevPos, Standard_False);
  theChild->myEnd = localToGlobal(myPosition, Standard_False);

  // increment counter of children
  myNbChild++;
}

//=======================================================================
//function : SetName
//purpose  : 
//=======================================================================

void Message_ProgressScope::SetName(Standard_CString theName)
{
  myName = theName;
  if (myProgress)
    myProgress->Increment(0, this); // just to redisplay
}

//=======================================================================
//function : localToGlobal
//purpose  : 
//=======================================================================

Standard_Real Message_ProgressScope::localToGlobal(const Standard_Real theVal,
                                                   const Standard_Boolean isInf) const
{
  if (theVal <= 0.)
    return myStart;
  if (myMax - theVal < RealSmall())
    return myEnd;

  if (!isInf)
    return myStart + (myEnd - myStart) * theVal / myMax;

  double x = theVal / myMax;
  //      return myStart + ( myEnd - myStart ) * ( 1. - std::exp ( -x ) ); // exponent
  return myStart + (myEnd - myStart) * (1. - 1. / x);          // hyperbola
}
