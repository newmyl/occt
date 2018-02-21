// Created on: 2018-03-26
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

#include <memory>

#include <GeomliteTest.hxx>

#include <Adaptor2d_Curve2d.hxx>
#include <Adaptor3d_Curve.hxx>
#include <Adaptor3d_Surface.hxx>
#include <Adaptor2d_HCurve2d.hxx>
#include <Adaptor3d_HCurve.hxx>
#include <Adaptor3d_HSurface.hxx>
#include <Draw.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <DrawTrSurf.hxx>
#include <DBRep.hxx>
#include <GeomAdaptor_HCurve.hxx>
#include <GeomAdaptor_HSurface.hxx>
#include <Geom2dAdaptor_HCurve.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRepAdaptor_HCurve.hxx>
#include <BRepAdaptor_HSurface.hxx>
#include <BRepAdaptor_HCurve2d.hxx>


namespace AdaptorTypes
{
  static Handle(Adaptor2d_HCurve2d) anAHC2d;
  static Handle(Adaptor3d_HCurve) anAHC3d;
  static Handle(Adaptor3d_HSurface) anAHS3d;
}

//=======================================================================
//function : NulifyAdaptors
//purpose  : 
//=======================================================================
static void NulifyAdaptors()
{
  AdaptorTypes::anAHC2d.Nullify();
  AdaptorTypes::anAHC3d.Nullify();
  AdaptorTypes::anAHS3d.Nullify();
}

//=======================================================================
//function : MakeGeneralAdaptor
//purpose  : 
//=======================================================================
static Standard_Integer Nulify(Draw_Interpretor& theDI,
                               Standard_Integer /*theNArg*/,
                               const char** /*theArgVal*/)
{
  try
  {
    NulifyAdaptors();
  }
  catch (Standard_Failure)
  {
    theDI << "Error: not all adapters have been nullified.\n";
  }

  theDI << "All adapters have been nullified successful.\n";

  return 0;
}

//=======================================================================
//function : MakeGeneralAdaptor
//purpose  : 
//=======================================================================
static Standard_Integer MakeGeneralAdaptor(Draw_Interpretor& theDI,
                                           Standard_Integer theNArg,
                                           const char** theArgVal)
{
  if (theNArg < 2)
  {
    theDI << "Use: " << theArgVal[0] << " <arg>\n";
    theDI << "  * -- <arg> - argument for adapter creation. It can be:\n";
    theDI << "    - 3D/2D-curve;\n"
             "    - Surface;\n"
             "    - Edge;\n"
             "    - Face;\n"
             "    - Compound of edge+face (the edge must have 2D-curve on the face)\n";
    return 1;
  }

  TopoDS_Edge anE;
  TopoDS_Face aF;
  const Handle(Geom_Geometry) aG = DrawTrSurf::Get(theArgVal[1]);
  const Handle(Geom2d_Curve) aC2d = DrawTrSurf::GetCurve2d(theArgVal[1]);

  if (aG.IsNull() && aC2d.IsNull())
  {
    TopoDS_Shape aShape = DBRep::Get(theArgVal[1]);
    if (aShape.IsNull())
    {
      theDI << "Error: Please set arguments (curve, surface or shape) to adapter creation.\n";
      return 1;
    }

    TopExp_Explorer anExp(aShape, TopAbs_FACE);
    if (anExp.More())
    {
      aF = TopoDS::Face(anExp.Current());
    }

    anExp.Init(aShape, TopAbs_EDGE, TopAbs_FACE);
    if (anExp.More())
    {
      anE = TopoDS::Edge(anExp.Current());
    }
  }

  if (!aC2d.IsNull() || (!aF.IsNull() && !anE.IsNull()))
  {
    if (!aC2d.IsNull())
    {
      AdaptorTypes::anAHC2d = new Geom2dAdaptor_HCurve(aC2d);
      if (AdaptorTypes::anAHC2d)
      {
        theDI << "OK: Adaptor2d_Curve2d has been created.\n";
      }
      else
      {
        theDI << "Error: Wrong type of adapter.\n";
        return 1;
      }
    }
    else
    {
      AdaptorTypes::anAHC2d = new BRepAdaptor_HCurve2d(BRepAdaptor_Curve2d(anE, aF));
      if (AdaptorTypes::anAHC2d)
      {
        theDI << "OK: Adaptor2d_Curve2d has been created.\n";
      }
      else
      {
        theDI << "Error: Wrong type of adapter.\n";
        return 1;
      }
    }

    return 0;
  }
  
  if (!aG.IsNull())
  {
    const Handle(Geom_Curve) aC = Handle(Geom_Curve)::DownCast(aG);
    if (!aC.IsNull())
    {
      AdaptorTypes::anAHC3d = new GeomAdaptor_HCurve(aC);
      if (AdaptorTypes::anAHC3d)
      {
        theDI << "OK: Adaptor3d_Curve has been created.\n";
      }
      else
      {
        theDI << "Error: Wrong type of adapter.\n";
        return 1;
      }
    }
    else
    {
      const Handle(Geom_Surface) aS = Handle(Geom_Surface)::DownCast(aG);
      if (!aS.IsNull())
      {
        AdaptorTypes::anAHS3d = new GeomAdaptor_HSurface(aS);
        if (AdaptorTypes::anAHS3d)
        {
          theDI << "OK: Adaptor3d_Surface has been created.\n";
        }
        else
        {
          theDI << "Error: Wrong type of adapter.\n";
          return 1;
        }
      }
      else
      {
        theDI << "Error: Please set arguments (curve, surface or shape) to adapter creation.\n";
        return 1;
      }
    }
  }
  else //if(aG.IsNull())
  {
    if (!anE.IsNull())
    {
      if (!aF.IsNull())
      {
        AdaptorTypes::anAHC3d = new BRepAdaptor_HCurve(BRepAdaptor_Curve(anE, aF));
      }
      else
      {
        AdaptorTypes::anAHC3d = new BRepAdaptor_HCurve(BRepAdaptor_Curve(anE));
      }

      if (AdaptorTypes::anAHC3d)
      {
        theDI << "OK: Adaptor3d_Curve has been created.\n";
      }
      else
      {
        theDI << "Error: Wrong type of adapter.\n";
        return 1;
      }
    }
    else
    {
      AdaptorTypes::anAHS3d = new BRepAdaptor_HSurface(BRepAdaptor_Surface(aF));
      if (AdaptorTypes::anAHS3d)
      {
        theDI << "OK: Adaptor3d_Surface has been created.\n";
      }
      else
      {
        theDI << "Error: Wrong type of adapter.\n";
        return 1;
      }
    }
  }

  return 0;
}

