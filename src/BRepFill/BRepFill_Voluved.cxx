// Created on: 2018-03-14
// Created by: Nikolai BUKHALOV
// Copyright (c) 1999-2018 OPEN CASCADE SAS
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


#include <BRepFill_Voluved.hxx>

#include <BRep_Builder.hxx>
#include <BRepFill_PipeShell.hxx>
#include <BRepFill_TransitionStyle.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BOPAlgo_Tools.hxx>
#include <BRepLib_FindSurface.hxx>
#include <Geom_Plane.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BOPAlgo_MakerVolume.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopExp_Explorer.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <math_MultipleVarFunctionWithHessian.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>
#include <Adaptor2d_HCurve2d.hxx>
#include <Adaptor3d_HSurface.hxx>
#include <math_NewtonMinimum.hxx>
#include <BOPTools_AlgoTools.hxx>
#include <math_Matrix.hxx>
#include <math_Vector.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepTools.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <BOPAlgo_BuilderFace.hxx>
#include <BOPAlgo_BuilderFace.hxx>
#include <Geom2d_Line.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <math_GlobOptMin.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Extrema_ExtPC.hxx>
#include <BOPDS_DS.hxx>
#include <BRepLib.hxx>

static const Standard_Real aPipeLinearTolerance = 1.0e-4;
static const Standard_Real aPipeAngularTolerance = 1.0e-2;

static Standard_Boolean CheckSingularityAndAdd(const TopoDS_Face& theF,
                                               TopTools_ListOfShape& theListOfFaces,
                                               TopTools_ListOfShape& theListOfSplits);

class NormalFunc;

//=======================================================================
//function : Perform
//purpose  : 
//=======================================================================
void BRepFill_Voluved::Perform(const TopoDS_Wire& theSpine, 
                               const TopoDS_Wire& theProfile,
                               const Standard_Boolean theSolidReq)
{
  myErrorStatus = BRepFill_Voluved_Empty;
  mySpine = theSpine;
  myProfile = theProfile;
  myPipeShell.Nullify();
  myTopBottom.Nullify();
  myResult.Nullify();

  PerformSweep();
  GetLids();

  if ((myErrorStatus != BRepFill_Voluved_NotSolid) && !theSolidReq)
  {
    myResult = myPipeShell;
  }

  BuildSolid();
}

//=======================================================================
//function : PerformSweep
//purpose  : 
//=======================================================================
void BRepFill_Voluved::PerformSweep()
{
  if (myErrorStatus != BRepFill_Voluved_Empty)
    return;

  myErrorStatus = BRepFill_Voluved_SweepError;

  Handle(BRepFill_PipeShell) aPipe = new BRepFill_PipeShell(mySpine);
  aPipe->SetTolerance(aPipeLinearTolerance, aPipeLinearTolerance, aPipeAngularTolerance);
  aPipe->SetTransition(BRepFill_TransitionStyle::BRepFill_Round);
  aPipe->Add(myProfile, Standard_False, Standard_False);

  if (aPipe->Build(Standard_False))
  {
    myErrorStatus = BRepFill_Voluved_NoLids;
    myPipeShell = aPipe->Shape();
  }
}

//=======================================================================
//function : GetLids
//purpose  : 
//=======================================================================
void BRepFill_Voluved::GetLids()
{
  if (myPipeShell.IsNull())
    return;

  if (BRep_Tool::IsClosed(myProfile))
  {
    // No need in lids creation
    myErrorStatus = BRepFill_Voluved_NotSolid;
    return;
  }

  myErrorStatus = BRepFill_Voluved_NoLids;

  BRepLib_FindSurface aFS(mySpine, -1.0, Standard_True);
  const Handle(Geom_Plane) aSurf = Handle(Geom_Plane)::DownCast(aFS.Surface());

  if (aSurf.IsNull())
  {
    myErrorStatus = BRepFill_Voluved_NotPlanarSpine;
    return;
  }

  //Square of the default angular tolerance in
  //BOPAlgo_Tools::EdgesToWires(...) and BOPAlgo_Tools::WiresToFaces(...) methods
  const Standard_Real aSqAnguarTol = 1.0e-16;
  const gp_Dir &aNormal = aSurf->Axis().Direction();

  // Obtain free-edges from myPipeShell. All edges must be planar
  // and parallel to the plane of mySpine

  TopTools_IndexedDataMapOfShapeListOfShape aMap;

  TopExp::MapShapesAndAncestors(myPipeShell, TopAbs_EDGE, TopAbs_FACE, aMap);

  TopTools_ListOfShape aLE;

  gp_Pnt aP;
  gp_Vec aTan;

  for (Standard_Integer i = 1; i <= aMap.Size(); i++)
  {
    TopTools_ListOfShape& aListF = aMap(i);

    if (aListF.Extent() != 1)
      continue;

    const TopoDS_Edge &anE = TopoDS::Edge(aMap.FindKey(i));

    BRepAdaptor_Curve anAC(anE);
    if (!anAC.Is3DCurve())
    {
      // We are not interested in degenerated edges.
      continue;
    }
    
    anAC.D1(0.5*(anAC.FirstParameter() + anAC.LastParameter()), aP, aTan);

    const Standard_Real aSqModulus = aTan.SquareMagnitude();
    if (aSqModulus < Precision::Confusion())
      continue;

    const Standard_Real aDP = aTan.XYZ().Dot(aNormal.XYZ());
    if (aDP*aDP>aSqModulus*aSqAnguarTol)
    {
      //Only planar edges are considered
      continue;
    }
    
    aLE.Append(anE);
  }

  if (aLE.IsEmpty())
  {
    myErrorStatus = BRepFill_Voluved_NotPlanarSpine;
    return;
  }

  // Split interfered edges
  BOPAlgo_PaveFiller aPF;
  aPF.SetArguments(aLE);
  aPF.SetRunParallel(Standard_True);

  aPF.Perform();
  if (aPF.HasErrors())
  {
    myErrorStatus = BRepFill_Voluved_NotPlanarSpine;
    return;
  }

  BOPAlgo_Builder aBuilder;
  TopTools_ListIteratorOfListOfShape aItLE(aLE);
  for (; aItLE.More(); aItLE.Next())
  {
    const TopoDS_Shape& aS = aItLE.Value();
    aBuilder.AddArgument(aS);
  }

  aBuilder.SetRunParallel(Standard_True);
  aBuilder.PerformWithFiller(aPF);
  if (aBuilder.HasErrors())
  {
    myErrorStatus = BRepFill_Voluved_NotPlanarSpine;
    return;
  }

  const TopoDS_Shape& aFreeEdges = aBuilder.Shape();

  // Collect all free edges to wires and create planar 
  // top and bottom lids from these wires.
  BRep_Builder aBB;
  TopoDS_Compound aCompW;
  aBB.MakeCompound(aCompW);
  aBB.MakeCompound(myTopBottom);
  BOPAlgo_Tools::EdgesToWires(aFreeEdges, aCompW, Standard_True);
  BOPAlgo_Tools::WiresToFaces(aCompW, myTopBottom);

  myErrorStatus = BRepFill_Voluved_NotSolid;
}

