// Created on: 2013-09-19
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

#include <Graphic3d_ShaderObject.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_ShaderObject.hxx>
#include <OSD_Path.hxx>
#include <Standard_Assert.hxx>
#include <TCollection_AsciiString.hxx>

#ifdef _WIN32
  #include <malloc.h> // for alloca()
#endif

IMPLEMENT_STANDARD_RTTIEXT(OpenGl_ShaderObject,OpenGl_Resource)

// =======================================================================
// function : CreateFromSource
// purpose  :
// =======================================================================
Handle(Graphic3d_ShaderObject) OpenGl_ShaderObject::CreateFromSource (TCollection_AsciiString& theSource,
                                                                      Graphic3d_TypeOfShaderObject theType,
                                                                      const ShaderVariableList& theUniforms,
                                                                      const ShaderVariableList& theStageInOuts,
                                                                      const TCollection_AsciiString& theInName,
                                                                      const TCollection_AsciiString& theOutName,
                                                                      Standard_Integer theNbGeomInputVerts)
{
  TCollection_AsciiString aSrcUniforms, aSrcInOuts, aSrcInStructs, aSrcOutStructs;
  for (ShaderVariableList::Iterator anUniformIter (theUniforms); anUniformIter.More(); anUniformIter.Next())
  {
    const ShaderVariable& aVar = anUniformIter.Value();
    if ((aVar.Stages & theType) != 0)
    {
      aSrcUniforms += TCollection_AsciiString("\nuniform ") + aVar.Name + ";";
    }
  }
  for (ShaderVariableList::Iterator aVarListIter (theStageInOuts); aVarListIter.More(); aVarListIter.Next())
  {
    const ShaderVariable& aVar = aVarListIter.Value();
    Standard_Integer aStageLower = IntegerLast(), aStageUpper = IntegerFirst();
    Standard_Integer aNbStages = 0;
    for (Standard_Integer aStageIter = Graphic3d_TOS_VERTEX; aStageIter <= (Standard_Integer )Graphic3d_TOS_COMPUTE; aStageIter = aStageIter << 1)
    {
      if ((aVar.Stages & aStageIter) != 0)
      {
        ++aNbStages;
        aStageLower = Min (aStageLower, aStageIter);
        aStageUpper = Max (aStageUpper, aStageIter);
      }
    }
    if ((Standard_Integer )theType < aStageLower
     || (Standard_Integer )theType > aStageUpper)
    {
      continue;
    }

    const Standard_Boolean hasGeomStage = theNbGeomInputVerts > 0
                                       && aStageLower <  Graphic3d_TOS_GEOMETRY
                                       && aStageUpper >= Graphic3d_TOS_GEOMETRY;
    const Standard_Boolean isAllStagesVar = aStageLower == Graphic3d_TOS_VERTEX
                                         && aStageUpper == Graphic3d_TOS_FRAGMENT;
    if (hasGeomStage
    || !theInName.IsEmpty()
    || !theOutName.IsEmpty())
    {
      if (aSrcInStructs.IsEmpty()
       && aSrcOutStructs.IsEmpty()
       && isAllStagesVar)
      {
        if (theType == aStageLower)
        {
          aSrcOutStructs = "\nout VertexData\n{";
        }
        else if (theType == aStageUpper)
        {
          aSrcInStructs = "\nin VertexData\n{";
        }
        else // requires theInName/theOutName
        {
          aSrcInStructs  = "\nin  VertexData\n{";
          aSrcOutStructs = "\nout VertexData\n{";
        }
      }
    }

    if (isAllStagesVar
     && (!aSrcInStructs.IsEmpty()
      || !aSrcOutStructs.IsEmpty()))
    {
      if (!aSrcInStructs.IsEmpty())
      {
        aSrcInStructs  += TCollection_AsciiString("\n  ") + aVar.Name + ";";
      }
      if (!aSrcOutStructs.IsEmpty())
      {
        aSrcOutStructs += TCollection_AsciiString("\n  ") + aVar.Name + ";";
      }
    }
    else
    {
      if (theType == aStageLower)
      {
        aSrcInOuts += TCollection_AsciiString("\nTHE_SHADER_OUT ") + aVar.Name + ";";
      }
      else if (theType == aStageUpper)
      {
        aSrcInOuts += TCollection_AsciiString("\nTHE_SHADER_IN ") + aVar.Name + ";";
      }
    }
  }

  if (theType == Graphic3d_TOS_GEOMETRY)
  {
    aSrcUniforms.Prepend (TCollection_AsciiString ("\nlayout (triangles) in;\nlayout (triangle_strip, max_vertices = ") + theNbGeomInputVerts + ") out;");
    if (!aSrcInStructs.IsEmpty())
    {
      aSrcInStructs  += TCollection_AsciiString ("\n} ") + theInName  + "[" + theNbGeomInputVerts + "];";
    }
    if (!aSrcOutStructs.IsEmpty())
    {
      aSrcOutStructs += TCollection_AsciiString ("\n} ") + theOutName + ";";
    }
  }
  else if (!aSrcInStructs.IsEmpty())
  {
    aSrcInStructs += "\n};";
  }
  else if (!aSrcOutStructs.IsEmpty())
  {
    aSrcOutStructs += "\n};";
  }

  theSource.Prepend (aSrcUniforms + aSrcInStructs + aSrcOutStructs + aSrcInOuts);
  return Graphic3d_ShaderObject::CreateFromSource (theType, theSource);
}