//=======================================================================
//function : CheckPeriodic
//purpose  : 
//=======================================================================
void CheckPeriodic(Draw_Interpretor& theDI)
{
  if (!AdaptorTypes::anAHC2d.IsNull())
  {
    if (AdaptorTypes::anAHC2d->IsPeriodic())
    {
      theDI << "Adaptor2d_Curve2d is periodic with period " << AdaptorTypes::anAHC2d->Period() << "\n";
    }
    else
    {
      theDI << "Adaptor2d_Curve2d is not periodic\n";
    }

    return;
  }

  if (!AdaptorTypes::anAHC3d.IsNull())
  {
    if (AdaptorTypes::anAHC3d->IsPeriodic())
    {
      theDI << "Adaptor3d_Curve is periodic with period " << AdaptorTypes::anAHC3d->Period() << "\n";
    }
    else
    {
      theDI << "Adaptor3d_Curve is not periodic\n";
    }

    return;
  }

  if (AdaptorTypes::anAHS3d->IsUPeriodic())
  {
    theDI << "Adaptor3d_Surface is U-periodic with U-period " << AdaptorTypes::anAHS3d->UPeriod() << "\n";
  }
  else
  {
    theDI << "Adaptor3d_Surface is not U-periodic\n";
  }

  if (AdaptorTypes::anAHS3d->IsVPeriodic())
  {
    theDI << "Adaptor3d_Surface is U-periodic with V-period " << AdaptorTypes::anAHS3d->VPeriod() << "\n";
  }
  else
  {
    theDI << "Adaptor3d_Surface is not V-periodic\n";
  }
}