//=======================================================================
//function : BuildSolid
//purpose  : 
//=======================================================================
void BRepFill_Voluved::BuildSolid()
{
  if (myErrorStatus != BRepFill_Voluved_NotSolid)
    return;

  myErrorStatus = BRepFill_Voluved_NotVolume;

  TopTools_MapOfShape aMapF;
  TopTools_ListOfShape aLF, aLSplits;

  TopExp_Explorer anExpF(myPipeShell, TopAbs_FACE);

  for (; anExpF.More(); anExpF.Next())
  {
    const TopoDS_Face &aF = TopoDS::Face(anExpF.Current());
    if (!aMapF.Add(aF))
      continue;

    CheckSingularityAndAdd(aF, aLF, aLSplits);
  }

  if (!myTopBottom.IsNull())
  {
    TopoDS_Iterator anItLids(myTopBottom);
    for (; anItLids.More(); anItLids.Next())
    {
      const TopoDS_Face &aF = TopoDS::Face(anItLids.Value());
      aLF.Append(aF);
    }
  }
  
  {
    // Fix SameParameter
    TopoDS_ListIteratorOfListOfShape anItF(aLF);
    for (; anItF.More(); anItF.Next())
    {
      const TopoDS_Shape &aF = anItF.Value();
      BRepLib::SameParameter(aF, Precision::Confusion(), Standard_True);
    }
  }

  BOPAlgo_MakerVolume aMV;
  aMV.SetArguments(aLF);
  aMV.SetIntersect(Standard_True);
  aMV.SetRunParallel(Standard_True);
  aMV.SetAvoidInternalShapes(Standard_True);
  aMV.Perform();

  if (aMV.HasErrors())
  {
    return;
  }
  
  myResult = aMV.Shape();
  RemoveExcessSolids(myResult, aLF, aLSplits, aMV);

  myErrorStatus = BRepFill_Voluved_OK;
}

//=======================================================================
//function : ExtractOuterSolid
//purpose  : 
//=======================================================================
void BRepFill_Voluved::ExtractOuterSolid(TopoDS_Shape& theShape,
                                         TopTools_ListOfShape& theArgsList)
{
  TopTools_IndexedDataMapOfShapeListOfShape aMapS;
  TopExp::MapShapesAndAncestors(theShape, TopAbs_FACE, TopAbs_SOLID, aMapS);

  Standard_Boolean hasBeenDeleted = Standard_False;

  for (Standard_Integer i = 1; i <= aMapS.Extent(); i++)
  {
    const TopTools_ListOfShape &aL = aMapS(i);
    if (aL.Extent() > 1)
    {
      // Face is shared between several solids. ==> 
      // It cannot participate in the outer contour and should be removed.

      const TopoDS_Face &aF = TopoDS::Face(aMapS.FindKey(i));
      theArgsList.Remove(aF);
      hasBeenDeleted = Standard_True;
    }
  }

  if (!hasBeenDeleted)
    return;

  BOPAlgo_MakerVolume aMV;
  aMV.SetArguments(theArgsList);
  aMV.SetIntersect(Standard_True);
  aMV.SetRunParallel(Standard_True);
  aMV.SetAvoidInternalShapes(Standard_True);
  aMV.Perform();

  if (aMV.HasErrors())
  {
    return;
  }

  theShape = aMV.Shape();
}