// =======================================================================
// function : OpenGl_ShaderObject
// purpose  : Creates uninitialized shader object
// =======================================================================
OpenGl_ShaderObject::OpenGl_ShaderObject (GLenum theType)
: myType     (theType),
  myShaderID (NO_SHADER)
{
  //
}

// =======================================================================
// function : ~OpenGl_ShaderObject
// purpose  : Releases resources of shader object
// =======================================================================
OpenGl_ShaderObject::~OpenGl_ShaderObject()
{
  Release (NULL);
}

// =======================================================================
// function : LoadSource
// purpose  : Loads shader source code
// =======================================================================
Standard_Boolean OpenGl_ShaderObject::LoadSource (const Handle(OpenGl_Context)&  theCtx,
                                                  const TCollection_AsciiString& theSource)
{
  if (myShaderID == NO_SHADER)
  {
    return Standard_False;
  }

  const GLchar* aLines = theSource.ToCString();
  theCtx->core20fwd->glShaderSource (myShaderID, 1, &aLines, NULL);
  return Standard_True;
}

// =======================================================================
// function : Compile
// purpose  : Compiles the shader object
// =======================================================================
Standard_Boolean OpenGl_ShaderObject::Compile (const Handle(OpenGl_Context)& theCtx)
{
  if (myShaderID == NO_SHADER)
  {
    return Standard_False;
  }

  // Try to compile shader
  theCtx->core20fwd->glCompileShader (myShaderID);

  // Check compile status
  GLint aStatus = GL_FALSE;
  theCtx->core20fwd->glGetShaderiv (myShaderID, GL_COMPILE_STATUS, &aStatus);
  return aStatus != GL_FALSE;
}

// =======================================================================
// function : FetchInfoLog
// purpose  : Fetches information log of the last compile operation
// =======================================================================
Standard_Boolean OpenGl_ShaderObject::FetchInfoLog (const Handle(OpenGl_Context)& theCtx,
                                                    TCollection_AsciiString&      theLog)
{
  if (myShaderID == NO_SHADER)
  {
    return Standard_False;
  }

  // Load information log of the compiler
  GLint aLength = 0;
  theCtx->core20fwd->glGetShaderiv (myShaderID, GL_INFO_LOG_LENGTH, &aLength);
  if (aLength > 0)
  {
    GLchar* aLog = (GLchar*) alloca (aLength);
    memset (aLog, 0, aLength);
    theCtx->core20fwd->glGetShaderInfoLog (myShaderID, aLength, NULL, aLog);
    theLog = aLog;
  }

  return Standard_True;
}

// =======================================================================
// function : Create
// purpose  : Creates new empty shader object of specified type
// =======================================================================
Standard_Boolean OpenGl_ShaderObject::Create (const Handle(OpenGl_Context)& theCtx)
{
  if (myShaderID == NO_SHADER
   && theCtx->core20fwd != NULL)
  {
    myShaderID = theCtx->core20fwd->glCreateShader (myType);
  }

  return myShaderID != NO_SHADER;
}

// =======================================================================
// function : Release
// purpose  : Destroys shader object
// =======================================================================
void OpenGl_ShaderObject::Release (OpenGl_Context* theCtx)
{
  if (myShaderID == NO_SHADER)
  {
    return;
  }

  Standard_ASSERT_RETURN (theCtx != NULL,
    "OpenGl_ShaderObject destroyed without GL context! Possible GPU memory leakage...",);

  if (theCtx->core20fwd != NULL
   && theCtx->IsValid())
  {
    theCtx->core20fwd->glDeleteShader (myShaderID);
  }
  myShaderID = NO_SHADER;
}
