// Copyright (c) 2012 OPEN CASCADE SAS
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

#ifndef _Graphic3d_Vec2_HeaderFile
#define _Graphic3d_Vec2_HeaderFile

#include <NCollection_Vec2.hxx>
#include <Standard_TypeDef.hxx>

typedef NCollection_Vec2<Standard_ShortReal> Graphic3d_Vec2;
typedef NCollection_Vec2<Standard_Real>      Graphic3d_Vec2d;
typedef NCollection_Vec2<Standard_Integer>   Graphic3d_Vec2i;
typedef NCollection_Vec2<unsigned int>       Graphic3d_Vec2u;
typedef NCollection_Vec2<Standard_Byte>      Graphic3d_Vec2ub;
typedef NCollection_Vec2<Standard_Character> Graphic3d_Vec2b;

#endif // _Graphic3d_Vec2_HeaderFile
