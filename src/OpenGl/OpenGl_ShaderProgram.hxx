// Created on: 2013-09-19
// Created by: Denis BOGOLEPOV
// Copyright (c) 2013 OPEN CASCADE SAS
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

#ifndef _OpenGl_ShaderProgram_Header
#define _OpenGl_ShaderProgram_Header

#include <NCollection_DataMap.hxx>
#include <NCollection_Sequence.hxx>
#include <TCollection_AsciiString.hxx>

#include <Graphic3d_ShaderObject.hxx>
#include <Graphic3d_ShaderProgram.hxx>

#include <InterfaceGraphic_tgl_all.hxx>

#include <OpenGl_Vec.hxx>
#include <OpenGl_Matrix.hxx>
#include <OpenGl_ShaderObject.hxx>
#include <Handle_OpenGl_ShaderProgram.hxx>

//! The enumeration of OCCT-specific OpenGL/GLSL variables.
enum OpenGl_StateVariable
{
  // OpenGL matrix state
  OpenGl_OCC_MODEL_WORLD_MATRIX,
  OpenGl_OCC_WORLD_VIEW_MATRIX,
  OpenGl_OCC_PROJECTION_MATRIX,
  OpenGl_OCC_MODEL_WORLD_MATRIX_INVERSE,
  OpenGl_OCC_WORLD_VIEW_MATRIX_INVERSE,
  OpenGl_OCC_PROJECTION_MATRIX_INVERSE,
  OpenGl_OCC_MODEL_WORLD_MATRIX_TRANSPOSE,
  OpenGl_OCC_WORLD_VIEW_MATRIX_TRANSPOSE,
  OpenGl_OCC_PROJECTION_MATRIX_TRANSPOSE,
  OpenGl_OCC_MODEL_WORLD_MATRIX_INVERSE_TRANSPOSE,
  OpenGl_OCC_WORLD_VIEW_MATRIX_INVERSE_TRANSPOSE,
  OpenGl_OCC_PROJECTION_MATRIX_INVERSE_TRANSPOSE,

  // OpenGL clip planes state
  OpenGl_OCC_CLIP_PLANE_EQUATIONS,
  OpenGl_OCC_CLIP_PLANE_SPACES,

  // OpenGL light state
  OpenGl_OCC_LIGHT_SOURCE_COUNT,
  OpenGl_OCC_LIGHT_SOURCE_TYPES,
  OpenGl_OCC_LIGHT_SOURCE_PARAMS,
  OpenGl_OCC_LIGHT_AMBIENT,

  // Material state
  OpenGl_OCCT_ACTIVE_SAMPLER,
  OpenGl_OCCT_TEXTURE_ENABLE,
  OpenGl_OCCT_DISTINGUISH_MODE,
  OpenGl_OCCT_FRONT_MATERIAL,
  OpenGl_OCCT_BACK_MATERIAL,

  // DON'T MODIFY THIS ITEM (insert new items before it)
  OpenGl_OCCT_NUMBER_OF_STATE_VARIABLES
};

class OpenGl_ShaderProgram;

//! Interface for generic setter of user-defined uniform variables.
struct OpenGl_SetterInterface
{
  //! Sets user-defined uniform variable to specified program.
  virtual void Set (const Handle(OpenGl_Context)&           theCtx,
                    const Handle(Graphic3d_ShaderVariable)& theVariable,
                    OpenGl_ShaderProgram*                   theProgram) = 0;

  //! Destructor
  virtual ~OpenGl_SetterInterface() {}
};

//! List of OpenGL shader objects.
typedef NCollection_Sequence<Handle(OpenGl_ShaderObject)>    OpenGl_ShaderList;

//! List of shader variable setters.
typedef NCollection_DataMap<size_t, OpenGl_SetterInterface*> OpenGl_SetterList;

//! Support tool for setting user-defined uniform variables.
class OpenGl_VariableSetterSelector
{
public:

  //! Creates new setter selector.
  OpenGl_VariableSetterSelector();

  //! Releases memory resources of setter selector.
  ~OpenGl_VariableSetterSelector();

