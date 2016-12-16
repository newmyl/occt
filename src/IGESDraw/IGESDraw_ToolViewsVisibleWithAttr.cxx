// Created by: CKY / Contract Toubro-Larsen
// Copyright (c) 1993-1999 Matra Datavision
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

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#include <IGESBasic_HArray1OfLineFontEntity.hxx>
#include <IGESData_DirChecker.hxx>
#include <IGESData_Dump.hxx>
#include <IGESData_HArray1OfIGESEntity.hxx>
#include <IGESData_IGESDumper.hxx>
#include <IGESData_IGESEntity.hxx>
#include <IGESData_IGESReaderData.hxx>
#include <IGESData_IGESWriter.hxx>
#include <IGESData_LineFontEntity.hxx>
#include <IGESData_ParamCursor.hxx>
#include <IGESData_ParamReader.hxx>
#include <IGESData_ViewKindEntity.hxx>
#include <IGESDraw_HArray1OfViewKindEntity.hxx>
#include <IGESDraw_ToolViewsVisibleWithAttr.hxx>
#include <IGESDraw_ViewsVisibleWithAttr.hxx>
#include <IGESGraph_Color.hxx>
#include <IGESGraph_HArray1OfColor.hxx>
#include <Interface_Check.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_Macros.hxx>
#include <Interface_ShareTool.hxx>
#include <Message_Messenger.hxx>
#include <Standard_DomainError.hxx>
#include <TColStd_HArray1OfInteger.hxx>

#include <stdio.h>

void IGESDraw_ToolViewsVisibleWithAttr::ReadOwnParams
  (const Handle(IGESDraw_ViewsVisibleWithAttr)& ent,
   const Handle(IGESData_IGESReaderData)& IR, IGESData_ParamReader& PR) const
{
  //Standard_Boolean st; //szv#4:S4163:12Mar99 not needed

  Standard_Integer tempNbBlocks, tempNbEntity;
  Handle(IGESDraw_HArray1OfViewKindEntity) tempViewEntities;
  Handle(IGESGraph_HArray1OfColor) tempColorDefinitions;
  Handle(TColStd_HArray1OfInteger) tempLineFonts;
  Handle(TColStd_HArray1OfInteger) tempColorValues;
  Handle(TColStd_HArray1OfInteger) tempLineWeights;
  Handle(IGESData_HArray1OfIGESEntity) tempDisplayEntities;
  Handle(IGESBasic_HArray1OfLineFontEntity) tempLineDefinitions;

  if (PR.ReadInteger(tempNbBlocks,"Number Of Blocks")) {
    // Initialise HArray1 only if there is no error reading its Length
    if (tempNbBlocks <= 0)
      PR.AddFail("Number Of Blocks : Not Positive");
    else {
      tempViewEntities     = new IGESDraw_HArray1OfViewKindEntity(1, tempNbBlocks);
      tempLineFonts        = new TColStd_HArray1OfInteger(1, tempNbBlocks);
      tempLineDefinitions  = new IGESBasic_HArray1OfLineFontEntity(1, tempNbBlocks);
      tempColorValues      = new TColStd_HArray1OfInteger(1, tempNbBlocks);
      tempColorDefinitions = new IGESGraph_HArray1OfColor(1, tempNbBlocks);
      tempLineWeights      = new TColStd_HArray1OfInteger(1, tempNbBlocks);
    }
  }

  if (PR.DefinedElseSkip())
    PR.ReadInteger(tempNbEntity,"Number of Entities Displayed");
  else {
    tempNbEntity = 0;
    PR.AddWarning("Number of Entities Displayed : undefined, set to Zero");
  }
  // Initialise HArray1 only if there is no error reading its Length
  if      (tempNbEntity < 0)
    PR.AddFail ("Number Of Entities Displayed : Less than Zero");

  // Read the HArray1 only if its Length was read without any Error
  if (! (tempViewEntities.IsNull())) {
    // Assumption : When tempViewEntities != NULL, all other parallel
    //              arrays are also non-NULL
    Standard_Integer I;
    for (I = 1; I <= tempNbBlocks; I++) {
      Handle(IGESData_ViewKindEntity) tempView;
      Standard_Integer        tempLineFont;
      Handle(IGESData_LineFontEntity) tempEntity1;
      Standard_Integer        tempColorValue;
      Handle(IGESGraph_Color) tempColorDef;
      Standard_Integer        tempLineWeightValue;

      if (PR.ReadEntity(IR, "View Entity", STANDARD_TYPE(IGESData_ViewKindEntity), tempView))
	tempViewEntities->SetValue(I, tempView);

      if (PR.ReadInteger(tempLineFont,"Line Font Value"))
	tempLineFonts->SetValue(I, tempLineFont);

      if (tempLineFont == 0 &&
	  PR.ReadEntity(IR, "Line Font Definition",
			STANDARD_TYPE(IGESData_LineFontEntity),
			tempEntity1, Standard_True))
	tempLineDefinitions->SetValue(I, tempEntity1);

      Standard_Integer curnum = PR.CurrentNumber();
      //  Reading Color : Value (>0) or Definition (<0 = D.E. Pointer)
      if (PR.DefinedElseSkip())
	PR.ReadInteger(tempColorValue,"Color Value");
      else {
	tempColorValue = 0;
	PR.AddWarning ("Color Value : undefined, set to Zero");
      }
      if (tempColorValue < 0) {
	tempColorValues->SetValue(I, -1);
	tempColorDef = GetCasted(IGESGraph_Color,PR.ParamEntity(IR,curnum));
	if (tempColorDef.IsNull()) PR.AddFail
	  ("A Color Definition Entity is incorrect");
	else  tempColorDefinitions->SetValue(I, tempColorDef);
      }
      else
	tempColorValues->SetValue(I, tempColorValue);

      if (PR.ReadInteger(tempLineWeightValue,"Line Weight Value"))
	tempLineWeights->SetValue(I, tempLineWeightValue);
    }
  }

  // Read the HArray1 only if its Length was read without any Error
  if (tempNbEntity > 0) {
    PR.ReadEnts (IR,PR.CurrentList(tempNbEntity),
		 "Displayed Entities",tempDisplayEntities); //szv#4:S4163:12Mar99 `st=` not needed
  }

  DirChecker(ent).CheckTypeAndForm(PR.CCheck(),ent);
  ent->Init
    (tempViewEntities, tempLineFonts, tempLineDefinitions,
     tempColorValues, tempColorDefinitions, tempLineWeights,
     tempDisplayEntities);
}

