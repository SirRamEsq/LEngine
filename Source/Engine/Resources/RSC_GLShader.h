#ifndef L_ENGINE_GL_SHADER
#define L_ENGINE_GL_SHADER

#include <GL/glew.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../Errorlog.h"
#include "../Exceptions.h"

// forward Declare

class RSC_GLShader;
class RSC_GLProgram;

class LEngineShaderException : public LEngineException {
 public:
  LEngineShaderException(const std::string &w, const RSC_GLShader *shad);

  const RSC_GLShader *shader;
};

class LEngineShaderProgramException : public LEngineException {
 public:
  LEngineShaderProgramException(const std::string &w,
                                const RSC_GLProgram *prog);

  const RSC_GLProgram *program;
};

bool CheckShaderCompileErrors(GLuint shader, std::string type);

enum L_GL_SHADER_TYPE {
  SHADER_FRAGMENT = GL_FRAGMENT_SHADER,
  SHADER_VERTEX = GL_VERTEX_SHADER,
  SHADER_GEOMETRY = GL_GEOMETRY_SHADER
};

// Will automatically allocate, compile, and free opengl Shader memory
class RSC_GLShader {
 public:
  RSC_GLShader(std::string filepath, L_GL_SHADER_TYPE type);
  ~RSC_GLShader();

  L_GL_SHADER_TYPE GetShaderType() const { return mShaderType; }
  GLuint GetShaderID() const { return mHandleID; }

  static std::string LoadShaderFromFile(const std::string &filepath);

  bool IsUsable();

 private:
  L_GL_SHADER_TYPE mShaderType;

  GLuint mHandleID;
  bool mUsable;
};

// This class owns nothing
class RSC_GLProgram {
 public:
  RSC_GLProgram();
  ~RSC_GLProgram();

  bool AddShader(const RSC_GLShader *shader);
  bool LinkProgram();

  GLuint GetHandle() const { return mHandleID; }

  // throws LEngineProgramException if name doesn't exist
  GLuint GetUniformBlockHandle(const std::string &name) const;

  // throws LEngineProgramException if name doesn't exist
  GLint GetUniformLocation(const std::string &name) const;

  void DeleteShaders();

  void Bind() const;

  static void BindNULL();
  static GLuint GetBoundProgram();

 private:
  const RSC_GLShader *mShaderFragment;
  const RSC_GLShader *mShaderVertex;
  const RSC_GLShader *mShaderGeometry;

  GLuint mHandleID;
  static GLuint currentlyBoundProgram;
};

#endif  // L_ENGINE_GL_SHADER