//=======================================================================
//function : RemoveExcessSolids
//purpose  : 
//=======================================================================
void BRepFill_Voluved::RemoveExcessSolids(TopoDS_Shape& theShape,
                                          TopTools_ListOfShape& theArgsList,
                                          TopTools_ListOfShape& theLSplits,
                                          BOPAlgo_MakerVolume& theMV)
{
  if (myErrorStatus != BRepFill_Voluved_NotVolume)
    return;
  
  TopExp_Explorer anExpSo;
  for (Standard_Integer i = 0; i < 2; i++)
  {
    anExpSo.Init(theShape, TopAbs_SOLID);
    if (!anExpSo.More())
    {
      return;
    }

    anExpSo.Next();
    if (!anExpSo.More())
    {
      // Only one solid has been generated
      myResult = TopoDS::Solid(anExpSo.Current());
      return;
    }

    if (i != 0)
      break;

    ExtractOuterSolid(theShape, theArgsList);
  }

  // Create a list of invalid faces. The face is invalid if
  // BOPAlgo_MakerVolume changes its orientation while creating solids.
  // Faces from theLSplits are not checked.
  TopTools_ListOfShape aListInvFaces;
  TopTools_ListIteratorOfListOfShape anItl(theArgsList);
  for (anExpSo.Init(theShape, TopAbs_SOLID); anItl.More(); anItl.Next())
  {
    const TopoDS_Face &aF = TopoDS::Face(anItl.Value());
    if (theLSplits.Contains(aF))
      continue;

    for (TopTools_ListIteratorOfListOfShape anItM(theMV.Modified(aF));
         anItM.More(); anItM.Next())
    {
      const TopoDS_Face &aFM = TopoDS::Face(anItM.Value());

      if (aFM.Orientation() != aF.Orientation())
        aListInvFaces.Append(aFM);
    }
  }

  TopTools_ListOfShape aSolidList;
  for (anExpSo.Init(theShape, TopAbs_SOLID); anExpSo.More(); anExpSo.Next())
  {
    Standard_Boolean isToDelete = Standard_False;
    const TopoDS_Solid &aSo = TopoDS::Solid(anExpSo.Current());
    for (TopExp_Explorer anExpF(aSo, TopAbs_FACE);
         anExpF.More(); anExpF.Next())
    {
      const TopoDS_Face& aF = TopoDS::Face(anExpF.Current());
      if (aListInvFaces.Contains(aF))
      {
        isToDelete = Standard_True;
        break;
      }
    }

    if (!isToDelete)
    {
      aSolidList.Append(aSo);
    }
  }

  if (aSolidList.Extent() < 1)
  {
    myResult = theShape;
    return;
  }

  if (aSolidList.Extent() == 1)
  {
    myResult = aSolidList.First();
    return;
  }

  BRep_Builder aBB;
  TopoDS_Compound aCmpSol;
  aBB.MakeCompound(aCmpSol);

  for (anItl.Init(aSolidList); anItl.More(); anItl.Next())
  {
    const TopoDS_Solid &aSo = TopoDS::Solid(anItl.Value());
    aBB.Add(aCmpSol, aSo);
  }

  myResult = aCmpSol;
}

//=======================================================================
//class : NormalFunc
//purpose  : 
//=======================================================================
class NormalFunc : public math_MultipleVarFunctionWithHessian
{
public:
  NormalFunc(const Adaptor3d_CurveOnSurface& theCOS) :myCOnS(theCOS)
  {
  }

  virtual Standard_Integer NbVariables() const Standard_OVERRIDE
  {
    return 1;
  }


  virtual Standard_Boolean Value(const math_Vector& X, Standard_Real& F) Standard_OVERRIDE;
  virtual Standard_Boolean Gradient(const math_Vector& X, math_Vector& G) Standard_OVERRIDE;
  virtual Standard_Boolean Values(const math_Vector& theX,
                                  Standard_Real& theF,
                                  math_Vector& theG) Standard_OVERRIDE
  {
    if (!Value(theX, theF))
    return Standard_False;

    if (!Gradient(theX, theG))
      return Standard_False;

    return Standard_True;
  };

  virtual Standard_Boolean Values(const math_Vector& theX,
                                  Standard_Real& theF,
                                  math_Vector& theG,
                                  math_Matrix& theH) Standard_OVERRIDE
  {
    if (!Values(theX, theF, theG))
    return Standard_False;

    theH(1, 1) = theG(1);
    return Standard_True;
  };

  Standard_Real FirstParameter() const
  {
    return myCOnS.FirstParameter();
  }

  Standard_Real LastParameter() const
  {
    return myCOnS.LastParameter();
  }

  gp_Pnt GetPoint(const Standard_Real theX)
  {
    const Handle(Adaptor2d_HCurve2d) &aC = myCOnS.GetCurve();
    const Handle(Adaptor3d_HSurface) &aS = myCOnS.GetSurface();
    const gp_Pnt2d aP2d(aC->Value(theX));
    return aS->Value(aP2d.X(), aP2d.Y());
  }

protected:

  NormalFunc& operator=(NormalFunc&);

private:
  const Adaptor3d_CurveOnSurface& myCOnS;
};

