// Created on: 1991-07-01
// Created by: Remi LEQUETTE
// Copyright (c) 1991-1999 Matra Datavision
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

#ifndef _BRep_Builder_HeaderFile
#define _BRep_Builder_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Builder.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomAbs_Shape.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>

class Standard_NullObject;
class Standard_DomainError;
class TopoDS_Face;
class Geom_Surface;
class TopLoc_Location;
class Poly_Triangulation;
class TopoDS_Edge;
class Geom_Curve;
class Poly_Polygon3D;
class Poly_PolygonOnTriangulation;
class Geom2d_Curve;
class gp_Pnt2d;
class Poly_Polygon2D;
class TopoDS_Vertex;
class gp_Pnt;


//! A framework providing advanced tolerance control.
//! It is used to build Shapes.
//! If tolerance control is required, you are advised to:
//! 1. build a default precision for topology, using the
//! classes provided in the BRepAPI package
//! 2. update the tolerance of the resulting shape.
//! Note that only vertices, edges and faces have
//! meaningful tolerance control. The tolerance value
//! must always comply with the condition that face
//! tolerances are more restrictive than edge tolerances
//! which are more restrictive than vertex tolerances. In
//! other words: Tol(Vertex) >= Tol(Edge) >= Tol(Face).
//! Other rules in setting tolerance include:
//! - you can open up tolerance but should never restrict it
//! - an edge cannot be included within the fusion of the
//! tolerance spheres of two vertices
class BRep_Builder  : public TopoDS_Builder
{
public:

  DEFINE_STANDARD_ALLOC

  
  //! Makes an undefined Face.
    static void MakeFace (TopoDS_Face& F);
  
  //! Makes a Face with a surface.
  Standard_EXPORT static void MakeFace (TopoDS_Face& F, const Handle(Geom_Surface)& S, const Standard_Real Tol);
  
  //! Makes a Face with a surface and a location.
  Standard_EXPORT static void MakeFace (TopoDS_Face& F, const Handle(Geom_Surface)& S, const TopLoc_Location& L, const Standard_Real Tol);
  
  //! Makes a Face with a triangulation. The triangulation
  //! is in the same reference system than the TFace.
  Standard_EXPORT static void MakeFace (TopoDS_Face& F, const Handle(Poly_Triangulation)& T);
  
  //! Updates the face F using the tolerance value Tol,
  //! surface S and location Location.
  Standard_EXPORT static void UpdateFace (const TopoDS_Face& F, const Handle(Geom_Surface)& S, const TopLoc_Location& L, const Standard_Real Tol);
  
  //! Changes a  face triangulation.
  //!
  //! A null Triangulation removes the triangulation.
  Standard_EXPORT static void UpdateFace (const TopoDS_Face& F, const Handle(Poly_Triangulation)& T);
  
  //! Updates the face Tolerance.
  Standard_EXPORT static void UpdateFace (const TopoDS_Face& F, const Standard_Real Tol);
  
  //! Sets the  NaturalRestriction flag of  the face.
  Standard_EXPORT static void NaturalRestriction (const TopoDS_Face& F, const Standard_Boolean N);
  
  //! Makes an undefined Edge (no geometry).
  Standard_EXPORT static void MakeEdge (TopoDS_Edge& E);
  
  //! Makes an Edge with a curve.
    static void MakeEdge (TopoDS_Edge& E, const Handle(Geom_Curve)& C, const Standard_Real Tol);
  
  //! Makes an Edge with a curve and a location.
    static void MakeEdge (TopoDS_Edge& E, const Handle(Geom_Curve)& C, const TopLoc_Location& L, const Standard_Real Tol);
  
  //! Makes an Edge with a polygon 3d.
    static void MakeEdge (TopoDS_Edge& E, const Handle(Poly_Polygon3D)& P);
  
  //! makes an Edge polygon on Triangulation.
    static void MakeEdge (TopoDS_Edge& E, const Handle(Poly_PolygonOnTriangulation)& N, const Handle(Poly_Triangulation)& T);
  
  //! makes an Edge polygon on Triangulation.
    static void MakeEdge (TopoDS_Edge& E, const Handle(Poly_PolygonOnTriangulation)& N, const Handle(Poly_Triangulation)& T, const TopLoc_Location& L);
  
  //! Sets a 3D curve for the edge.
  //! If <C> is a null handle, remove any existing 3d curve.
    static void UpdateEdge (const TopoDS_Edge& E, const Handle(Geom_Curve)& C, const Standard_Real Tol);
  
