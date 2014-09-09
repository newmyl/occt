// File:	HelixTest_HelixCommands.cxx

#include <HelixTest.ixx>
#include <Draw_Interpretor.hxx>
#include <HelixGeom_HelixCurve.hxx>

#include <Draw_Viewer.hxx>
#include <Draw_Marker3D.hxx>
#include <Draw_Segment3D.hxx>
#include <Draw_Color.hxx>
#include <HelixGeom_Tools.hxx>

#include <DrawTrSurf.hxx>
#include <TCollection_AsciiString.hxx>
#include <HelixGeom_BuilderHelixCoil.hxx>
#include <TColGeom_SequenceOfCurve.hxx>
#include <HelixGeom_BuilderHelix.hxx>

#include <Geom_BSplineCurve.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>

#include <gp_Pln.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <HelixBRep_BuilderHelix.hxx>
#include <DBRep.hxx>
#include <TopoDS_Shape.hxx>
#include <gp.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAdaptor_HCurve.hxx>
#include <TColGeom_HArray1OfCurve.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfBoolean.hxx>


//
Standard_IMPORT Draw_Viewer dout;
//
static gp_Ax3 theHelixAxis(gp_Pnt(0., 0., 0), gp_Dir(0., 0., 1.), gp_Dir(1., 0., 0.));

static Standard_Integer comphelix   (Draw_Interpretor& , Standard_Integer , const char ** );
static Standard_Integer helix   (Draw_Interpretor& , Standard_Integer , const char ** );
static Standard_Integer spiral   (Draw_Interpretor& , Standard_Integer , const char ** );
static Standard_Integer setaxis   (Draw_Interpretor& , Standard_Integer , const char ** );
static Standard_Integer comphelix2   (Draw_Interpretor& , Standard_Integer , const char ** );
static Standard_Integer helix2   (Draw_Interpretor& , Standard_Integer , const char ** );
static Standard_Integer spiral2   (Draw_Interpretor& , Standard_Integer , const char ** );
//=======================================================================
//function : HelixTest::HelixCommands
//purpose  : 
//=======================================================================
  void HelixTest::HelixCommands(Draw_Interpretor& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done) return;
  done = Standard_True;
  // Chapters name
  const char* g = "Helix commands";
  // Commands
  theCommands.Add("setaxis", "setaxis x y z Nx Ny Nz Xx Xy Xz",
		  __FILE__, setaxis, g);
  theCommands.Add("comphelix", "comphelix name np D1 [Di...] H1 [Hi...] P1 [Pi...] PF1 [PFi...]",
		  __FILE__, comphelix, g);
  theCommands.Add("helix", "helix name np D1 H1 [Hi...] P1 [Pi...] PF1 [PFi...]",
		  __FILE__, helix, g);
  theCommands.Add("spiral", "spiral name np D1 D2 H1 [Hi...] P1 [Pi...] PF1 [PFi...]",
		  __FILE__, spiral, g);
  theCommands.Add("comphelix2", "comphelix2 name np D1 [Di...] P1 [Pi...] N1 [Ni...]",
		  __FILE__, comphelix2, g);
  theCommands.Add("helix2", "helix2 name np D1 P1 [Pi...] N1 [Ni...]",
		  __FILE__, helix2, g);
  theCommands.Add("spiral2", "spiral2 name np D1 D2 P1 [Pi...] N1 [Ni...]",
		  __FILE__, spiral2, g);
}