//=======================================================================
//function : Value
//purpose  : +aD1v_x^2*aD1u_y^2 + aD1v_x^2*aD1u_z^2 +
//           +aD1v_y^2*aD1u_z^2 + aD1u_x^2*aD1v_y^2 + 
//           +aD1u_x^2*aD1v_z^2 + aD1u_y^2*aD1v_z^2 -
//           -  2*(+aD1u_x*aD1v_x*aD1u_y*aD1v_y + 
//                 +aD1u_x*aD1v_x*aD1u_z*aD1v_z +
//                 +aD1u_y*aD1v_y*aD1u_z*aD1v_z)
//=======================================================================
Standard_Boolean NormalFunc::Value(const math_Vector& theX, Standard_Real& theF)
{
  const Handle(Adaptor2d_HCurve2d) &aC = myCOnS.GetCurve();
  const Handle(Adaptor3d_HSurface) &aS = myCOnS.GetSurface();

  const gp_Pnt2d aP2d(aC->Value(theX(1)));
  gp_Pnt aP3d;
  gp_Vec aD1u, aD1v;
  aS->D1(aP2d.X(), aP2d.Y(), aP3d, aD1u, aD1v);

  theF = aD1u.Crossed(aD1v).SquareMagnitude();
  return Standard_True;
}

//=======================================================================
//function : Gradient
//purpose  :
//2 * ((aD1v_x*aD1u_y)*(aD1u_y*(aD2uv_x*aDc_x + aD2v_x*aDc_y) + aD1v_x*(aD2u_y*aDc_x + aD2uv_y*aDc_y)) +
//     (aD1v_x*aD1u_z)*(aD1u_z*(aD2uv_x*aDc_x + aD2v_x*aDc_y) + aD1v_x*(aD2u_z*aDc_x + aD2uv_z*aDc_y)) +
//     (aD1v_y*aD1u_z)*(aD1u_z*(aD2uv_y*aDc_x + aD2v_y*aDc_y) + aD1v_y*(aD2u_z*aDc_x + aD2uv_z*aDc_y)) +
//     (aD1u_x*aD1v_y)*(aD1u_x*(aD2uv_y*aDc_x + aD2v_y*aDc_y) + aD1v_y*(aD2u_x*aDc_x + aD2uv_x*aDc_y)) +
//     (aD1u_x*aD1v_z)*(aD1u_x*(aD2uv_z*aDc_x + aD2v_z*aDc_y) + aD1v_z*(aD2u_x*aDc_x + aD2uv_x*aDc_y)) +
//     (aD1u_y*aD1v_z)*(aD1u_y*(aD2uv_z*aDc_x + aD2v_z*aDc_y) + aD1v_z*(aD2u_y*aDc_x + aD2uv_y*aDc_y)) -
//
//     (aD2u_x*aDc_x + aD2uv_x*aDc_y)*aD1v_x*aD1u_y*aD1v_y -
//     aD1u_x*(aD2uv_x*aDc_x + aD2v_x*aDc_y)*aD1u_y*aD1v_y -
//     aD1u_x*aD1v_x*(aD2u_y*aDc_x + aD2uv_y*aDc_y)*aD1v_y -
//     aD1u_x*aD1v_x*aD1u_y*(aD2uv_y*aDc_x + aD2v_y*aDc_y) -
//
//     (aD2u_x*aDc_x + aD2uv_x*aDc_y)*aD1v_x*aD1u_z*aD1v_z -
//     aD1u_x*(aD2uv_x*aDc_x + aD2v_x*aDc_y)*aD1u_z*aD1v_z -
//     aD1u_x*aD1v_x*(aD2u_z*aDc_x + aD2uv_z*aDc_y)*aD1v_z -
//     aD1u_x*aD1v_x*aD1u_z*(aD2uv_z*aDc_x + aD2v_z*aDc_y) -
//
//     (aD2u_y*aDc_x + aD2uv_y*aDc_y)*aD1v_y*aD1u_z*aD1v_z -
//     aD1u_y*(aD2uv_y*aDc_x + aD2v_y*aDc_y)*aD1u_z*aD1v_z -
//     aD1u_y*aD1v_y*(aD2u_z*aDc_x + aD2uv_z*aDc_y)*aD1v_z -
//     aD1u_y*aD1v_y*aD1u_z*(aD2uv_z*aDc_x + aD2v_z*aDc_y))
//=======================================================================
Standard_Boolean NormalFunc::Gradient(const math_Vector& theX, math_Vector& theG)
{
  const Handle(Adaptor2d_HCurve2d) &aC = myCOnS.GetCurve();
  const Handle(Adaptor3d_HSurface) &aS = myCOnS.GetSurface();

  gp_Pnt2d aP2d;
  gp_Vec2d aDc;
  aC->D1(theX(1), aP2d, aDc);

  gp_Pnt aP3d;
  gp_Vec aD1u, aD1v, aD2u, aD2v, aD2uv;
  aS->D2(aP2d.X(), aP2d.Y(), aP3d, aD1u, aD1v, aD2u, aD2v, aD2uv);

  theG(1) = (aD1v.X()*aD1u.Y())*(aD1u.Y()*(aD2uv.X()*aDc.X() + aD2v.X()*aDc.Y()) +
            aD1v.X()*(aD2u.Y()*aDc.X() + aD2uv.Y()*aDc.Y())) + 
            (aD1v.X()*aD1u.Z())*(aD1u.Z()*(aD2uv.X()*aDc.X() + 
            aD2v.X()*aDc.Y()) + aD1v.X()*(aD2u.Z()*aDc.X() + aD2uv.Z()*aDc.Y())) +
            (aD1v.Y()*aD1u.Z())*(aD1u.Z()*(aD2uv.Y()*aDc.X() + aD2v.Y()*aDc.Y()) + 
            aD1v.Y()*(aD2u.Z()*aDc.X() + aD2uv.Z()*aDc.Y())) + (aD1u.X()*aD1v.Y())*
            (aD1u.X()*(aD2uv.Y()*aDc.X() + aD2v.Y()*aDc.Y()) + aD1v.Y()*(aD2u.X()*
            aDc.X() + aD2uv.X()*aDc.Y())) + (aD1u.X()*aD1v.Z())*(aD1u.X()*(aD2uv.Z()*
            aDc.X() + aD2v.Z()*aDc.Y()) + aD1v.Z()*(aD2u.X()*aDc.X() + 
            aD2uv.X()*aDc.Y())) + (aD1u.Y()*aD1v.Z())*(aD1u.Y()*(aD2uv.Z()*aDc.X() + 
            aD2v.Z()*aDc.Y()) + aD1v.Z()*(aD2u.Y()*aDc.X() + aD2uv.Y()*aDc.Y())) -
            (aD2u.X()*aDc.X() + aD2uv.X()*aDc.Y())*aD1v.X()*aD1u.Y()*aD1v.Y() - 
            aD1u.X()*(aD2uv.X()*aDc.X() + aD2v.X()*aDc.Y())*aD1u.Y()*aD1v.Y() -
            aD1u.X()*aD1v.X()*(aD2u.Y()*aDc.X() + aD2uv.Y()*aDc.Y())*aD1v.Y() - 
            aD1u.X()*aD1v.X()*aD1u.Y()*(aD2uv.Y()*aDc.X() + aD2v.Y()*aDc.Y()) - 
            (aD2u.X()*aDc.X() + aD2uv.X()*aDc.Y())*aD1v.X()*aD1u.Z()*aD1v.Z() - 
            aD1u.X()*(aD2uv.X()*aDc.X() + aD2v.X()*aDc.Y())*aD1u.Z()*aD1v.Z() - 
            aD1u.X()*aD1v.X()*(aD2u.Z()*aDc.X() + aD2uv.Z()*aDc.Y())*aD1v.Z() - 
            aD1u.X()*aD1v.X()*aD1u.Z()*(aD2uv.Z()*aDc.X() + aD2v.Z()*aDc.Y()) - 
            (aD2u.Y()*aDc.X() + aD2uv.Y()*aDc.Y())*aD1v.Y()*aD1u.Z()*aD1v.Z() - 
            aD1u.Y()*(aD2uv.Y()*aDc.X() + aD2v.Y()*aDc.Y())*aD1u.Z()*aD1v.Z() - 
            aD1u.Y()*aD1v.Y()*(aD2u.Z()*aDc.X() + aD2uv.Z()*aDc.Y())*aD1v.Z() -
            aD1u.Y()*aD1v.Y()*aD1u.Z()*(aD2uv.Z()*aDc.X() + aD2v.Z()*aDc.Y());

  return Standard_True;
}