  //! Sets a 3D curve for the edge.
  //! If <C> is a null handle, remove any existing 3d curve.
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Geom_Curve)& C, const TopLoc_Location& L, const Standard_Real Tol);
  
  //! Sets a pcurve for the edge on the face.
  //! If <C> is a null handle, remove any existing pcurve.
    static void UpdateEdge (const TopoDS_Edge& E, const Handle(Geom2d_Curve)& C, const TopoDS_Face& F, const Standard_Real Tol);
  
  //! Sets pcurves for the edge on the  closed face.  If
  //! <C1> or <C2> is a null handle, remove any existing
  //! pcurve.
    static void UpdateEdge (const TopoDS_Edge& E, const Handle(Geom2d_Curve)& C1, const Handle(Geom2d_Curve)& C2, const TopoDS_Face& F, const Standard_Real Tol);
  
  //! Sets a pcurve for the edge on the face.
  //! If <C> is a null handle, remove any existing pcurve.
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Geom2d_Curve)& C, const Handle(Geom_Surface)& S, const TopLoc_Location& L, const Standard_Real Tol);
  
  //! Sets a pcurve for the edge on the face.
  //! If <C> is a null handle, remove any existing pcurve.
  //! Sets UV bounds for curve repsentation
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Geom2d_Curve)& C, const Handle(Geom_Surface)& S, const TopLoc_Location& L, const Standard_Real Tol, const gp_Pnt2d& Pf, const gp_Pnt2d& Pl);
  
  //! Sets pcurves for the edge on the closed surface.
  //! <C1> or <C2> is a null handle, remove any existing
  //! pcurve.
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Geom2d_Curve)& C1, const Handle(Geom2d_Curve)& C2, const Handle(Geom_Surface)& S, const TopLoc_Location& L, const Standard_Real Tol);
  
  //! Sets pcurves for the edge on the closed surface.
  //! <C1> or <C2> is a null handle, remove any existing
  //! pcurve.
  //! Sets UV bounds for curve repsentation
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Geom2d_Curve)& C1, const Handle(Geom2d_Curve)& C2, const Handle(Geom_Surface)& S, const TopLoc_Location& L, const Standard_Real Tol, const gp_Pnt2d& Pf, const gp_Pnt2d& Pl);
  
  //! Changes an Edge 3D polygon.
  //! A null Polygon removes the 3d Polygon.
    static void UpdateEdge (const TopoDS_Edge& E, const Handle(Poly_Polygon3D)& P);
  
  //! Changes an Edge 3D polygon.
  //! A null Polygon removes the 3d Polygon.
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Poly_Polygon3D)& P, const TopLoc_Location& L);
  
  //! Changes an Edge polygon on Triangulation.
    static void UpdateEdge (const TopoDS_Edge& E, const Handle(Poly_PolygonOnTriangulation)& N, const Handle(Poly_Triangulation)& T);
  
  //! Changes an Edge polygon on Triangulation.
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Poly_PolygonOnTriangulation)& N, const Handle(Poly_Triangulation)& T, const TopLoc_Location& L);
  
  //! Changes an Edge polygon on Triangulation.
    static void UpdateEdge (const TopoDS_Edge& E, const Handle(Poly_PolygonOnTriangulation)& N1, const Handle(Poly_PolygonOnTriangulation)& N2, const Handle(Poly_Triangulation)& T);
  
  //! Changes an Edge polygon on Triangulation.
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Poly_PolygonOnTriangulation)& N1, const Handle(Poly_PolygonOnTriangulation)& N2, const Handle(Poly_Triangulation)& T, const TopLoc_Location& L);
  
  //! Changes Edge polygon on a face.
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Poly_Polygon2D)& P, const TopoDS_Face& S);
  
  //! Changes Edge polygon on a face.
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Poly_Polygon2D)& P, const Handle(Geom_Surface)& S, const TopLoc_Location& T);
  
  //! Changes Edge polygons on a face.
  //!
  //! A null Polygon removes the 2d Polygon.
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Poly_Polygon2D)& P1, const Handle(Poly_Polygon2D)& P2, const TopoDS_Face& S);
  
  //! Changes Edge polygons on a face.
  //!
  //! A null Polygon removes the 2d Polygon.
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Handle(Poly_Polygon2D)& P1, const Handle(Poly_Polygon2D)& P2, const Handle(Geom_Surface)& S, const TopLoc_Location& L);
  
  //! Updates the edge tolerance.
  Standard_EXPORT static void UpdateEdge (const TopoDS_Edge& E, const Standard_Real Tol);
  
  //! Sets the geometric continuity on the edge.
  Standard_EXPORT static void Continuity (const TopoDS_Edge& E, const TopoDS_Face& F1, const TopoDS_Face& F2, const GeomAbs_Shape C);
  
  //! Sets the geometric continuity on the edge.
  Standard_EXPORT static void Continuity (const TopoDS_Edge& E, const Handle(Geom_Surface)& S1, const Handle(Geom_Surface)& S2, const TopLoc_Location& L1, const TopLoc_Location& L2, const GeomAbs_Shape C);
  
  //! Sets the same parameter flag for the edge <E>.
  Standard_EXPORT static void SameParameter (const TopoDS_Edge& E, const Standard_Boolean S);
  
  //! Sets the same range flag for the edge <E>.
  Standard_EXPORT static void SameRange (const TopoDS_Edge& E, const Standard_Boolean S);
  
  //! Sets the degenerated flag for the edge <E>.
  Standard_EXPORT static void Degenerated (const TopoDS_Edge& E, const Standard_Boolean D);
  
  //! Sets the range of the 3d curve if Only3d=TRUE,
  //! otherwise sets the range to all the representations
  Standard_EXPORT static void Range (const TopoDS_Edge& E, const Standard_Real First, const Standard_Real Last, const Standard_Boolean Only3d = Standard_False);
  
  //! Sets the range  of the edge  on the pcurve on  the
  //! surface.
  Standard_EXPORT static void Range (const TopoDS_Edge& E, const Handle(Geom_Surface)& S, const TopLoc_Location& L, const Standard_Real First, const Standard_Real Last);
  
  //! Sets the range of the edge on the pcurve on the face.
    static void Range (const TopoDS_Edge& E, const TopoDS_Face& F, const Standard_Real First, const Standard_Real Last);
  
  //! Add  to <Eout>  the  geometric representations  of
  //! <Ein>.
  Standard_EXPORT static void Transfert (const TopoDS_Edge& Ein, const TopoDS_Edge& Eout);
  
  //! Makes an udefined vertex without geometry.
    static void MakeVertex (TopoDS_Vertex& V);
  
  //! Makes a vertex from a 3D point.
    static void MakeVertex (TopoDS_Vertex& V, const gp_Pnt& P, const Standard_Real Tol);
  
  //! Sets a 3D point on the vertex.
  Standard_EXPORT static void UpdateVertex (const TopoDS_Vertex& V, const gp_Pnt& P, const Standard_Real Tol);
  
  //! Sets  the parameter  for the   vertex on the  edge
  //! curves.
  Standard_EXPORT static void UpdateVertex (const TopoDS_Vertex& V, const Standard_Real P, const TopoDS_Edge& E, const Standard_Real Tol);
  
  //! Sets  the parameter  for the  vertex  on the  edge
  //! pcurve  on the face.
    static void UpdateVertex (const TopoDS_Vertex& V, const Standard_Real P, const TopoDS_Edge& E, const TopoDS_Face& F, const Standard_Real Tol);
  
  //! Sets  the parameter  for the  vertex  on the  edge
  //! pcurve  on the surface.
  Standard_EXPORT static void UpdateVertex (const TopoDS_Vertex& V, const Standard_Real P, const TopoDS_Edge& E, const Handle(Geom_Surface)& S, const TopLoc_Location& L, const Standard_Real Tol);
  
  //! Sets the parameters for the vertex on the face.
  Standard_EXPORT static void UpdateVertex (const TopoDS_Vertex& Ve, const Standard_Real U, const Standard_Real V, const TopoDS_Face& F, const Standard_Real Tol);
  
  //! Updates the vertex tolerance.
  Standard_EXPORT static void UpdateVertex (const TopoDS_Vertex& V, const Standard_Real Tol);
  
  //! Transfert the parameters  of   Vin on  Ein as  the
  //! parameter of Vout on Eout.
  Standard_EXPORT static void Transfert (const TopoDS_Edge& Ein, const TopoDS_Edge& Eout, const TopoDS_Vertex& Vin, const TopoDS_Vertex& Vout);




protected:





private:





};


#include <BRep_Builder.lxx>





#endif // _BRep_Builder_HeaderFile
