// Created on: 2013-09-16
// Copyright (c) 2013-2014 OPEN CASCADE SAS
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

#include <Font_BRepFont.hxx>

#include <BRep_Tool.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <Font_FTLibrary.hxx>
#include <Font_TextFormatter.hxx>
#include <GCE2d_MakeSegment.hxx>
#include <GC_MakeSegment.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_Plane.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2d_Line.hxx>
#include <GeomAPI.hxx>
#include <GeomAdaptor_HSurface.hxx>
#include <GeomLib.hxx>
#include <gp_Pln.hxx>
#include <TColGeom2d_HSequenceOfBoundedCurve.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>
#include <TopTools_DataMapOfShapeSequenceOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

IMPLEMENT_STANDARD_RTTIEXT(Font_BRepFont,Font_FTFont)

namespace
{
  // pre-defined font rendering options
  static const unsigned int THE_FONT_SIZE      = 72;
  static const unsigned int THE_RESOLUTION_DPI = 4800;

  // compute scaling factor for specified font size
  inline Standard_Real getScale (const Standard_Real theSize)
  {
    return theSize / Standard_Real(THE_FONT_SIZE) * 72.0 / Standard_Real(THE_RESOLUTION_DPI);
  }

  //! Auxiliary method to convert FT_Vector to gp_XY
  static gp_XY readFTVec (const FT_Vector& theVec,
                          const Standard_Real theScaleUnits)
  {
    return gp_XY (theScaleUnits * Standard_Real(theVec.x) / 64.0, theScaleUnits * Standard_Real(theVec.y) / 64.0);
  }

  //! Auxiliary method for classification wire theW2 with respect to wire theW1
  static TopAbs_State classifyWW(
    const TopoDS_Wire& theW1, const TopoDS_Wire& theW2, const TopoDS_Face& theF)
  {
    TopAbs_State aRes = TopAbs_UNKNOWN;

    TopoDS_Face aF = TopoDS::Face(theF.EmptyCopied());
    aF.Orientation(TopAbs_FORWARD);
    BRep_Builder aB;
    aB.Add(aF, theW1);
    BRepTopAdaptor_FClass2d clas(aF, ::Precision::PConfusion());
    for (TopoDS_Iterator ew(theW2); ew.More(); ew.Next())
    {
      TopoDS_Edge ed = TopoDS::Edge(ew.Value());
      Standard_Real cf, cl;
      Handle(Geom2d_Curve) cw = BRep_Tool::CurveOnSurface(ed, theF, cf, cl);
      if (cw.IsNull()) {
        continue;
      }
      gp_Pnt2d unp = cw->Value((cf + cl) / 2.);
      TopAbs_State ste = clas.Perform(unp, Standard_False);
      if (ste == TopAbs_OUT || ste == TopAbs_IN)
      {
        if (aRes == TopAbs_UNKNOWN)
        {
          aRes = ste;
        }
        else {
          if (aRes != ste)
          {
            aRes = TopAbs_UNKNOWN;
            break;
          }
        }
      }
    }

    return aRes;
  }

}

// =======================================================================
// function : Constructor
// purpose  :
// =======================================================================
Font_BRepFont::Font_BRepFont ()
: myPrecision  (Precision::Confusion()),
  myScaleUnits (1.0),
  myIsCompositeCurve (Standard_False),
  my3Poles     (1, 3),
  my4Poles     (1, 4)
{
  init();
}

// =======================================================================
// function : init
// purpose  :
// =======================================================================
void Font_BRepFont::init()
{
  mySurface        = new Geom_Plane (gp_Pln (gp::XOY()));
  myCurve2dAdaptor = new Geom2dAdaptor_HCurve();
  Handle(Adaptor3d_HSurface) aSurfAdaptor = new GeomAdaptor_HSurface (mySurface);
  myCurvOnSurf.Load (aSurfAdaptor);
}

// =======================================================================
// function : Constructor
// purpose  :
// =======================================================================
Font_BRepFont::Font_BRepFont (const NCollection_String& theFontPath,
                              const Standard_Real       theSize)
: myPrecision  (Precision::Confusion()),
  myScaleUnits (1.0),
  myIsCompositeCurve (Standard_False),
  my3Poles     (1, 3),
  my4Poles     (1, 4)
{
  init();
  if (theSize <= myPrecision * 100.0)
  {
    return;
  }

  myScaleUnits = getScale (theSize);
  Font_FTFont::Init (theFontPath, THE_FONT_SIZE, THE_RESOLUTION_DPI);
}