//=======================================================================
//structure : EInfoTosplit
//purpose  : 
//=======================================================================
struct EInfoTosplit
{
  TopoDS_Edge myEdge;
  TopoDS_Vertex myVertexToSplit;
  Standard_Real myParam;
  Standard_Real myTolerance;
};

//=======================================================================
//function : RebuildFaces
//purpose  : Creates a wires from theEdges and puts it to the new face
//            which is empty-copied from theSourceFace.
//=======================================================================
static void RebuildFaces(const TopTools_ListOfShape& theLE,
                         const TopoDS_Face& theSourceFace,
                         TopTools_ListOfShape& theList)
{
  //build new faces
  BOPAlgo_BuilderFace aBF;

  TopoDS_Face aF = TopoDS::Face(theSourceFace.Oriented(TopAbs_FORWARD));

  aBF.SetFace(aF);
  aBF.SetShapes(theLE);

  aBF.Perform();

  const TopTools_ListOfShape& aLFR = aBF.Areas();

  if (aLFR.IsEmpty())
  {
    theList.Append(theSourceFace);
    return;
  }

  TopTools_ListIteratorOfListOfShape aItFR(aLFR);
  for (; aItFR.More(); aItFR.Next())
  {
    const TopoDS_Shape& aFR = TopoDS::Face(aItFR.Value());
    theList.Append(aFR);
  }
}