  //! Sets user-defined uniform variable to specified program.
  void Set (const Handle(OpenGl_Context)&           theCtx,
            const Handle(Graphic3d_ShaderVariable)& theVariable,
            OpenGl_ShaderProgram*                   theProgram) const;

private:

  //! List of variable setters.
  OpenGl_SetterList mySetterList;
};

//! Defines types of uniform state variables.
enum OpenGl_UniformStateType
{
  OpenGl_LIGHT_SOURCES_STATE,
  OpenGl_CLIP_PLANES_STATE,
  OpenGl_MODEL_WORLD_STATE,
  OpenGl_WORLD_VIEW_STATE,
  OpenGl_PROJECTION_STATE,
  OpenGl_MATERIALS_STATE
};

//! Total number of state types.
const int MaxStateTypes = 6;

//! Wrapper for OpenGL program object.
class OpenGl_ShaderProgram : public OpenGl_Resource
{

public:

  //! Non-valid shader name.
  static const GLuint NO_PROGRAM = 0;

  //! Invalid location of uniform/attribute variable.
  static const GLint INVALID_LOCATION = -1;

  //! List of pre-defined OCCT state uniform variables.
  static Standard_CString PredefinedKeywords[OpenGl_OCCT_NUMBER_OF_STATE_VARIABLES];

protected:

  //! Creates uninitialized shader program.
  Standard_EXPORT OpenGl_ShaderProgram (const Handle(Graphic3d_ShaderProgram)& theProxy = NULL);

  static OpenGl_VariableSetterSelector mySetterSelector;

public:

  //! Releases resources of shader program.
  Standard_EXPORT virtual ~OpenGl_ShaderProgram();

  //! Creates new empty shader program of specified type.
  Standard_EXPORT Standard_Boolean Create (const Handle(OpenGl_Context)& theCtx);

  //! Destroys shader program.
  Standard_EXPORT virtual void Release (const OpenGl_Context* theCtx);

  //! Attaches shader object to the program object.
  Standard_EXPORT Standard_Boolean AttachShader (const Handle(OpenGl_Context)&      theCtx,
                                                 const Handle(OpenGl_ShaderObject)& theShader);

  //! Detaches shader object to the program object.
  Standard_EXPORT Standard_Boolean DetachShader (const Handle(OpenGl_Context)&      theCtx,
                                                 const Handle(OpenGl_ShaderObject)& theShader);

  //! Initializes program object with the list of shader objects.
  Standard_EXPORT Standard_Boolean Initialize (const Handle(OpenGl_Context)&     theCtx,
                                               const Graphic3d_ShaderObjectList& theShaders);

  //! Links the program object.
  Standard_EXPORT Standard_Boolean Link (const Handle(OpenGl_Context)& theCtx);

  //! Fetches information log of the last link operation.
  Standard_EXPORT Standard_Boolean FetchInfoLog (const Handle(OpenGl_Context)& theCtx,
                                                 TCollection_AsciiString&      theLog);

  //! Fetches uniform variables from proxy shader program.
  Standard_EXPORT Standard_Boolean ApplyVariables (const Handle(OpenGl_Context)& theCtx);

  //! Sets the program object as part of current rendering state.
  Standard_EXPORT void Bind (const Handle(OpenGl_Context)& theCtx) const;

  //! Binds the program object and applies variables from proxy shader program.
  Standard_EXPORT Standard_Boolean BindWithVariables (const Handle(OpenGl_Context)& theCtx);

  //! Reverts to fixed-function graphics pipeline (FFP).
  Standard_EXPORT static void Unbind (const Handle(OpenGl_Context)& theCtx);

  //! @return true if current object was initialized
  inline bool IsValid() const
  {
    return myProgramID != NO_PROGRAM;
  }

private:

  //! Returns index of last modification of variables of specified state type.
  Standard_EXPORT Standard_Size ActiveState (const OpenGl_UniformStateType theType) const;

  //! Updates index of last modification of variables of specified state type.
  Standard_EXPORT void UpdateState (const OpenGl_UniformStateType theType,
                                    const Standard_Size           theIndex);

public:

  //! Returns location of the specific uniform variable.
  Standard_EXPORT GLint GetUniformLocation (const Handle(OpenGl_Context)& theCtx,
                                            const GLchar*                 theName) const;