void IGESDraw_ToolViewsVisibleWithAttr::WriteOwnParams
  (const Handle(IGESDraw_ViewsVisibleWithAttr)& ent, IGESData_IGESWriter& IW)  const
{
  Standard_Integer up  = ent->NbViews();
  IW.Send(up);
  IW.Send(ent->NbDisplayedEntities());

  Standard_Integer I;
  for (I = 1; I <= up; I++) {
    IW.Send(ent->ViewItem(I));
    IW.Send(ent->LineFontValue(I));
    IW.Send(ent->FontDefinition(I));  // controlled by LineFontValue, both sent
    if (ent->IsColorDefinition(I))
      IW.Send(ent->ColorDefinition(I),Standard_True);  // negative
    else
      IW.Send(ent->ColorValue(I));
    IW.Send(ent->LineWeightItem(I));
  }
  up  = ent->NbDisplayedEntities();
  for (I = 1; I <= up; I++)
    IW.Send(ent->DisplayedEntity(I));
}

void  IGESDraw_ToolViewsVisibleWithAttr::OwnShared
  (const Handle(IGESDraw_ViewsVisibleWithAttr)& ent, Interface_EntityIterator& iter) const
{
  Standard_Integer up  = ent->NbViews();

  Standard_Integer I;
  for (I = 1; I <= up; I++) {
    iter.GetOneItem(ent->ViewItem(I));
    iter.GetOneItem(ent->FontDefinition(I));
    if ( ent->IsColorDefinition(I) )
      iter.GetOneItem(ent->ColorDefinition(I));
  }
//  Displayed -> Implied
}


IGESData_DirChecker IGESDraw_ToolViewsVisibleWithAttr::DirChecker
  (const Handle(IGESDraw_ViewsVisibleWithAttr)& /*ent*/)  const
{
  IGESData_DirChecker DC(402, 4);
  DC.Structure(IGESData_DefVoid);
  DC.LineFont(IGESData_DefVoid);
  DC.LineWeight(IGESData_DefVoid);
  DC.Color(IGESData_DefVoid);
  DC.BlankStatusIgnored();
  DC.SubordinateStatusRequired(0);
  DC.UseFlagRequired(1);
  DC.HierarchyStatusIgnored();

  return DC;
}

