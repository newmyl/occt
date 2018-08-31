// Created on: 2013-12-20
// Created by: Denis BOGOLEPOV
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

#ifndef _BVH_TreeBase_Header
#define _BVH_TreeBase_Header

#include <BVH_Box.hxx>

template<class T, int N> class BVH_Builder;

//! A non-template class for using as base for BVH_TreeBase
//! (just to have a named base class).
class BVH_TreeBaseTransient : public Standard_Transient
{
  DEFINE_STANDARD_RTTIEXT(BVH_TreeBaseTransient, Standard_Transient)
protected:
  BVH_TreeBaseTransient() {}
};

//! Stores parameters of bounding volume hierarchy (BVH).
//! Bounding volume hierarchy (BVH) organizes geometric objects in
//! the tree based on spatial relationships. Each node in the tree
//! contains an axis-aligned bounding box of all the objects below
//! it. Bounding volume hierarchies are used in many algorithms to
//! support efficient operations on the sets of geometric objects,
//! such as collision detection, ray-tracing, searching of nearest
//! objects, and view frustum culling.
template<class T, int N>
class BVH_TreeBase : public BVH_TreeBaseTransient
{
  friend class BVH_Builder<T, N>;

public: //! @name custom data types

  typedef typename BVH_Box<T, N>::BVH_VecNt BVH_VecNt;

public: //! @name general methods

  //! Creates new empty BVH tree.
  BVH_TreeBase() : myDepth (0) { }

  //! Releases resources of BVH tree.
  virtual ~BVH_TreeBase() {}

  //! Returns depth (height) of BVH tree.
  int Depth() const
  {
    return myDepth;
  }

  //! Returns total number of BVH tree nodes.
  int Length() const
  {
    return BVH::Array<int, 4>::Size (myNodeInfoBuffer);
  }

//! @name methods for accessing BVH_Vec4i properties within NodeInfoBuffer()
public:

  //! Returns index of first primitive of the given leaf node.
  static int& BegPrimitive (BVH_Vec4i& theNodeInfo) { return theNodeInfo.y(); }

  //! Returns index of first primitive of the given leaf node.
  static int BegPrimitive (const BVH_Vec4i& theNodeInfo) { return theNodeInfo.y(); }

  //! Returns index of last primitive of the given leaf node.
  static int& EndPrimitive (BVH_Vec4i& theNodeInfo) { return theNodeInfo.z(); }

  //! Returns index of last primitive of the given leaf node.
  static int EndPrimitive (const BVH_Vec4i& theNodeInfo) { return theNodeInfo.z(); }

  //! Returns number of primitives in the given leaf node.
  static int NbPrimitives (const BVH_Vec4i& theNodeInfo) { return EndPrimitive (theNodeInfo) - BegPrimitive (theNodeInfo) + 1; }

  //! Returns level (depth) of the given node.
  static int& Level (BVH_Vec4i& theNodeInfo) { return theNodeInfo.w(); }

  //! Returns level (depth) of the given node.
  static int Level (const BVH_Vec4i& theNodeInfo) { return theNodeInfo.w(); }

  //! Checks whether the given node is outer.
  static bool IsOuter (const BVH_Vec4i& theNodeInfo) { return theNodeInfo.x() != 0; }

public: //! @name methods for accessing individual nodes

  //! Returns minimum point of the given node.
  BVH_VecNt& MinPoint (const int theNodeIndex)
  {
    return BVH::Array<T, N>::ChangeValue (myMinPointBuffer, theNodeIndex);
  }

  //! Returns maximum point of the given node.
  BVH_VecNt& MaxPoint (const int theNodeIndex)
  {
    return BVH::Array<T, N>::ChangeValue (myMaxPointBuffer, theNodeIndex);
  }

  //! Returns minimum point of the given node.
  const BVH_VecNt& MinPoint (const int theNodeIndex) const
  {
    return BVH::Array<T, N>::Value (myMinPointBuffer, theNodeIndex);
  }

  //! Returns maximum point of the given node.
  const BVH_VecNt& MaxPoint (const int theNodeIndex) const
  {
    return BVH::Array<T, N>::Value (myMaxPointBuffer, theNodeIndex);
  }

  //! Returns index of first primitive of the given leaf node.
  int& BegPrimitive (int theNodeIndex) { return BegPrimitive (BVH::Array<int, 4>::ChangeValue (myNodeInfoBuffer, theNodeIndex)); }

  //! Returns index of last primitive of the given leaf node.
  int& EndPrimitive (int theNodeIndex) { return EndPrimitive (BVH::Array<int, 4>::ChangeValue (myNodeInfoBuffer, theNodeIndex)); }

  //! Returns index of first primitive of the given leaf node.
  int BegPrimitive (int theNodeIndex) const { return BegPrimitive (BVH::Array<int, 4>::Value (myNodeInfoBuffer, theNodeIndex)); }

  //! Returns index of last primitive of the given leaf node.
  int EndPrimitive (int theNodeIndex) const { return EndPrimitive (BVH::Array<int, 4>::Value (myNodeInfoBuffer, theNodeIndex)); }

  //! Returns number of primitives in the given leaf node.
  int NbPrimitives (int theNodeIndex) const { return EndPrimitive (theNodeIndex) - BegPrimitive (theNodeIndex) + 1; }

  //! Returns level (depth) of the given node.
  int& Level (int theNodeIndex) { return BVH::Array<int, 4>::ChangeValue (myNodeInfoBuffer, theNodeIndex).w(); }

  //! Returns level (depth) of the given node.
  int Level (int theNodeIndex) const { return BVH::Array<int, 4>::Value (myNodeInfoBuffer, theNodeIndex).w(); }

  //! Checks whether the given node is outer.
  bool IsOuter (int theNodeIndex) const { return IsOuter (BVH::Array<int, 4>::Value (myNodeInfoBuffer, theNodeIndex)); }

public: //! @name methods for accessing serialized tree data

  //! Returns array of node data records.
  BVH_Array4i& NodeInfoBuffer()
  {
    return myNodeInfoBuffer;
  }

  //! Returns array of node data records.
  const BVH_Array4i& NodeInfoBuffer() const
  {
    return myNodeInfoBuffer;
  }

  //! Returns array of node minimum points.
  typename BVH::ArrayType<T, N>::Type& MinPointBuffer()
  {
    return myMinPointBuffer;
  }

  //! Returns array of node maximum points.
  typename BVH::ArrayType<T, N>::Type& MaxPointBuffer()
  {
    return myMaxPointBuffer;
  }

  //! Returns array of node minimum points.
  const typename BVH::ArrayType<T, N>::Type& MinPointBuffer() const
  {
    return myMinPointBuffer;
  }

  //! Returns array of node maximum points.
  const typename BVH::ArrayType<T, N>::Type& MaxPointBuffer() const
  {
    return myMaxPointBuffer;
  }

public: //! @name protected fields

  //! Array of node data records.
  BVH_Array4i myNodeInfoBuffer;

  //! Array of node minimum points.
  typename BVH::ArrayType<T, N>::Type myMinPointBuffer;

  //! Array of node maximum points.
  typename BVH::ArrayType<T, N>::Type myMaxPointBuffer;

  //! Current depth of BVH tree (set by builder).
  int myDepth;

};

//! Type corresponding to quad BVH.
struct BVH_QuadTree { };

//! Type corresponding to binary BVH.
struct BVH_BinaryTree { };

//! BVH tree with given arity (2 or 4).
template<class T, int N, class Arity = BVH_BinaryTree>
class BVH_Tree
{
  // Invalid type
};

#endif // _BVH_TreeBase_Header
