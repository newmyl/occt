// Created on: 
// Created by: 
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

#include <Graphic3d_HLR.hxx>
#include <TCollection_AsciiString.hxx>

#define WRAP_STR(x) #x

//==============================================================================
//function : uniform variables for shaders
//purpose  :
//==============================================================================
const TCollection_AsciiString UNIFORM = WRAP_STR(

  uniform  mat4    uModelViewProjection;
  uniform  mat4    uModelView;
  uniform  float   uOrthoScale;

  uniform  vec4    uColor;
  uniform  float   uUseVertexColor;
  uniform  vec3    uSelectionColor;
  uniform  float   uIsSelected;
  uniform  float   uNbClipPlanes;
  uniform  float   uCappingIndex;
  uniform  vec4[8] uPlaneEquations;
  uniform  vec3    uBackgroundColor;
  uniform  vec3    uSilhouetteColor;
  uniform  float   uIsSilhouettePass;
);

//==============================================================================
//function : vertex shader for HLR
//purpose  :
//==============================================================================
const TCollection_AsciiString VERTEX_SHADER = UNIFORM + WRAP_STR(

  in vec4 position;
  in vec4 normal;
  in vec4 color0;
  out vec4 vPosition;
  out vec4 vNormal;
  out vec3 vNormalWorld;
  out vec4 vColor;

  void main()
  {
    gl_Position = uModelViewProjection * position;
    vPosition = uModelView * position;
  
    vec4 aNormal = vec4(normal.xyz, 0.0);
    vNormal = uModelView * aNormal;
    vNormalWorld = aNormal.xyz;
    vec3 aDisplacedPosition;
  
    if (uIsSilhouettePass > 0.1)
    {
      float aShift = 0.0008;
  
      if (uOrthoScale > 0.0)
      {
        aShift *= uOrthoScale;
        aDisplacedPosition = position.xyz + (normal.xyz * aShift);
      }
      else
      {
        aDisplacedPosition = position.xyz + (normal.xyz * aShift * gl_Position.w);
      }

      gl_Position = uModelViewProjection * vec4(aDisplacedPosition, 1.0);
    }

    vColor = color0;
  }
);

//==============================================================================
//function : fragment shader for HLR
//purpose  :
//==============================================================================
const TCollection_AsciiString FRAGMENT_SHADER  = UNIFORM + WRAP_STR(

  in vec4 vPosition;
  in vec4 vNormal;
  in vec3 vNormalWorld;
  in vec4 vColor;

  out vec4 outColor;

  void main()
  {
    for (int i = 0; i < 8; ++i)
    {
      if (i == int(uNbClipPlanes))
        break;

      float aClipDist = dot(uPlaneEquations[i].xyz, vPosition.xyz) + uPlaneEquations[i].w;
      if (aClipDist < 0.0)
      {
        discard;
      }
    }

    vec3 aColor = uBackgroundColor;

    if (uIsSilhouettePass > 0.1)
    {
      aColor = uSilhouetteColor;
      if (uIsSelected > 0.1)
        aColor = uSelectionColor;
    }
  
    outColor = vec4(aColor, uColor.a);
  }
);

// =======================================================================
// function : generateShaderPrefix
// purpose  :
// =======================================================================
#define EOL "\n"
const TCollection_AsciiString HEADER = 
  EOL"#version 140"
  EOL"#extension GL_EXT_gpu_shader4 : require"
  EOL"#extension GL_ARB_bindless_texture : require"
  EOL"#extension GL_ARB_shader_image_load_store : require"
  EOL;


namespace Graphic3d_HLR
{
  // =======================================================================
  // function : header
  // purpose  :
  // =======================================================================
  TCollection_AsciiString header()
  {
    return HEADER;
  }

  // =======================================================================
  // function : vertexShader
  // purpose  :
  // =======================================================================
  TCollection_AsciiString vertexShader()
  {
    return VERTEX_SHADER;
  }

  // =======================================================================
  // function : fragmentShader
  // purpose  :
  // =======================================================================
  TCollection_AsciiString fragmentShader()
  {
    return FRAGMENT_SHADER;
  }
};
