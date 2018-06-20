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
  : myProgress(0L), myParent(0L),
    myStart(0.), myEnd(1.),
    myName(theName), myNbChild(0)
{
  SetScale(theMin, theMax, theStep, isInfinite);
  // set progress after setting scale to avoid calling virtual methods
  myProgress = theProgress;
}

//=======================================================================
//function : Message_ProgressScope
//purpose  : 
//=======================================================================

Message_ProgressScope::Message_ProgressScope(Message_ProgressScope* theParent,
                                             Standard_CString theName,
                                             Standard_Real theMin, Standard_Real theMax,
                                             Standard_Real theStep, Standard_Boolean isInfinite)
  : myProgress(theParent ? theParent->myProgress : 0L), myParent(theParent),
    myName(theName), myNbChild(0)
{
  if (myProgress)  // disable sub-scoping if no progress is given
    myParent->addScope(this);
  SetScale(theMin, theMax, theStep, isInfinite);
}

#ifndef OCCT_NO_RVALUE_REFERENCE
//=======================================================================
//function : Message_ProgressScope
//purpose  : 
//=======================================================================

Message_ProgressScope::Message_ProgressScope(Message_ProgressScope&& theOther)
  : myProgress(theOther.myProgress), myParent(theOther.myParent),
    myStart(theOther.myStart), myEnd(theOther.myEnd),
    myMax(theOther.myMax), myStep(theOther.myStep),
    myPosition(theOther.myPosition), myPosBySS(theOther.myPosBySS),
    myName(theOther.myName), myNbChild(theOther.myNbChild),
    myIsInfinite(theOther.myIsInfinite)
{
  if (myParent)
  {
    theOther.myParent = 0L;
    theOther.myProgress = 0L;
  }
}

//=======================================================================
//function : operator=
//purpose  : 
//=======================================================================

Message_ProgressScope& Message_ProgressScope::operator=(Message_ProgressScope&& theOther)
{
  myProgress = theOther.myProgress;
  myParent = theOther.myParent;
  myStart = theOther.myStart;
  myEnd = theOther.myEnd;
  myMax = theOther.myMax;
  myStep = theOther.myStep;
  myPosition = theOther.myPosition;
  myPosBySS = theOther.myPosBySS;
  myName = theOther.myName;
  myNbChild = theOther.myNbChild;
  myIsInfinite = theOther.myIsInfinite;
  if (myParent)
  {
    theOther.myParent = 0L;
    theOther.myProgress = 0L;
  }
  return *this;
}
#endif

//=======================================================================
//function : SetScale
//purpose  : 
//=======================================================================

void Message_ProgressScope::SetScale(double theMin, double theMax, double theStep, bool isInfinite)
{
  myMax = theMax - theMin;
  myStep = theStep;
  myIsInfinite = isInfinite;
  myPosition = myPosBySS = 0.;
  if (myProgress)
    // show progress for the first time
    myProgress->Increment(0, this);
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
  Standard_Real aCurr = localToGlobal(Max(myPosition, myPosBySS), myIsInfinite);
  myPosition = myMax;
  Standard_Real aDelta = myEnd - aCurr;
  if (aDelta > 0.)
    myProgress->Increment(aDelta, this);

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
    Standard_Real aCurr = localToGlobal(Max(myPosition, myPosBySS), myIsInfinite);
    Standard_Real aNext = localToGlobal(aValue, myIsInfinite);
    myPosition = aValue;
    Standard_Real aDelta = aNext - aCurr;
    if (aDelta > 0.)
      myProgress->Increment(aDelta, this);
  }
}

//=======================================================================
//function : addScope
//purpose  : 
//=======================================================================

void Message_ProgressScope::addScope(Message_ProgressScope* theChild)
{
  // reserve space on the scale for the new child
  Standard_Real aPrevPos = myPosBySS;
  myPosBySS = Min(myPosBySS + myStep, myMax);

  // compute child range on total progress scale
  theChild->myStart = localToGlobal(aPrevPos, Standard_False);
  theChild->myEnd = localToGlobal(myPosBySS, Standard_False);

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
  return myStart + (myEnd - myStart) * x / (1. + x);          // hyperbola
}
