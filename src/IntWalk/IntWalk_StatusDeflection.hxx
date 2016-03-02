// Created on: 1991-03-28
// Created by: Jacques GOUSSARD
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

#ifndef _IntWalk_StatusDeflection_HeaderFile
#define _IntWalk_StatusDeflection_HeaderFile


enum IntWalk_StatusDeflection
{
IntWalk_PasTropGrand,
IntWalk_StepTooSmall,
IntWalk_PointConfondu,
IntWalk_ArretSurPointPrecedent,
IntWalk_ArretSurPoint,
IntWalk_IntersectsBoundary,
IntWalk_IntersectPointNotFound,
IntWalk_TooManyPoints,
IntWalk_AnotherDirection,
IntWalk_DirectionNotFound,
IntWalk_ForcefulAdding,
IntWalk_OK
};

#endif // _IntWalk_StatusDeflection_HeaderFile