  //! Returns index of the generic vertex attribute by variable name.
  Standard_EXPORT GLint GetAttributeLocation (const Handle(OpenGl_Context)& theCtx,
                                              const GLchar*                 theName) const;

  //! Returns location of the OCCT state uniform variable.
  Standard_EXPORT GLint GetStateLocation (const GLuint theVariable) const;

public:

  //! Returns the value of the integer uniform variable.
  Standard_EXPORT Standard_Boolean GetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               OpenGl_Vec4i&                 theValue) const;

  Standard_EXPORT Standard_Boolean GetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               OpenGl_Vec4i&                 theValue) const;

  //! Returns the value of the float uniform variable.
  Standard_EXPORT Standard_Boolean GetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               OpenGl_Vec4&                  theValue) const;

  //! Returns the value of the float uniform variable.
  Standard_EXPORT Standard_Boolean GetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               OpenGl_Vec4&                  theValue) const;

public:

  //! Returns the integer vertex attribute.
  Standard_EXPORT Standard_Boolean GetAttribute (const Handle(OpenGl_Context)& theCtx,
                                                 const GLchar*                 theName,
                                                 OpenGl_Vec4i&                 theValue) const;

  //! Returns the integer vertex attribute.
  Standard_EXPORT Standard_Boolean GetAttribute (const Handle(OpenGl_Context)& theCtx,
                                                 GLint                         theIndex,
                                                 OpenGl_Vec4i&                 theValue) const;

  //! Returns the float vertex attribute.
  Standard_EXPORT Standard_Boolean GetAttribute (const Handle(OpenGl_Context)& theCtx,
                                                 const GLchar*                 theName,
                                                 OpenGl_Vec4&                  theValue) const;

  //! Returns the float vertex attribute.
  Standard_EXPORT Standard_Boolean GetAttribute (const Handle(OpenGl_Context)& theCtx,
                                                 GLint                         theIndex,
                                                 OpenGl_Vec4&                  theValue) const;

public:

  //! Specifies the value of the integer uniform variable.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               GLint                         theValue);

  //! Specifies the value of the integer uniform variable.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               GLint                         theValue);

  //! Specifies the value of the integer uniform 2D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               const OpenGl_Vec2i&           theValue);

  //! Specifies the value of the integer uniform 2D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               const OpenGl_Vec2i&           theValue);

  //! Specifies the value of the integer uniform 3D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               const OpenGl_Vec3i&           theValue);

  //! Specifies the value of the integer uniform 3D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               const OpenGl_Vec3i&           theValue);

  //! Specifies the value of the integer uniform 4D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               const OpenGl_Vec4i&           theValue);

  //! Specifies the value of the integer uniform 4D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               const OpenGl_Vec4i&           theValue);

public:

  //! Specifies the value of the float uniform variable.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               GLfloat                       theValue);

  //! Specifies the value of the float uniform variable.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               GLfloat                       theValue);

  //! Specifies the value of the float uniform 2D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               const OpenGl_Vec2&            theValue);

  //! Specifies the value of the float uniform 2D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               const OpenGl_Vec2&            theValue);

  //! Specifies the value of the float uniform 3D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               const OpenGl_Vec3&            theValue);

  //! Specifies the value of the float uniform 3D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               const OpenGl_Vec3&            theValue);

  //! Specifies the value of the float uniform 4D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               const OpenGl_Vec4&            theValue);

  //! Specifies the value of the float uniform 4D vector.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               const OpenGl_Vec4&            theValue);