//=======================================================================
//function : MakeEdgeDegenerated
//purpose  : Returns TRUE if degenerated edge has been created.
//=======================================================================
static Standard_Boolean MakeEdgeDegenerated(const TopoDS_Vertex& theV,
                                            const TopoDS_Face& theFace,
                                            const gp_Pnt2d& thePf,
                                            const gp_Pnt2d& thePl,
                                            TopTools_ListOfShape& theLEdges)
{
  BRepAdaptor_Surface anAS(theFace, Standard_False);

  const Standard_Real aTol = 2.0*BRep_Tool::Tolerance(theV);
  const Standard_Real aTolU = anAS.UResolution(aTol),
                      aTolV = anAS.VResolution(aTol);

  if ((Abs(thePf.X() - thePl.X()) < aTolU) && (Abs(thePf.Y() - thePl.Y()) < aTolV))
    return Standard_False;

  const TopoDS_Vertex aVf = TopoDS::Vertex(theV.Oriented(TopAbs_FORWARD)),
                      aVl = TopoDS::Vertex(theV.Oriented(TopAbs_REVERSED));
  
  const gp_XY aV = thePl.XY() - thePf.XY();
  const Handle(Geom2d_Line) aL1 = new Geom2d_Line(thePf, gp_Dir2d(aV));
  const Handle(Geom2d_Line) aL2 = new Geom2d_Line(thePl, gp_Dir2d(aV.Reversed()));

  BRep_Builder aBB;
  TopoDS_Edge anEdegen1, anEdegen2;
  aBB.MakeEdge(anEdegen1);
  aBB.MakeEdge(anEdegen2);

  aBB.UpdateEdge(anEdegen1, aL1, theFace, Precision::Confusion());
  aBB.UpdateEdge(anEdegen2, aL2, theFace, Precision::Confusion());

  anEdegen1.Orientation(TopAbs_FORWARD);
  anEdegen2.Orientation(TopAbs_FORWARD);

  aBB.Add(anEdegen1, aVf);
  aBB.Add(anEdegen1, aVl);
  aBB.Add(anEdegen2, aVf);
  aBB.Add(anEdegen2, aVl);

  aBB.Degenerated(anEdegen1, Standard_True);
  aBB.Degenerated(anEdegen2, Standard_True);

  const Standard_Real aLPar = aV.Modulus();
  aBB.Range(anEdegen1, 0.0, aLPar);
  aBB.Range(anEdegen2, 0.0, aLPar);

  theLEdges.Append(anEdegen1);
  theLEdges.Append(anEdegen2);

  return Standard_True;
}