// =======================================================================
// function : Constructor
// purpose  :
// =======================================================================
Font_BRepFont::Font_BRepFont (const NCollection_String& theFontName,
                              const Font_FontAspect     theFontAspect,
                              const Standard_Real       theSize)
: myPrecision  (Precision::Confusion()),
  myScaleUnits (1.0),
  myIsCompositeCurve (Standard_False),
  my3Poles     (1, 3),
  my4Poles     (1, 4)
{
  init();
  if (theSize <= myPrecision * 100.0)
  {
    return;
  }

  myScaleUnits = getScale (theSize);
  Font_FTFont::Init (theFontName, theFontAspect, THE_FONT_SIZE, THE_RESOLUTION_DPI);
}

// =======================================================================
// function : Release
// purpose  :
// =======================================================================
void Font_BRepFont::Release()
{
  myCache.Clear();
  Font_FTFont::Release();
}

// =======================================================================
// function : SetCompositeCurveMode
// purpose  :
// =======================================================================
void Font_BRepFont::SetCompositeCurveMode (const Standard_Boolean theToConcatenate)
{
  if (myIsCompositeCurve != theToConcatenate)
  {
    myIsCompositeCurve = theToConcatenate;
    myCache.Clear();
  }
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
bool Font_BRepFont::Init (const NCollection_String& theFontPath,
                          const Standard_Real       theSize)
{
  if (theSize <= myPrecision * 100.0)
  {
    return false;
  }

  myScaleUnits = getScale (theSize);
  return Font_FTFont::Init (theFontPath, THE_FONT_SIZE, THE_RESOLUTION_DPI);
}

// =======================================================================
// function : Init
// purpose  :
// =======================================================================
bool Font_BRepFont::Init (const NCollection_String& theFontName,
                          const Font_FontAspect     theFontAspect,
                          const Standard_Real       theSize)
{
  if (theSize <= myPrecision * 100.0)
  {
    return false;
  }

  myScaleUnits = getScale (theSize);
  return Font_FTFont::Init (theFontName, theFontAspect, THE_FONT_SIZE, THE_RESOLUTION_DPI);
}

// =======================================================================
// function : RenderGlyph
// purpose  :
// =======================================================================
TopoDS_Shape Font_BRepFont::RenderGlyph (const Standard_Utf32Char& theChar)
{
  TopoDS_Shape aShape;
  Standard_Mutex::Sentry aSentry (myMutex);
  renderGlyph (theChar, aShape);
  return aShape;
}

// =======================================================================
// function : to3d
// purpose  :
// =======================================================================
bool Font_BRepFont::to3d (const Handle(Geom2d_Curve)& theCurve2d,
                          const GeomAbs_Shape        theContinuity,
                          Handle(Geom_Curve)&        theCurve3d)
{
  Standard_Real aMaxDeviation   = 0.0;
  Standard_Real anAverDeviation = 0.0;
  myCurve2dAdaptor->ChangeCurve2d().Load (theCurve2d);
  const Handle(Adaptor2d_HCurve2d)& aCurve = myCurve2dAdaptor; // to avoid ambiguity
  myCurvOnSurf.Load (aCurve);
  GeomLib::BuildCurve3d (myPrecision, myCurvOnSurf,
                         myCurve2dAdaptor->FirstParameter(), myCurve2dAdaptor->LastParameter(),
                         theCurve3d, aMaxDeviation, anAverDeviation, theContinuity);
  return !theCurve3d.IsNull();
}


// =======================================================================
// function : buildFaces
// purpose  : auxilary for renderGlyph
// =======================================================================
Standard_Boolean Font_BRepFont::buildFaces(
  const TopTools_SequenceOfShape& theWires,
  TopoDS_Shape& theRes)
{
  // split to few faces (if it is needed) and update orientation of wires
  TopoDS_Shape aRes;
  // classify wires
  //Font_DataMapOfShapeMapOfShape aMapOutInts;
  TopTools_DataMapOfShapeSequenceOfShape aMapOutInts;
  TopTools_DataMapOfShapeInteger aMapNbOuts;
  Standard_Integer aNbWires = theWires.Length();
  TopoDS_Face aF;
  myBuilder.MakeFace(aF, mySurface, myPrecision);
  for (Standard_Integer i = 1; i <= aNbWires; i++)
  {
    TopoDS_Wire aW1 = TopoDS::Wire(theWires.Value(i));
    if (!aMapNbOuts.IsBound(aW1))
    {
      Standard_Integer aNbOuts = 0;
      aMapNbOuts.Bind(aW1, aNbOuts);
    }
    TopTools_SequenceOfShape anIntWs;
    for (Standard_Integer j = 1; j <= aNbWires; j++)
    {
      if (i == j) continue;
      TopoDS_Wire aW2 = TopoDS::Wire(theWires.Value(j));
      TopAbs_State aClass = classifyWW(aW1, aW2, aF);
      if (aClass == TopAbs_IN)
      {
        anIntWs.Append(aW2);
        if (aMapNbOuts.IsBound(aW2))
        {
          aMapNbOuts.ChangeFind(aW2)++;
        }
        else
        {
          Standard_Integer aNbOuts = 1;
          aMapNbOuts.Bind(aW2, aNbOuts);
        }
      }
    }
    aMapOutInts.Bind(aW1, anIntWs);
  }
  // check out wires and remove "not out" wires from maps
  for (TopTools_DataMapIteratorOfDataMapOfShapeInteger itO(aMapNbOuts); itO.More(); itO.Next())
  {
    Standard_Integer aTmp = itO.Value() % 2;
    if (aTmp > 0)
    {
      // not out wire
      aMapOutInts.UnBind(itO.Key());
    }
  }
  // create faces for out wires
  TopTools_MapOfShape anUsedShapes;
  Standard_Integer aNbFaces = 0;
  TopoDS_Compound aFaceComp;
  myBuilder.MakeCompound(aFaceComp);
  while (aMapOutInts.Extent() > 0)
  {
    // find out wire with max number of outs 
    TopoDS_Shape aW;
    Standard_Integer aMaxNbOuts = -1;
    for (TopTools_DataMapIteratorOfDataMapOfShapeSequenceOfShape itMOI(aMapOutInts); itMOI.More(); itMOI.Next())
    {
      TopoDS_Shape aKey = itMOI.Key();
      Standard_Integer aNbOuts = aMapNbOuts.Find(aKey);
      if (aNbOuts > aMaxNbOuts)
      {
        aMaxNbOuts = aNbOuts;
        aW = aKey;
      }
    }
    // create face for selected wire
    TopoDS_Face aNewF;
    myBuilder.MakeFace(aNewF, mySurface, myPrecision);
    myBuilder.Add(aNewF, aW);
    anUsedShapes.Add(aW);
    const TopTools_SequenceOfShape& anIns = aMapOutInts.Find(aW);
    for (Standard_Integer i=1; i<= anIns.Length(); i++)
    {
      TopoDS_Wire aWin = TopoDS::Wire(anIns.Value(i));
      if (anUsedShapes.Contains(aWin))
      {
        continue;
      }
      aWin.Reverse();
      myBuilder.Add(aNewF, aWin);
      anUsedShapes.Add(aWin);
    }
    aNbFaces++;
    myBuilder.Add(aFaceComp, aNewF);
    aMapOutInts.UnBind(aW);
  }
  if (aNbFaces > 0)
  {
    if (aNbFaces > 1)
    {
      theRes = aFaceComp;
    }
    else
    {
      TopoDS_Iterator itSh(aFaceComp);
      theRes = itSh.Value();
    }
  }
  return (aNbFaces > 0);
}


// =======================================================================
// function : renderGlyph
// purpose  :
// =======================================================================
Standard_Boolean Font_BRepFont::renderGlyph (const Standard_Utf32Char theChar,
                                             TopoDS_Shape&            theShape)
{
  theShape.Nullify();
  if (!loadGlyph (theChar)
   || myFTFace->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
  {
    return Standard_False;
  }
  else if (myCache.Find (theChar, theShape))
  {
    return !theShape.IsNull();
  }

  FT_Outline& anOutline = myFTFace->glyph->outline;

  if (!anOutline.n_contours)
    return Standard_False;

  TopLoc_Location aLoc;
  TopTools_SequenceOfShape aWires;
  //TopoDS_Compound aFaceCompDraft;

  // Get orientation is useless since it doesn't retrieve any in-font information and just computes orientation.
  // Because it fails in some cases - leave this to ShapeFix.
  //const FT_Orientation anOrient = FT_Outline_Get_Orientation (&anOutline);
  for (short aContour = 0, aStartIndex = 0; aContour < anOutline.n_contours; ++aContour)
  {
    const FT_Vector* aPntList = &anOutline.points[aStartIndex];
    const char* aTags      = &anOutline.tags[aStartIndex];
    const short anEndIndex = anOutline.contours[aContour];
    const short aPntsNb    = (anEndIndex - aStartIndex) + 1;
    aStartIndex = anEndIndex + 1;
    if (aPntsNb < 3)
    {
      // closed contour can not be constructed from < 3 points
      continue;
    }

    BRepBuilderAPI_MakeWire aWireMaker;

    gp_XY aPntPrev;
    gp_XY aPntCurr = readFTVec (aPntList[aPntsNb - 1], myScaleUnits);
    gp_XY aPntNext = readFTVec (aPntList[0], myScaleUnits);

    Standard_Integer aLinePnts = (FT_CURVE_TAG(aTags[aPntsNb - 1]) == FT_Curve_Tag_On) ? 1 : 0;
    gp_XY aPntLine1 = aPntCurr;

    // see http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-6.html
    // for a full description of FreeType tags.
    for (short aPntId = 0; aPntId < aPntsNb; ++aPntId)
    {
      aPntPrev = aPntCurr;
      aPntCurr = aPntNext;
      aPntNext = readFTVec (aPntList[(aPntId + 1) % aPntsNb], myScaleUnits);

      // process tags
      if (FT_CURVE_TAG(aTags[aPntId]) == FT_Curve_Tag_On)
      {
        if (aLinePnts < 1)
        {
          aPntLine1 = aPntCurr;
          aLinePnts = 1;
          continue;
        }

        const gp_XY         aDirVec  = aPntCurr - aPntLine1;
        const Standard_Real aLen     = aDirVec.Modulus();
        if (aLen <= myPrecision)
        {
          aPntLine1 = aPntCurr;
          aLinePnts = 1;
          continue;
        }

        if (myIsCompositeCurve)
        {
          Handle(Geom2d_TrimmedCurve) aLine = GCE2d_MakeSegment (gp_Pnt2d (aPntLine1), gp_Pnt2d (aPntCurr));
          myConcatMaker.Add (aLine, myPrecision);
        }
        else
        {
          Handle(Geom_Curve)  aCurve3d;
          Handle(Geom2d_Line) aCurve2d = new Geom2d_Line (gp_Pnt2d (aPntLine1), gp_Dir2d (aDirVec));
          if (to3d (aCurve2d, GeomAbs_C1, aCurve3d))
          {
            TopoDS_Edge anEdge = BRepLib_MakeEdge (aCurve3d, 0.0, aLen);
            myBuilder.UpdateEdge (anEdge, aCurve2d, mySurface, aLoc, myPrecision);
            aWireMaker.Add (anEdge);
          }
        }
        aPntLine1 = aPntCurr;
      }
      else if (FT_CURVE_TAG(aTags[aPntId]) == FT_Curve_Tag_Conic)
      {
        aLinePnts = 0;
        gp_XY aPntPrev2 = aPntPrev;
        gp_XY aPntNext2 = aPntNext;

        // previous point is either the real previous point (an "on" point),
        // or the midpoint between the current one and the previous "conic off" point
        if (FT_CURVE_TAG(aTags[(aPntId - 1 + aPntsNb) % aPntsNb]) == FT_Curve_Tag_Conic)
        {
          aPntPrev2 = (aPntCurr + aPntPrev) * 0.5;
        }

        // next point is either the real next point or the midpoint
        if (FT_CURVE_TAG(aTags[(aPntId + 1) % aPntsNb]) == FT_Curve_Tag_Conic)
        {
          aPntNext2 = (aPntCurr + aPntNext) * 0.5;
        }

        my3Poles.SetValue (1, aPntPrev2);
        my3Poles.SetValue (2, aPntCurr);
        my3Poles.SetValue (3, aPntNext2);
        Handle(Geom2d_BezierCurve) aBezierArc = new Geom2d_BezierCurve (my3Poles);
        if (myIsCompositeCurve)
        {
          myConcatMaker.Add (aBezierArc, myPrecision);
        }
        else
        {
          Handle(Geom_Curve) aCurve3d;
          if (to3d (aBezierArc, GeomAbs_C1, aCurve3d))
          {
            TopoDS_Edge anEdge = BRepLib_MakeEdge (aCurve3d);
            myBuilder.UpdateEdge (anEdge, aBezierArc, mySurface, aLoc, myPrecision);
            aWireMaker.Add (anEdge);
          }
        }
      }
      else if (FT_CURVE_TAG(aTags[aPntId])                 == FT_Curve_Tag_Cubic
            && FT_CURVE_TAG(aTags[(aPntId + 1) % aPntsNb]) == FT_Curve_Tag_Cubic)
      {
        aLinePnts = 0;
        my4Poles.SetValue (1, aPntPrev);
        my4Poles.SetValue (2, aPntCurr);
        my4Poles.SetValue (3, aPntNext);
        my4Poles.SetValue (4, gp_Pnt2d(readFTVec (aPntList[(aPntId + 2) % aPntsNb], myScaleUnits)));
        Handle(Geom2d_BezierCurve) aBezier = new Geom2d_BezierCurve (my4Poles);
        if (myIsCompositeCurve)
        {
          myConcatMaker.Add (aBezier, myPrecision);
        }
        else
        {
          Handle(Geom_Curve) aCurve3d;
          if (to3d (aBezier, GeomAbs_C1, aCurve3d))
          {
            TopoDS_Edge anEdge = BRepLib_MakeEdge (aCurve3d);
            myBuilder.UpdateEdge (anEdge, aBezier, mySurface, aLoc, myPrecision);
            aWireMaker.Add (anEdge);
          }
        }
      }
    }

    if (myIsCompositeCurve)
    {
      Handle(Geom2d_BSplineCurve) aDraft2d = myConcatMaker.BSplineCurve();
      if (aDraft2d.IsNull())
      {
        continue;
      }

      const gp_Pnt2d aFirstPnt = aDraft2d->StartPoint();
      const gp_Pnt2d aLastPnt  = aDraft2d->EndPoint();
      if (!aFirstPnt.IsEqual (aLastPnt, myPrecision))
      {
        Handle(Geom2d_TrimmedCurve) aLine = GCE2d_MakeSegment (aLastPnt, aFirstPnt);
        myConcatMaker.Add (aLine, myPrecision);
      }

      Handle(Geom2d_BSplineCurve) aCurve2d = myConcatMaker.BSplineCurve();
      Handle(Geom_Curve)          aCurve3d;
      if (to3d (aCurve2d, GeomAbs_C0, aCurve3d))
      {
        TopoDS_Edge anEdge = BRepLib_MakeEdge (aCurve3d);
        myBuilder.UpdateEdge (anEdge, aCurve2d, mySurface, aLoc, myPrecision);
        aWireMaker.Add (anEdge);
      }
      myConcatMaker.Clear();
    }
    else
    {
      if (!aWireMaker.IsDone())
      {
        continue;
      }

      TopoDS_Vertex aFirstV, aLastV;
      TopExp::Vertices (aWireMaker.Wire(), aFirstV, aLastV);
      gp_Pnt aFirstPoint = BRep_Tool::Pnt (aFirstV);
      gp_Pnt aLastPoint  = BRep_Tool::Pnt (aLastV);
      if (!aFirstPoint.IsEqual (aLastPoint, myPrecision))
      {
        aWireMaker.Add (BRepLib_MakeEdge (aFirstV, aLastV));
      }
    }

    if (!aWireMaker.IsDone())
    {
      continue;
    }

    TopoDS_Wire aWireDraft = aWireMaker.Wire();

    // collect all wires and set CCW orientation
    TopoDS_Face aF;
    myBuilder.MakeFace(aF, mySurface, myPrecision);
    myBuilder.Add(aF, aWireDraft);
    BRepTopAdaptor_FClass2d clas(aF, ::Precision::PConfusion());
    TopAbs_State staout = clas.PerformInfinitePoint();
    if (staout != TopAbs_OUT) {
      // need to reverse
      aWireDraft.Reverse();
    }
    aWires.Append(aWireDraft);
  }

  if (!aWires.IsEmpty())
  {
    buildFaces(aWires, theShape);
  }

  myCache.Bind (theChar, theShape);
  return !theShape.IsNull();
}