//=======================================================================
//function : setaxis
//purpose  : 
//=======================================================================
 Standard_Integer setaxis(Draw_Interpretor& di, Standard_Integer n, const char ** a)
{
  if (n < 10) {
    di << "Usage : " << a[0] << " x y z Nx Ny Nz Xx Xy Xz" <<"\n";
    di << "        " << "x y z - location" <<"\n";
    di << "        " << "Nx Ny Nz - direction" <<"\n";
    di << "        " << "Xx Xy Xz - X direction" <<"\n";
    return 1;
  }
  Standard_Real xx[9];
  Standard_Integer i;
  for(i = 0; i < 9; ++i) {
    xx[i] = atof(a[i+1]);
  }

  theHelixAxis.SetLocation(gp_Pnt(xx[0], xx[1], xx[2]));
  theHelixAxis.SetDirection(gp_Dir(xx[3], xx[4], xx[5]));
  theHelixAxis.SetXDirection(gp_Dir(xx[6], xx[7], xx[8]));

  return 0;
}
//
//=======================================================================
//function : comphelix
//purpose  : 
//=======================================================================
 Standard_Integer comphelix(Draw_Interpretor& di, Standard_Integer n, const char ** a)
{
  if (n < 8) {
    di << "Usage : " << a[0] << " name np D1 D2 [Di...] H1 [Hi...] P1 [Pi...] PF1 [PFi...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 D2 ... (must be np+1 values) - diameters" <<"\n";
    di << "        " << "H1, H2 ... (must be np values) - heights" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches or numbers of turns" <<"\n";
    di << "        " << "PF1, PF2 ... (must be np values) - 0 or 1, if PFi = 1, Pi is pitch, otherwise Pi is number of turns" <<"\n";
    return 1;
  }
  //
  Standard_Integer i, aNb, ic;
  HelixBRep_BuilderHelix aBH;
  //
  aNb = atoi(a[2]);
  if (n != 3 + (aNb+1) + aNb*3) {
    di << "Usage : " << a[0] << " name np D1 D2 [Di...] H1 [Hi...] P1 [Pi...] PF1 [PFi...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 D2 ... (must be np+1 values) - diameters" <<"\n";
    di << "        " << "H1, H2 ... (must be np values) - heights" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches or numbers of turns" <<"\n";
    di << "        " << "PF1, PF2 ... (must be np values) - 0 or 1, if PFi = 1, Pi is pitch, otherwise Pi is number of turns" <<"\n";
    return 1;
  }
  //

  TColStd_Array1OfReal aDiams(1, aNb + 1);
  TColStd_Array1OfReal aHeights(1, aNb);
  TColStd_Array1OfReal aPitches(1, aNb);
  TColStd_Array1OfBoolean bIsPitches(1, aNb);

  ic = 3;
  for(i = 1; i <= aNb+1; ++i) {
    aDiams(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aHeights(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aPitches(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    bIsPitches(i) = atoi(a[ic]);
    ++ic;
  }
  //

  aBH.SetParameters(theHelixAxis, aDiams, aHeights, aPitches, bIsPitches);

  aBH.Perform();

  if(aBH.ErrorStatus() == 0) {
    Standard_Real aMaxError=aBH.ToleranceReached();
    di << "WarningStatus = " << aBH.WarningStatus() <<"\n";
    di << "ToleranceReached = " << aMaxError <<"\n";
  //
    const TopoDS_Shape& aW=aBH.Shape();
    DBRep::Set(a[1], aW);
  }
  else {
    di << "ErrorStatus = " << aBH.ErrorStatus() << "\n";
  }
    
  return 0;
}

//=======================================================================
//function : helix
//purpose  : 
//=======================================================================
 Standard_Integer helix(Draw_Interpretor& di, Standard_Integer n, const char ** a)
{
  if (n < 7) {
    di << "Usage : " << a[0] << " name np D1 H1 [Hi...] P1 [Pi...] PF1 [PFi...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 - diameter" <<"\n";
    di << "        " << "H1, H2 ... (must be np values) - heights" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches or numbers of turns" <<"\n";
    di << "        " << "PF1, PF2 ... (must be np values) - 0 or 1, if PFi = 1, Pi is pitch, otherwise Pi is number of turns" <<"\n";
    return 1;
  }
  //
  Standard_Integer i, aNb, ic;
  HelixBRep_BuilderHelix aBH;
  //
  aNb = atoi(a[2]);
  if (n != 3 + 1 + aNb*3) {
    di << "Usage : " << a[0] << " name np D1 H1 [Hi...] P1 [Pi...] PF1 [PFi...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 - diameter" <<"\n";
    di << "        " << "H1, H2 ... (must be np values) - heights" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches or numbers of turns" <<"\n";
    di << "        " << "PF1, PF2 ... (must be np values) - 0 or 1, if PFi = 1, Pi is pitch, otherwise Pi is number of turns" <<"\n";
    return 1;
  }
  //

  TColStd_Array1OfReal aDiams(1, 1);
  TColStd_Array1OfReal aHeights(1, aNb);
  TColStd_Array1OfReal aPitches(1, aNb);
  TColStd_Array1OfBoolean bIsPitches(1, aNb);

  ic = 3;
  for(i = 1; i <= 1; ++i) {
    aDiams(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aHeights(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aPitches(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    bIsPitches(i) = atoi(a[ic]);
    ++ic;
  }
  //

  aBH.SetParameters(theHelixAxis, aDiams(1), aHeights, aPitches, bIsPitches);

  aBH.Perform();

  if(aBH.ErrorStatus() == 0) {
    Standard_Real aMaxError=aBH.ToleranceReached();
    di << "WarningStatus = " << aBH.WarningStatus() <<"\n";
    di << "ToleranceReached = " << aMaxError <<"\n";
  //
    const TopoDS_Shape& aW=aBH.Shape();
    DBRep::Set(a[1], aW);
  }
  else {
    di << "ErrorStatus = " << aBH.ErrorStatus() << "\n";
  }
    
  return 0;
}

//=======================================================================
//function : spiral
//purpose  : 
//=======================================================================
 Standard_Integer spiral(Draw_Interpretor& di, Standard_Integer n, const char ** a)
{
  if (n < 8) {
    di << "Usage : " << a[0] << " name np D1 D2 H1 [Hi...] P1 [Pi...] PF1 [PFi...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 D2 - first and last diameters" <<"\n";
    di << "        " << "H1, H2 ... (must be np values) - heights" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches or numbers of turns" <<"\n";
    di << "        " << "PF1, PF2 ... (must be np values) - 0 or 1, if PFi = 1, Pi is pitch, otherwise Pi is number of turns" <<"\n";
    return 1;
  }
  //
  Standard_Integer i, aNb, ic;
  HelixBRep_BuilderHelix aBH;
  //
  aNb = atoi(a[2]);
  if (n != 3 + 2 + aNb*3) {
    di << "Usage : " << a[0] << " name np D1 D2 H1 [Hi...] P1 [Pi...] PF1 [PFi...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 D2 - first and last diameters" <<"\n";
    di << "        " << "H1, H2 ... (must be np values) - heights" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches or numbers of turns" <<"\n";
    di << "        " << "PF1, PF2 ... (must be np values) - 0 or 1, if PFi = 1, Pi is pitch, otherwise Pi is number of turns" <<"\n";
    return 1;
  }
  //

  TColStd_Array1OfReal aDiams(1, 2);
  TColStd_Array1OfReal aHeights(1, aNb);
  TColStd_Array1OfReal aPitches(1, aNb);
  TColStd_Array1OfBoolean bIsPitches(1, aNb);

  ic = 3;
  for(i = 1; i <= 2; ++i) {
    aDiams(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aHeights(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aPitches(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    bIsPitches(i) = atoi(a[ic]);
    ++ic;
  }
  //

  aBH.SetParameters(theHelixAxis, aDiams(1), aDiams(2), aHeights, aPitches, bIsPitches);

  aBH.Perform();

  if(aBH.ErrorStatus() == 0) {
    Standard_Real aMaxError=aBH.ToleranceReached();
    di << "WarningStatus = " << aBH.WarningStatus() <<"\n";
    di << "ToleranceReached = " << aMaxError <<"\n";
  //
    const TopoDS_Shape& aW=aBH.Shape();
    DBRep::Set(a[1], aW);
  }
  else {
    di << "ErrorStatus = " << aBH.ErrorStatus() << "\n";
  }

  return 0;
}

//=======================================================================
//function : comphelix2
//purpose  : 
//=======================================================================
 Standard_Integer comphelix2(Draw_Interpretor& di, Standard_Integer n, const char ** a)
{
  if (n < 7) {
    di << "Usage : " << a[0] << " name np D1 D2 [Di...] P1 [Pi...] N1 [Ni...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 D2, ... (must be np+1 values) - diameters" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches" <<"\n";
    di << "        " << "N1, N2, ...  (must be np values) - numbers of turns" <<"\n";
    return 1;
  }
  //
  Standard_Integer i, aNb, ic;
  HelixBRep_BuilderHelix aBH;
  //
  aNb = atoi(a[2]);
  if (n != 3 + (aNb+1) + aNb*2) {
    di << "Usage : " << a[0] << " name np D1 D2 [Di...] P1 [Pi...] N1 [Ni...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 D2, ... (must be np+1 values) - diameters" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches" <<"\n";
    di << "        " << "N1, N2, ...  (must be np values) - numbers of turns" <<"\n";
    return 1;
  }
  //

  TColStd_Array1OfReal aDiams(1, aNb + 1);
  TColStd_Array1OfReal aPitches(1, aNb);
  TColStd_Array1OfReal aNbTurns(1, aNb);

  ic = 3;
  for(i = 1; i <= aNb+1; ++i) {
    aDiams(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aPitches(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aNbTurns(i) = atof(a[ic]);
    ++ic;
  }

  //

  aBH.SetParameters(theHelixAxis, aDiams, aPitches, aNbTurns);

  aBH.Perform();

  if(aBH.ErrorStatus() == 0) {
    Standard_Real aMaxError=aBH.ToleranceReached();
    di << "WarningStatus = " << aBH.WarningStatus() <<"\n";
    di << "ToleranceReached = " << aMaxError <<"\n";
  //
    const TopoDS_Shape& aW=aBH.Shape();
    DBRep::Set(a[1], aW);
  }
  else {
    di << "ErrorStatus = " << aBH.ErrorStatus() << "\n";
  }
    
  return 0;
}

//=======================================================================
//function : helix2
//purpose  : 
//=======================================================================
 Standard_Integer helix2(Draw_Interpretor& di, Standard_Integer n, const char ** a)
{
  if (n < 6) {
    di << "Usage : " << a[0] << " name np D1 P1 [Pi...] N1 [Ni...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 - diameter" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches" <<"\n";
    di << "        " << "N1, N2, ...  (must be np values) - numbers of turns" <<"\n";
    return 1;
  }
  //
  Standard_Integer i, aNb, ic;
  HelixBRep_BuilderHelix aBH;
  //
  aNb = atoi(a[2]);
  if (n != 3 + 1 + aNb*2) {
    di << "Usage : " << a[0] << " name np D1 H1 [Hi...] P1 [Pi...] PF1 [PFi...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 - diameter" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches" <<"\n";
    di << "        " << "N1, N2, ...  (must be np values) - numbers of turns" <<"\n";
    return 1;
  }
  //

  TColStd_Array1OfReal aDiams(1, 1);
  TColStd_Array1OfReal aPitches(1, aNb);
  TColStd_Array1OfReal aNbTurns(1, aNb);

  ic = 3;
  for(i = 1; i <= 1; ++i) {
    aDiams(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aPitches(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aNbTurns(i) = atof(a[ic]);
    ++ic;
  }
  //

  aBH.SetParameters(theHelixAxis, aDiams(1), aPitches, aNbTurns);

  aBH.Perform();

  if(aBH.ErrorStatus() == 0) {
    Standard_Real aMaxError=aBH.ToleranceReached();
    di << "WarningStatus = " << aBH.WarningStatus() <<"\n";
    di << "ToleranceReached = " << aMaxError <<"\n";
  //
    const TopoDS_Shape& aW=aBH.Shape();
    DBRep::Set(a[1], aW);
  }
  else {
    di << "ErrorStatus = " << aBH.ErrorStatus() << "\n";
  }
    
  return 0;
}

//=======================================================================
//function : spiral2
//purpose  : 
//=======================================================================
 Standard_Integer spiral2(Draw_Interpretor& di, Standard_Integer n, const char ** a)
{
  if (n < 7) {
    di << "Usage : " << a[0] << " name np D1 D2 P1 [Pi...] N1 [Ni...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 D2 - first and last diameters" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches" <<"\n";
    di << "        " << "N1, N2, ...  (must be np values) - numbers of turns" <<"\n";
    return 1;
  }
  //
  Standard_Integer i, aNb, ic;
  HelixBRep_BuilderHelix aBH;
  //
  aNb = atoi(a[2]);
  if (n != 3 + 2 + aNb*2) {
    di << "Usage : " << a[0] << " name np D1 D2 H1 [Hi...] P1 [Pi...] PF1 [PFi...]" <<"\n";
    di << "        " << "name - name of result" <<"\n";
    di << "        " << "np - number of helix parts" <<"\n";
    di << "        " << "D1 D2 - first and last diameters" <<"\n";
    di << "        " << "P1, P2, ...  (must be np values) - pitches" <<"\n";
    di << "        " << "N1, N2, ...  (must be np values) - numbers of turns" <<"\n";
    return 1;
  }
  //

  TColStd_Array1OfReal aDiams(1, 2);
  TColStd_Array1OfReal aPitches(1, aNb);
  TColStd_Array1OfReal aNbTurns(1, aNb);

  ic = 3;
  for(i = 1; i <= 2; ++i) {
    aDiams(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aPitches(i) = atof(a[ic]);
    ++ic;
  }

  for(i = 1; i <= aNb; ++i) {
    aNbTurns(i) = atof(a[ic]);
    ++ic;
  }
  //

  aBH.SetParameters(theHelixAxis, aDiams(1), aDiams(2), aPitches, aNbTurns);

  aBH.Perform();

  if(aBH.ErrorStatus() == 0) {
    Standard_Real aMaxError=aBH.ToleranceReached();
    di << "WarningStatus = " << aBH.WarningStatus() <<"\n";
    di << "ToleranceReached = " << aMaxError <<"\n";
  //
    const TopoDS_Shape& aW=aBH.Shape();
    DBRep::Set(a[1], aW);
  }
  else {
    di << "ErrorStatus = " << aBH.ErrorStatus() << "\n";
  }

  return 0;
}
