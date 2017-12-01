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
  RSC_GLShader(const std::string &glslCode, L_GL_SHADER_TYPE type);
  ~RSC_GLShader();

  L_GL_SHADER_TYPE GetShaderType() const { return mShaderType; }
  GLuint GetShaderID() const { return mHandleID; }

  static std::string LoadShaderFromFile(const std::string &filepath);
  static std::unique_ptr<RSC_GLShader> LoadResource(
      const std::string &filePath);

  bool IsUsable();

 private:
  L_GL_SHADER_TYPE mShaderType;

  GLuint mHandleID;
  bool mUsable;
};

/**
 * This class owns nothing
 * Class can be used with GenericContainer like other resources
 * Uniform locations are not const functions, however, so grabbing
 * a program from the Kernel GenericContainer will not allow you to set uniforms
 * instead, copy the program given to you from the kernel and set up uniforms on
 * that
 *
 * Long story short: If you're using uniforms, COPY the shared
 * (GenericContainer) program
 */
class RSC_GLProgram {
 public:
  RSC_GLProgram();
  RSC_GLProgram(const RSC_GLProgram *r);
  ~RSC_GLProgram();

  bool AddShader(const RSC_GLShader *shader);
  bool LinkProgram();

  GLuint GetHandle() const { return mHandleID; }

  // throws LEngineProgramException if name doesn't exist
  GLuint GetUniformBlockHandle(const std::string &name);

  // throws LEngineProgramException if name doesn't exist
  GLint GetUniformLocation(const std::string &name) const;

  void DeleteShaders();

  void Bind() const;

  static void BindNULL();
  static GLuint GetBoundProgram();

  static std::unique_ptr<RSC_GLProgram> LoadResource(
      const std::string &filePath);
  static std::unique_ptr<RSC_GLProgram> LoadResourceFromXML(
      const std::string &xml);

 private:
  const RSC_GLShader *mShaderFragment;
  const RSC_GLShader *mShaderVertex;
  const RSC_GLShader *mShaderGeometry;

  GLuint mHandleID;
  static GLuint currentlyBoundProgram;
};

#endif  // L_ENGINE_GL_SHADER
