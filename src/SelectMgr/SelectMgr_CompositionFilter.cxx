// Created on: 1996-01-29
// Created by: Robert COUBLANC
// Copyright (c) 1996-1999 Matra Datavision
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


#include <SelectMgr_CompositionFilter.hxx>
#include <SelectMgr_Filter.hxx>
#include <SelectMgr_ListIteratorOfListOfFilter.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(SelectMgr_CompositionFilter,SelectMgr_Filter)

SelectMgr_CompositionFilter::SelectMgr_CompositionFilter()
 : myReport (new Message_Report())
{
  myReport->SetActive (Standard_False);
}

void SelectMgr_CompositionFilter::Add(const Handle(SelectMgr_Filter)& afilter)
{
  myFilters.Append(afilter);
}

void SelectMgr_CompositionFilter::Remove(const Handle(SelectMgr_Filter)& afilter)
{
  SelectMgr_ListIteratorOfListOfFilter It(myFilters);
  for(;It.More();It.Next()){
    if (afilter==It.Value()){ 
      myFilters.Remove(It);
      return;
    }
  }
}


Standard_Boolean SelectMgr_CompositionFilter::IsEmpty() const
{
  return myFilters.IsEmpty();
}

Standard_Boolean SelectMgr_CompositionFilter::IsIn(const Handle(SelectMgr_Filter)& afilter) const
{
  SelectMgr_ListIteratorOfListOfFilter It(myFilters);
  for(;It.More();It.Next())
    if (afilter==It.Value()) 
      return Standard_True;
  return Standard_False;

}

void  SelectMgr_CompositionFilter::Clear()
{
  myFilters.Clear();
}


Standard_Boolean SelectMgr_CompositionFilter::ActsOn(const TopAbs_ShapeEnum aStandardMode) const
{
  SelectMgr_ListIteratorOfListOfFilter It(myFilters);
  for(;It.More();It.Next()){
    if (It.Value()->ActsOn(aStandardMode))
      return Standard_True;
  }
  
  return Standard_False;
}

void SelectMgr_CompositionFilter::SetReportActive (const Standard_Boolean theState)
{
  myReport->SetActive (theState);

  for (SelectMgr_ListIteratorOfListOfFilter aSubFiltersIt (myFilters); aSubFiltersIt.More(); aSubFiltersIt.Next())
  {
    if (aSubFiltersIt.Value()->IsKind (STANDARD_TYPE (SelectMgr_CompositionFilter)))
      Handle(SelectMgr_CompositionFilter)::DownCast (aSubFiltersIt.Value())->SetReportActive (theState);
  }
}

void SelectMgr_CompositionFilter::MergeReport (const Handle(Message_Report)& theReport,
                                               const Handle(Message_Alert)& theParentAlert)
{
  if (!myReport->IsActive (Message_Info))
    return;

  if (myReport->GetLastAlert (Message_Info, true).IsNull())
  {
    myReport->AddAlert (Message_Info, new Message_Alert (DynamicType()->Name()));
  }
  theReport->Merge (myReport, theParentAlert);

  Handle(Message_Alert) aParentAlert = theReport->GetLastAlert (Message_Info, true);
  for (SelectMgr_ListIteratorOfListOfFilter aSubFiltersIt (myFilters); aSubFiltersIt.More(); aSubFiltersIt.Next())
  {
    if (aSubFiltersIt.Value()->IsKind (STANDARD_TYPE (SelectMgr_CompositionFilter)))
      Handle(SelectMgr_CompositionFilter)::DownCast (aSubFiltersIt.Value())->MergeReport (theReport, aParentAlert);
  }
}

void SelectMgr_CompositionFilter::ClearReport()
{
  myReport->Clear();
  for (SelectMgr_ListIteratorOfListOfFilter aSubFiltersIt (myFilters); aSubFiltersIt.More(); aSubFiltersIt.Next())
  {
    if (aSubFiltersIt.Value()->IsKind (STANDARD_TYPE (SelectMgr_CompositionFilter)))
      Handle(SelectMgr_CompositionFilter)::DownCast (aSubFiltersIt.Value())->ClearReport();
  }
}
