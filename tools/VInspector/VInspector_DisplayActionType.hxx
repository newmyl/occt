// Created on: 2018-08-19
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2018 OPEN CASCADE SAS
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

#ifndef VInspector_DisplayActionType_H
#define VInspector_DisplayActionType_H

//! \enum VInspector_DisplayActionType
enum VInspector_DisplayActionType
{
  VInspector_DisplayActionType_NoneId, //!< No action activated
  VInspector_DisplayActionType_DisplayId, //!< Display action
  VInspector_DisplayActionType_RedisplayId, //!< Redisplay action
  VInspector_DisplayActionType_EraseId, //!< Erase action
  VInspector_DisplayActionType_RemoveId //!< Remove action
};

#endif