void IGESDraw_ToolViewsVisibleWithAttr::OwnCheck
  (const Handle(IGESDraw_ViewsVisibleWithAttr)& ent,
   const Interface_ShareTool& , Handle(Interface_Check)& ach)  const
{
  Standard_Integer nb = ent->NbViews();
  Standard_Integer i; //svv Jan 10 2000 : porting on DEC
  for (i = 1; i <= nb; i ++) {
    if (ent->LineFontValue(i) != 0 && ent->IsFontDefinition(i)) ach->AddFail
      ("At least one Line Font Definition Mismatch (both Value and Entity");
  }
  Handle(IGESData_ViewKindEntity) entcomp (ent);
  Standard_Integer res = 0;
  nb = ent->NbDisplayedEntities();
  for (i = 1; i <= nb; i ++) {
    Handle(IGESData_IGESEntity) displayed = ent->DisplayedEntity(i);
    if (entcomp != displayed->View()) res ++;
  }
  if (!res) return;
  char mess[80];
  sprintf(mess,"Mismatch for %d Entities displayed",res);
  ach->AddFail(mess,"Mismatch for %d Entities displayed");
}


void IGESDraw_ToolViewsVisibleWithAttr::OwnDump
  (const Handle(IGESDraw_ViewsVisibleWithAttr)& ent, const IGESData_IGESDumper& dumper,
  const Handle(Message_Messenger)& S, const Standard_Integer level)  const
{
  Standard_Integer tempSubLevel = (level <= 4) ? 0 : 1;

  S << "IGESDraw_ViewsVisibleWithAttr" << endl;

  S << "View Entities            : " << endl
    << "Line Font Values         : " << endl
    << "Line Font Definitions    : " << endl
    << "Color Number/Definitions : " << endl
    << "Line Weights             : " << endl;
  S << "Count of View Blocks : "     << ent->NbViews() << endl;
  if (level > 4) {   // Level = 4 : nothing to Dump. Level = 5 & 6 : same Dump
    Standard_Integer I;
    Standard_Integer upper  = ent->NbViews();
    for (I = 1; I <= upper; I++) {
      S << "[" << I << "]: " << endl;
      S << "View Entity : ";
      dumper.Dump (ent->ViewItem(I),S, tempSubLevel);
      S << endl;

      if (ent->IsFontDefinition(I)) {
	S << "Line Font Definition  : ";
	dumper.Dump (ent->FontDefinition(I),S, tempSubLevel);
	S << endl;
      }
      else S << "Line Font Value       : " << ent->LineFontValue(I) << endl;

      if (ent->IsColorDefinition(I)) {
	S << "Color Definition : ";
	dumper.Dump (ent->ColorDefinition(I),S, tempSubLevel);
	S << endl;
      }
      else S << "Color Value      : " << ent->ColorValue(I) << endl;

      S      << "Line Weight      : " << ent->LineWeightItem(I) << endl;
    }
  }
  S << "Displayed Entities : ";
  IGESData_DumpEntities
    (S,dumper ,level,1, ent->NbDisplayedEntities(),ent->DisplayedEntity);
  S << endl;
}

Standard_Boolean  IGESDraw_ToolViewsVisibleWithAttr::OwnCorrect
  (const Handle(IGESDraw_ViewsVisibleWithAttr)& ent) const
{
//  Les entites affichees doivent referencer <ent>. Elles ont priorite.
  Standard_Boolean res = Standard_False;
  Standard_Integer nb = ent->NbDisplayedEntities();
  Handle(IGESData_ViewKindEntity) entcomp (ent);
  for (Standard_Integer i = 1; i <= nb; i ++) {
    Handle(IGESData_IGESEntity) displayed = ent->DisplayedEntity(i);
    if (entcomp != displayed->View()) res = Standard_True;
  }
  if (!res) return res;
  Handle(IGESData_HArray1OfIGESEntity) nulDisplayEntities;
  ent->InitImplied (nulDisplayEntities);
  return res;
}
