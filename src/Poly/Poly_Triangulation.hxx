// Copyright (c) 2015 OPEN CASCADE SAS
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

#ifndef _Poly_Triangulation_HeaderFile
#define _Poly_Triangulation_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineHandle.hxx>
#include <Standard_Real.hxx>
#include <Standard_Integer.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <Handle_TColgp_HArray1OfPnt2d.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <Handle_TShort_HArray1OfShortReal.hxx>
#include <MMgt_TShared.hxx>
#include <Standard_Boolean.hxx>
#include <NCollection_Vector.hxx>
#include <Poly_Triangle.hxx>

class TColgp_HArray1OfPnt2d;
class TShort_HArray1OfShortReal;
class Standard_DomainError;
class Standard_NullObject;
class TColgp_Array1OfPnt;
class Poly_Array1OfTriangle;
class TColgp_Array1OfPnt2d;
class TShort_Array1OfShortReal;

//! Provides a triangulation for a surface, a set of surfaces, or more generally a shape.
//! A triangulation consists of an approximate representation of the actual shape, using a collection of points and triangles.
//! The points are located on the surface. The edges of the triangles connect adjacent points with a
//! straight line that approximates the true curve on the surface.
//! A triangulation comprises:
//! -   A table of 3D nodes (3D points on the surface).
//! -   A table of triangles. Each triangle (Poly_Triangle object) comprises a triplet of indices in the table of 3D
//!     nodes specific to the triangulation.
//! -   A table of 2D nodes (2D points), parallel to the table of 3D nodes. This table is optional.
//!     If it exists, the coordinates of a 2D point are the (u, v) parameters of the corresponding 3D point on the surface approximated by the triangulation.
//! -   A deflection (optional), which maximizes the distance from a point on the surface to the corresponding point on its approximate triangulation.
//! In many cases, algorithms do not need to work with the exact representation of a surface.
//! A triangular representation induces simpler and more robust adjusting, faster performances, and the results are as good.
class Poly_Triangulation : public MMgt_TShared
{

public:

  //! Constructs a triangulation from a set of triangles.
  //! The triangulation is initialized without a triangle or a node, but capable of containing theNbNodes nodes, and theNbTriangles triangles.
  //! Here theHasUVNodes flag indicates whether 2D nodes will be associated with 3D ones, (i.e. to enable a 2D representation).
  Standard_EXPORT Poly_Triangulation (const Standard_Integer theNbNodes,
                                      const Standard_Integer theNbTriangles,
                                      const Standard_Boolean theHasUVNodes);

  //! Constructs a triangulation from a set of triangles.
  //! The triangulation is initialized with 3D points from Nodes and triangles from Triangles.
  Standard_EXPORT Poly_Triangulation (const TColgp_Array1OfPnt&    theNodes,
                                      const Poly_Array1OfTriangle& theTriangles);

  //! Constructs a triangulation from a set of triangles.
  //! The triangulation is initialized with 3D points from Nodes, 2D points from theUVNodes and triangles from theTriangles,
  //! where coordinates of a 2D point from theUVNodes are the (u, v) parameters of the corresponding 3D point
  //! from theNodes on the surface approximated by the constructed triangulation. If size of theUVNodes != size of theNodes
  //! then triangulation will be created without theUVNodes.
  Standard_EXPORT Poly_Triangulation (const TColgp_Array1OfPnt&    theNodes,
                                      const TColgp_Array1OfPnt2d&  theUVNodes,
                                      const Poly_Array1OfTriangle& theTriangles);

  //! Copy constructor for triangulation.
  Standard_EXPORT Poly_Triangulation (const Handle(Poly_Triangulation)& theTriangulation);

  //! Returns the deflection of this triangulation.
  Standard_EXPORT Standard_Real Deflection() const;

  //! Sets the deflection of this triangulation to theDeflection.
  //! See more on deflection in Polygon2D
  Standard_EXPORT void Deflection (const Standard_Real theDeflection);

  //! Deallocates the UV nodes.
  Standard_EXPORT void RemoveUVNodes();

  //! @return the number of nodes for this triangulation.
  Standard_Integer NbNodes() const { return myNodes.Size(); }

  //! @return the number of triangles for this triangulation.
  Standard_Integer NbTriangles() const { return myTriangles.Size(); }

  //! @return Standard_True if 2D nodes are associated with 3D nodes for this triangulation.
  Standard_Boolean HasUVNodes() const { return myHasUVNodes; }

  //! Adds Node to the triangulation. If triangulation has UVNodes or Normals
  //! they will be expanded and set to zero values to match the new number of nodes.
  //! @return index of the added Node.
  Standard_EXPORT Standard_Integer AddNode (const gp_Pnt& theNode);

  //! @return node at the given index.
  //! Raises Standard_OutOfRange exception if theIndex is less than 1 or greater than NbNodes.
  Standard_EXPORT const gp_Pnt& Node (const Standard_Integer theIndex) const;

  //! Give access to the node at the given index.
  //! Raises Standard_OutOfRange exception if theIndex is less than 1 or greater than NbNodes.
  Standard_EXPORT gp_Pnt& ChangeNode (const Standard_Integer theIndex);

  //! @return UVNode at the given index.
  //! Raises Standard_OutOfRange exception if theIndex is less than 1 or greater than NbNodes.
  Standard_EXPORT const gp_Pnt2d& UVNode (const Standard_Integer theIndex) const;

  //! Give access to the UVNode at the given index.
  //! Raises Standard_OutOfRange exception if theIndex is less than 1 or greater than NbNodes.
  Standard_EXPORT gp_Pnt2d& ChangeUVNode (const Standard_Integer theIndex);

  //! Adds triangle to the triangulation.
  //! @return index of the added triangle.
  Standard_EXPORT virtual Standard_Integer AddTriangle (const Poly_Triangle& theTriangle);

  //! @return triangle at the given index.
  //! Raises Standard_OutOfRange exception if theIndex is less than 1 or greater than NbTriangles.
  Standard_EXPORT const Poly_Triangle& Triangle (const Standard_Integer theIndex) const;

  //! Give access to the triangle at the given index.
  //! Raises Standard_OutOfRange exception if theIndex is less than 1 or greater than NbTriangles.
  Standard_EXPORT Poly_Triangle& ChangeTriangle (const Standard_Integer theIndex);

  //! Sets the table of node normals.
  //! Raises exception if length of theNormals != 3 * NbNodes
  Standard_EXPORT void SetNormals (const Handle(TShort_HArray1OfShortReal)& theNormals);

  //! Changes normal at the given index.
  //! Raises Standard_OutOfRange exception.
  Standard_EXPORT void SetNormal (const Standard_Integer theIndex,
                                  const gp_Dir&          theNormal);

  //! Returns Standard_True if nodal normals are defined.
  Standard_EXPORT Standard_Boolean HasNormals() const;

  //! @return normal at the given index.
  //! Raises Standard_OutOfRange exception.
  Standard_EXPORT const gp_Dir Normal (const Standard_Integer theIndex) const;

protected:

  Standard_Boolean                       myHasUVNodes;
  Standard_Real                          myDeflection;
  NCollection_Vector<gp_Pnt>             myNodes;
  NCollection_Vector<gp_Pnt2d>           myUVNodes;
  NCollection_Vector<Poly_Triangle>      myTriangles;
  NCollection_Vector<Standard_ShortReal> myNormals;

public:

  DEFINE_STANDARD_RTTI(Poly_Triangulation)

};

DEFINE_STANDARD_HANDLE(Poly_Triangulation, MMgt_TShared)

#endif // _Poly_Triangulation_HeaderFile