//=======================================================================
//function : CheckPeriodic
//purpose  : 
//=======================================================================
void CheckClosed(Draw_Interpretor& theDI)
{
  if (!AdaptorTypes::anAHC2d.IsNull())
  {
    if (AdaptorTypes::anAHC2d->IsClosed())
    {
      theDI << "Adaptor2d_Curve2d is closed\n";
    }
    else
    {
      theDI << "Adaptor2d_Curve2d is not closed\n";
    }

    return;
  }

  if (!AdaptorTypes::anAHC3d.IsNull())
  {
    if (AdaptorTypes::anAHC3d->IsClosed())
    {
      theDI << "Adaptor3d_Curve is closed\n";
    }
    else
    {
      theDI << "Adaptor3d_Curve is not closed\n";
    }

    return;
  }

  if (AdaptorTypes::anAHS3d->IsUClosed())
  {
    theDI << "Adaptor3d_Surface is U-closed\n";
  }
  else
  {
    theDI << "Adaptor3d_Surface is not U-closed\n";
  }

  if (AdaptorTypes::anAHS3d->IsVClosed())
  {
    theDI << "Adaptor3d_Surface is V-closed\n";
  }
  else
  {
    theDI << "Adaptor3d_Surface is not V-closed\n";
  }
}

//=======================================================================
//function : GetResolution
//purpose  : 
//=======================================================================
void GetResolution(Draw_Interpretor& theDI, Standard_Real theValue)
{
  if (!AdaptorTypes::anAHC2d.IsNull())
  {
    theDI << "Adaptor2d_Curve2d::Resolution = " << 
      AdaptorTypes::anAHC2d->Resolution(theValue) << "\n";
    return;
  }

  if (!AdaptorTypes::anAHC3d.IsNull())
  {
    theDI << "Adaptor3d_Curve::Resolution = " << 
      AdaptorTypes::anAHC3d->Resolution(theValue) << "\n";

    return;
  }

  const Standard_Real aUR = AdaptorTypes::anAHS3d->UResolution(theValue);
  const Standard_Real aVR = AdaptorTypes::anAHS3d->VResolution(theValue);

  theDI << "Adaptor3d_Surface: UResolution = " << aUR << "; VResolution = " << aVR << "\n";
}

//=======================================================================
//function : GetAdaptorProperty
//purpose  : 
//=======================================================================
static Standard_Integer GetAdaptorProperty(Draw_Interpretor& theDI,
                                           Standard_Integer theNArg,
                                           const char** theArgVal)
{
  if (theNArg == 1)
  {
    theDI << "Use: " << theArgVal[0] << " <set of properties>\n";
    theDI << "  -isperiodic - Returns IsPeriodic() property and the period;\n";
    theDI << "  -isclosed - Returns Adaptor::IsClosed();\n";
    theDI << "  -resolution <Value> - Returns a value of resolution;\n";

    return 1;
  }

  if (AdaptorTypes::anAHC2d.IsNull() &&
      AdaptorTypes::anAHC3d.IsNull() &&
      AdaptorTypes::anAHS3d.IsNull())
  {
    theDI << "Adapter is not initialized. Use \"makeadaptor\"-DRAW-command.\n";
    return 1;
  }

  for (Standard_Integer i = 1; i < theNArg; i++)
  {
    if (theArgVal[i][0] != '-')
    {
      theDI << "Error: " << theArgVal[i] << " is not an option\n";
      return 1;
    }

    Standard_Boolean isFound = Standard_False;

    if (!strcmp("-isperiodic", theArgVal[i]))
    {
      CheckPeriodic(theDI);
      isFound = Standard_True;
    }

    if (!strcmp("-isclosed", theArgVal[i]))
    {
      CheckClosed(theDI);
      isFound = Standard_True;
    }

    if (!strcmp("-resolution", theArgVal[i]))
    {
      const Standard_Real aT = Draw::Atof(theArgVal[++i]);
      GetResolution(theDI, aT);
      isFound = Standard_True;
    }

    if (isFound)
      continue;

    theDI << "Error: Unknown option " << theArgVal[i] << "\n";
    return 1;
  }

  return 0;
}

//=======================================================================
//function : AdaptorCommands
//purpose  : 
//=======================================================================
void  GeomliteTest::AdaptorCommands(Draw_Interpretor& theCommands)
{
  static Standard_Boolean loaded = Standard_False;
  if (loaded) return;
  loaded = Standard_True;

  const char* g;

  g = "Work with adapters";

  theCommands.Add("makeadaptor",
                  "use \"makeadaptor\" w/o any arguments to get help",
                  __FILE__,
                  MakeGeneralAdaptor, g);

  theCommands.Add("nulifyadaptors",
                  "Use: nulifyadaptors",
                  __FILE__,
                  Nulify, g);

  theCommands.Add("adaptorproperties",
                  "Use: \"adaptorproperties\" <set of properties>",
                  __FILE__,
                  GetAdaptorProperty, g);

}



