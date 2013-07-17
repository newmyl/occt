// Copyright (c) 1999-2012 OPEN CASCADE SAS
//
// The content of this file is subject to the Open CASCADE Technology Public
// License Version 6.5 (the "License"). You may not use the content of this file
// except in compliance with the License. Please obtain a copy of the License
// at http://www.opencascade.org and read it completely before using this file.
//
// The Initial Developer of the Original Code is Open CASCADE S.A.S., having its
// main offices at: 1, place des Freres Montgolfier, 78280 Guyancourt, France.
//
// The Original Code and all software distributed under the License is
// distributed on an "AS IS" basis, without warranty of any kind, and the
// Initial Developer hereby disclaims all such warranties, including without
// limitation, any warranties of merchantability, fitness for a particular
// purpose or non-infringement. Please see the License for the specific terms
// and conditions governing the rights and limitations under the License.

#include <STEPSelections_SelectForTransfer.hxx>
#include <Interface_EntityIterator.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <Transfer_TransientProcess.hxx>
#include <Standard_Type.hxx>
#include <XSControl_TransferReader.hxx>
#include <Interface_Graph.hxx>

IMPLEMENT_STANDARD_TYPE(STEPSelections_SelectForTransfer)
IMPLEMENT_STANDARD_SUPERTYPE_ARRAY()
  STANDARD_TYPE(XSControl_SelectForTransfer),
  STANDARD_TYPE(IFSelect_SelectExtract),
  STANDARD_TYPE(IFSelect_SelectDeduct),
  STANDARD_TYPE(IFSelect_Selection),
  STANDARD_TYPE(MMgt_TShared),
  STANDARD_TYPE(Standard_Transient),

IMPLEMENT_STANDARD_SUPERTYPE_ARRAY_END()
IMPLEMENT_STANDARD_TYPE_END(STEPSelections_SelectForTransfer)


IMPLEMENT_DOWNCAST(STEPSelections_SelectForTransfer,Standard_Transient)
IMPLEMENT_STANDARD_RTTI(STEPSelections_SelectForTransfer)


//=======================================================================
//function : STEPSelections_SelectForTransfer
//purpose  : 
//=======================================================================

STEPSelections_SelectForTransfer::STEPSelections_SelectForTransfer()
{
  
}
//=======================================================================
//function : STEPSelections_SelectForTransfer
//purpose  : 
//=======================================================================

STEPSelections_SelectForTransfer::STEPSelections_SelectForTransfer(const Handle(XSControl_TransferReader)& TR)
{
  SetReader(TR);
}

//=======================================================================
//function : RootResult
//purpose  : 
//=======================================================================

 Interface_EntityIterator STEPSelections_SelectForTransfer::RootResult(const Interface_Graph& /*G*/) const
{
  Interface_EntityIterator iter;
  Handle(TColStd_HSequenceOfTransient) roots = Reader()->TransientProcess()->RootsForTransfer();
  Standard_Integer nb = roots->Length();
    for(Standard_Integer i = 1; i <= nb ; i++) 
      iter.GetOneItem(roots->Value(i));
  return iter;
}