//=======================================================================
//function : InsertEDegenerated
//purpose  : 
//=======================================================================
static void InsertEDegenerated(const TopoDS_Face& theFace,
                               TopTools_ListOfShape& theLEdges)
{
  // Every degenerated edge (to split) must be added in theLEdges twice
  // with different orientations. Moreover, Degenerated edges cannot be shared.
  // Therefore, make copy of them before adding.

  BRep_Builder aBB;
  TopoDS_Wire aWir;
  aBB.MakeWire(aWir);

  TopTools_ListIteratorOfListOfShape anItr(theLEdges);
  for (; anItr.More(); anItr.Next())
  {
    const TopoDS_Edge &anE = TopoDS::Edge(anItr.Value());
    aBB.Add(aWir, anE);
  }

  TopTools_IndexedDataMapOfShapeListOfShape aMapVE;
  TopExp::MapShapesAndUniqueAncestors(aWir, TopAbs_VERTEX, TopAbs_EDGE, aMapVE);

  BRepTools_WireExplorer anExp(aWir, theFace);

  TopoDS_Edge anE1 = anExp.Current(), aFirstEdge, aLastEdge;

  if (anE1.IsNull())
  {
    // It is possible if aWir contains
    // only INTERNAL/EXTERNAL edges.

    return;
  }

  aFirstEdge = anE1;
  anExp.Next();

  if (!anExp.More())
  {
    // The wire contains only single edge.
    // But this edge can be closed itself (e.g. circle).

    TopoDS_Vertex aVf, aVl;
    TopExp::Vertices(anE1, aVf, aVl);
    if (!aVf.IsNull() && aVf.IsSame(aVl))
    {
      Standard_Real aF, aL;
      const Handle(Geom2d_Curve) aC = BRep_Tool::CurveOnSurface(anE1, theFace, aF, aL);
      aF = BRep_Tool::Parameter(aVf, anE1);
      aL = BRep_Tool::Parameter(aVl, anE1);
      const gp_Pnt2d aPf(aC->Value(aF)), aPl(aC->Value(aL));

      MakeEdgeDegenerated(aVf, theFace, aPf, aPl, theLEdges);
    }

    return;
  }

  // Map containing all vertices of degenerated edges
  TopTools_MapOfShape aMapVofDE;

  for (; anExp.More(); anExp.Next())
  {
    const TopoDS_Edge& anE2 = anExp.Current();
    aLastEdge = anE2;
    //if (anE1.IsSame(anE2))
    //{
    //  //Exclude a gap between two seam-edges (e.g. cylinder without bottom-base).
    //  anE1 = anE2;
    //  continue;
    //}

    const TopoDS_Vertex &aVertCurr = anExp.CurrentVertex();

    if (aMapVofDE.Contains(aVertCurr))
    {
      // Necessary degenerated edge has already been created.
      anE1 = anE2;
      continue;
    }

    Standard_Real aF, aL;
    const Handle(Geom2d_Curve) aC1 = BRep_Tool::CurveOnSurface(anE1, theFace, aF, aL),
                               aC2 = BRep_Tool::CurveOnSurface(anE2, theFace, aF, aL);
    aF = BRep_Tool::Parameter(aVertCurr, anE1);
    aL = BRep_Tool::Parameter(aVertCurr, anE2);
    const gp_Pnt2d aPf(aC1->Value(aF)), aPl(aC2->Value(aL));

    if (MakeEdgeDegenerated(aVertCurr, theFace, aPf, aPl, theLEdges))
    {
      aMapVofDE.Add(aVertCurr);
      anE1 = anE2;
      continue;
    }

    const TopTools_ListOfShape *anEList = aMapVE.Seek(aVertCurr);
    if ((anEList != 0) && (anEList->Extent() <= 2))
    {
      anE1 = anE2;
      continue;
    }

    // Case like cone with apex. In 2D space all is OK
    // (therefore BRepTools_WireExplorer processes this case
    // correctly). But in 3D-space, we have several edges with
    // the same vertex. Cone apex must be plugged by degenerated edge.

    Standard_Boolean hasDegenerated = Standard_False;
    TopTools_ListIteratorOfListOfShape anItr(*anEList);
    for (; anItr.More(); anItr.Next())
    {
      const TopoDS_Edge &anEdge = TopoDS::Edge(anItr.Value());
      if (BRep_Tool::Degenerated(anEdge))
      {
        hasDegenerated = Standard_True;
        break;
      }
    }

    if (hasDegenerated)
    {
      anE1 = anE2;
      continue;
    }

    // Look for the pair for anE1 and anE2 edges
    for (Standard_Integer i = 0; i < 2; i++)
    {
      const gp_Pnt2d &aPoint = i ? aPl : aPf;
      anItr.Initialize(*anEList);
      for (; anItr.More(); anItr.Next())
      {
        const TopoDS_Edge &anEdge = TopoDS::Edge(anItr.Value());

        if (anEdge.IsSame(anE1) || anEdge.IsSame(anE2))
          continue;

        const Handle(Geom2d_Curve) aC = BRep_Tool::CurveOnSurface(anEdge, theFace, aF, aL);
        aF = BRep_Tool::Parameter(aVertCurr, anEdge);
        const gp_Pnt2d aP(aC->Value(aF));

        if (MakeEdgeDegenerated(aVertCurr, theFace, aPoint, aP, theLEdges))
        {
          aMapVofDE.Add(aVertCurr);
          i = 2;
          break;
        }
      }
    }
    
    anE1 = anE2;
  }

  if (aFirstEdge.IsNull() || aLastEdge.IsNull())
    return;

  //if (aFirstEdge.IsSame(aLastEdge))
  //{
  //  //Exclude a gap between two seam-edges (e.g. cylinder without bottom-base).

  //  return;
  //}

  //TopExp::CommonVertex(...) does not work
  //if edges have more than one pair of common vertex
  //(e.g. two halves of circle). Here, we process this case.
  TopoDS_Vertex aV[4];
  TopExp::Vertices(aFirstEdge, aV[0], aV[1]);
  if (!aV[0].IsNull() && aV[0].IsSame(aV[1]))
  {
    // Possible reason is the NOT-CLOSED edge
    // has only single vertex and is covered by it.
    return;
  }

  TopExp::Vertices(aLastEdge, aV[2], aV[3]);
  if (!aV[2].IsNull() && aV[2].IsSame(aV[3]))
  {
    // Possible reason is the NOT-CLOSED edge
    // has only single vertex and is covered by it.
    return;
  }

  for (Standard_Integer anIDFE = 0; anIDFE < 2; anIDFE++)
  {
    for (Standard_Integer anIDLE = 2; anIDLE < 4; anIDLE++)
    {
      if (!aV[anIDFE].IsSame(aV[anIDLE]))
        continue;

      const NCollection_List<TopoDS_Shape> *anEList = aMapVE.Seek(aV[anIDFE]);
      if ((anEList != 0) && (anEList->Extent() > 2))
      {
        // Causes:
        //  1. Non-manifold topology.
        //  2. Case such as:
        //
        //        *************************
        //        *                       *
        //  seam  *                       *  seam
        //        *  edge1         edge2  *
        //        * ********    ********* *
        //       V1        V2   V3       V4
        //
        //
        //  V1 - vertex between edge1 and seam
        //  V4 - vertex between edge2 and seam
        //
        //  Indeed, V1 and V4 are same but they
        //  must not be joined.

        continue;
      }

      Standard_Real aF, aL;
      const Handle(Geom2d_Curve) aC1 = BRep_Tool::CurveOnSurface(aFirstEdge, theFace, aF, aL),
                                 aC2 = BRep_Tool::CurveOnSurface(aLastEdge, theFace, aF, aL);
      aF = BRep_Tool::Parameter(aV[anIDFE], aFirstEdge);
      aL = BRep_Tool::Parameter(aV[anIDLE], aLastEdge);
      const gp_Pnt2d aPf(aC1->Value(aF)), aPl(aC2->Value(aL));

      MakeEdgeDegenerated(aV[anIDFE], theFace, aPf, aPl, theLEdges);
    }
  }
}