public:

  //! Specifies the value of the float uniform 4x4 matrix.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               const OpenGl_Matrix&          theValue,
                                               GLboolean                     theTranspose = GL_FALSE);

  //! Specifies the value of the float uniform 4x4 matrix.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               const OpenGl_Matrix&          theValue,
                                               GLboolean                     theTranspose = GL_FALSE);

  //! Specifies the value of the float uniform 4x4 matrix.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               const Tmatrix3&               theValue,
                                               GLboolean                     theTranspose = GL_FALSE);

  //! Specifies the value of the float uniform 4x4 matrix.
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               const Tmatrix3&               theValue,
                                               GLboolean                     theTranspose = GL_FALSE);

  //! Specifies the value of the float uniform array
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               GLuint                        theCount,
                                               const Standard_ShortReal*     theData);

  //! Specifies the value of the float2 uniform array
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               GLuint                        theCount,
                                               const OpenGl_Vec2*            theData);

  //! Specifies the value of the float3 uniform array
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               GLuint                        theCount,
                                               const OpenGl_Vec3*            theData);

  //! Specifies the value of the float4 uniform array
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               GLuint                        theCount,
                                               const OpenGl_Vec4*            theData);

  //! Specifies the value of the integer uniform array
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               GLuint                        theCount,
                                               const Standard_Integer*       theData);

  //! Specifies the value of the int2 uniform array
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               GLuint                        theCount,
                                               const OpenGl_Vec2i*           theData);

  //! Specifies the value of the int3 uniform array
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               GLuint                        theCount,
                                               const OpenGl_Vec3i*           theData);

  //! Specifies the value of the int4 uniform array
  Standard_EXPORT Standard_Boolean SetUniform (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               GLuint                        theCount,
                                               const OpenGl_Vec4i*           theData);

public:

  //! Specifies the value of the sampler uniform variable.
  Standard_EXPORT Standard_Boolean SetSampler (const Handle(OpenGl_Context)& theCtx,
                                               const GLchar*                 theName,
                                               const GLenum                  theTextureUnit);

  //! Specifies the value of the sampler uniform variable.
  Standard_EXPORT Standard_Boolean SetSampler (const Handle(OpenGl_Context)& theCtx,
                                               GLint                         theLocation,
                                               const GLenum                  theTextureUnit);

protected:

  //! Increments counter of users.
  //! Used by OpenGl_ShaderManager.
  //! @return true when resource has been restored from delayed release queue
  bool Share()
  {
    return ++myShareCount == 1;
  }

  //! Decrements counter of users.
  //! Used by OpenGl_ShaderManager.
  //! @return true when there are no more users of this program has been left
  bool UnShare()
  {
    return --myShareCount == 0;
  }

protected:

  GLuint                          myProgramID;     //!< Handle of OpenGL shader program
  OpenGl_ShaderList               myShaderObjects; //!< List of attached shader objects
  Handle(Graphic3d_ShaderProgram) myProxy;         //!< Proxy shader program (from application layer)
  Standard_Integer                myShareCount;    //!< program users count, initialized with 1 (already shared by one user)

protected:

  //! Defines last modification for variables of each state type.
  Standard_Size myCurrentState[MaxStateTypes];

  //! Stores locations of OCCT state uniform variables.
  GLint myStateLocations[OpenGl_OCCT_NUMBER_OF_STATE_VARIABLES];

public:

  DEFINE_STANDARD_RTTI (OpenGl_ShaderProgram)
  friend class OpenGl_ShaderManager;

};

template<class T>
struct OpenGl_VariableSetter : public OpenGl_SetterInterface
{
  virtual void Set (const Handle(OpenGl_Context)&           theCtx,
                    const Handle(Graphic3d_ShaderVariable)& theVariable,
                    OpenGl_ShaderProgram*                   theProgram)
  {
    theProgram->SetUniform (theCtx,
                            theVariable->Name().ToCString(),
                            theVariable->Value()->As<T>());
  }
};

namespace OpenGl_HashMapInitializer
{
  template<class K, class V>
  struct MapListOfType
  {
    NCollection_DataMap<K, V> myDictionary;

    MapListOfType (K theKey, V theValue)
    {
      myDictionary.Bind (theKey, theValue);
    }

    MapListOfType& operator() (K theKey, V theValue)
    {
      myDictionary.Bind (theKey, theValue);
      return *this;
    }

    operator const NCollection_DataMap<K, V>& () const
    {
      return myDictionary;
    }
  };

  template<class K, class V>
  MapListOfType<K, V> CreateListOf (K theKey, V theValue)
  {
    return MapListOfType<K, V> (theKey, theValue);
  }
}

#endif // _OpenGl_ShaderProgram_Header