//=======================================================================
//function : CheckSingularityAndAdd
//purpose  : Returns TRUE if theF has been split
//=======================================================================
Standard_Boolean CheckSingularityAndAdd(const TopoDS_Face& theF,
                                        TopTools_ListOfShape& theListOfFaces,
                                        TopTools_ListOfShape& theListOfSplits)
{
  const BRepAdaptor_Surface anAS(theF, Standard_False);
  GeomAbs_SurfaceType aSType = anAS.GetType();

  if (aSType == GeomAbs_OffsetSurface)
  {
    aSType = anAS.BasisSurface()->GetType();
  }

  if (aSType == GeomAbs_Plane)
  {
    TopTools_MapOfShape aME;
    TopTools_ListOfShape aLE;
    TopExp_Explorer anExp(theF, TopAbs_EDGE);
    for (; anExp.More(); anExp.Next())
    {
      const TopoDS_Edge &anE = TopoDS::Edge(anExp.Current());

      if (aME.Add(anE))
        aLE.Append(anE);
    }

    // Split interfered edges
    BOPAlgo_PaveFiller aPF;
    aPF.SetArguments(aLE);
    aPF.SetRunParallel(Standard_True);

    aPF.Perform();
    if (aPF.HasErrors())
    {
      theListOfFaces.Append(theF);
      return Standard_False;
    }

    const BOPDS_DS &aDS = aPF.DS();
    if (aDS.NbShapes() == aDS.NbSourceShapes())
    {
      //Interfered edges have not been detected
      theListOfFaces.Append(theF);
      return Standard_False;
    }

    BOPAlgo_Builder aBuilder;
    TopTools_ListIteratorOfListOfShape aItLE(aLE);
    for (; aItLE.More(); aItLE.Next())
    {
      const TopoDS_Shape& aS = aItLE.Value();
      aBuilder.AddArgument(aS);
    }

    aBuilder.SetRunParallel(Standard_True);
    aBuilder.PerformWithFiller(aPF);
    if (aBuilder.HasErrors())
    {
      theListOfFaces.Append(theF);
      return Standard_False;
    }

    const TopoDS_Shape& anEdges = aBuilder.Shape();

    // Collect all free edges to wires and create planar 
    // top and bottom lids from these wires.
    BRep_Builder aBB;
    TopoDS_Compound aCompW, aCompF;
    aBB.MakeCompound(aCompW);
    aBB.MakeCompound(aCompF);
    BOPAlgo_Tools::EdgesToWires(anEdges, aCompW, Standard_True);
    BOPAlgo_Tools::WiresToFaces(aCompW, aCompF);

    aME.Clear();
    anExp.Init(aCompF, TopAbs_FACE);
    for (; anExp.More(); anExp.Next())
    {
      const TopoDS_Face &aF = TopoDS::Face(anExp.Current());
      theListOfFaces.Append(aF);
    }

    return Standard_True;
  }

  if ((aSType != GeomAbs_Cone) && 
      (aSType != GeomAbs_Sphere) && 
      (aSType != GeomAbs_BezierSurface) &&
      (aSType != GeomAbs_BSplineSurface) &&
      (aSType != GeomAbs_SurfaceOfRevolution))
  {
    theListOfFaces.Append(theF);
    return Standard_False;
  }

  BRep_Builder aBB;

  TopoDS_Compound aCWires;
  aBB.MakeCompound(aCWires);

  Standard_Boolean isSplit = Standard_False;
  TopTools_ListOfShape aLE;

  const TopoDS_Face aF = TopoDS::Face(theF.Oriented(TopAbs_FORWARD));

  for (TopoDS_Iterator anExpW(aF); anExpW.More(); anExpW.Next())
  {
    const TopoDS_Wire &aWir = TopoDS::Wire(anExpW.Value());

    TopTools_ListOfShape aLGF;
    TopExp_Explorer anEExp(aWir, TopAbs_EDGE);
    for (; anEExp.More(); anEExp.Next())
    {
      const TopoDS_Edge &anE = TopoDS::Edge(anEExp.Current());
      aLGF.Append(anE);
    }

    BOPAlgo_PaveFiller aPF;
    aPF.SetArguments(aLGF);
    aPF.Perform();

    if (aPF.HasErrors())
    {
      continue;
    }

    const BOPDS_DS &aDS = aPF.DS();
    if (aDS.NbShapes() == aDS.NbSourceShapes())
    {
      //No new shapes have been created
      continue;
    }

    BOPAlgo_Builder aBAB(NCollection_BaseAllocator::CommonBaseAllocator());
    TopTools_ListIteratorOfListOfShape aBItr(aLGF);
    for (; aBItr.More(); aBItr.Next())
    {
      const TopoDS_Shape &aSh = aBItr.Value();
      aBAB.AddArgument(aSh);
    }

    aBAB.SetRunParallel(Standard_True);
    aBAB.SetNonDestructive(Standard_True);
    aBAB.PerformWithFiller(aPF);
    if (aBAB.HasErrors())
    {
      continue;
    }

#if 0
    // This fragment requires fixing the issue #29656
    TopTools_MapOfShape aMM;
    TopExp_Explorer anExpEB(aBAB.Shape(), TopAbs_EDGE);
    for (; anExpEB.More(); anExpEB.Next())
    {
      const TopoDS_Edge anEE = TopoDS::Edge(anExpEB.Current());
      if (!aMM.Add(anEE))
        continue;

      aLE.Append(anEE);
    }
#else
    for (aBItr.Init(aLGF); aBItr.More(); aBItr.Next())
    {
      const TopoDS_Edge &aSh = TopoDS::Edge(aBItr.Value());
      const TopTools_ListOfShape &aLM = aBAB.Modified(aSh);
      if (aLM.IsEmpty() || BRep_Tool::Degenerated(aSh))
      {
        aLE.Append(aSh);
        continue;
      }

      TopTools_ListIteratorOfListOfShape anItLM(aLM);
      for (; anItLM.More(); anItLM.Next())
      {
        const TopoDS_Edge &anEM = TopoDS::Edge(anItLM.Value());
        aLE.Append(anEM);
      }
    }
#endif

    isSplit = Standard_True;
    InsertEDegenerated(aF, aLE);
  }

  if (!isSplit)
  {
    theListOfFaces.Append(theF);
    return Standard_False;
  }

  RebuildFaces(aLE, theF, theListOfSplits);

  TopTools_ListIteratorOfListOfShape anItrS(theListOfSplits);
  for (; anItrS.More(); anItrS.Next())
  {
    const TopoDS_Face &aF = TopoDS::Face(anItrS.Value());
    theListOfFaces.Append(aF);
  }

  return Standard_True;
}
