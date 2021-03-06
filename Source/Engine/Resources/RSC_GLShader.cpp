#include "RSC_GLShader.h"
#include "../Kernel.h"
#include "ResourceLoading.h"
#include <iostream>
#include <math.h>
#include <memory>

LEngineShaderException::LEngineShaderException(const std::string &w,
                                               const RSC_GLShader *shad)
    : LEngineException(w), shader(shad) {}

LEngineShaderProgramException::LEngineShaderProgramException(
    const std::string &w, const RSC_GLProgram *prog)
    : LEngineException(w), program(prog) {}

std::string RSC_GLShader::LoadShaderFromFile(const std::string &filepath) {
  try {
    std::string fullPath = "Resources/Shaders/" + filepath;
    auto shaderFile = LoadGenericFile(fullPath);
    if (shaderFile.get()->GetData() == NULL) {
      throw LEngineFileException("Shader is empty!", fullPath);
    }
    return std::string(shaderFile->GetData(), shaderFile->length);
  } catch (LEngineFileException e) {
    LOG_INFO(e.what());
    return "";
  }
}

std::unique_ptr<RSC_GLShader> RSC_GLShader::LoadResourceVertex(
    const std::string &filePath) {
  std::unique_ptr<RSC_GLShader> shader = NULL;
  try {
    std::string fullPath = "Resources/Shaders/" + filePath;
    auto data = LoadGenericFile(fullPath);
    if (data.get()->GetData() == NULL) {
      std::stringstream ss;
      ss << "GLShader " << fullPath << " couldn't be found.";
      LOG_ERROR(ss.str());
      return NULL;
    }
    std::string glslCode(data->GetData(), data->length);
    shader.reset(new RSC_GLShader(glslCode, SHADER_VERTEX));
  } catch (LEngineFileException e) {
    LOG_INFO(e.what());
    throw e;
  }

  return shader;
}
std::unique_ptr<RSC_GLShader> RSC_GLShader::LoadResourceFragment(
    const std::string &filePath) {
  std::unique_ptr<RSC_GLShader> shader = NULL;
  try {
    std::string fullPath = "Resources/Shaders/" + filePath;
    auto data = LoadGenericFile(fullPath);
    if (data.get()->GetData() == NULL) {
      std::stringstream ss;
      ss << "GLShader " << fullPath << " couldn't be found.";
      LOG_ERROR(ss.str());
      return NULL;
    }
    std::string glslCode(data->GetData(), data->length);
    shader.reset(new RSC_GLShader(glslCode, SHADER_FRAGMENT));
  } catch (LEngineFileException e) {
    LOG_INFO(e.what());
    throw e;
  }

  return shader;
}

RSC_GLShader::RSC_GLShader(const std::string &glslCode, L_GL_SHADER_TYPE type) {
  mUsable = false;

  // New shader ID
  mShaderType = type;
  mHandleID = glCreateShader(mShaderType);

  // Define memory location for these two variables so that glShaderSource can
  // take their addresses
  const GLchar *code = glslCode.c_str();
  const GLint leng = glslCode.length();

  // Compile
  glShaderSource(mHandleID, 1, &code, &leng);
  glCompileShader(mHandleID);

  // Check if Usable
  mUsable = CheckShaderCompileErrors(mHandleID, "SHADER");
  if (mUsable == false) {
    std::stringstream ss;
    ss << "Shader '" << mHandleID << "' of type '" << mShaderType
       << "' is unusable\n"
       << "Source Code;\n"
       << glslCode;
    LOG_ERROR(ss.str());
  }
}

bool RSC_GLShader::IsUsable() { return mUsable; }

RSC_GLShader::~RSC_GLShader() {
  // Free up the shader from openGl
  glDeleteShader(mHandleID);
  mHandleID = 0;
}

// Checks for compile errors in either shader or program
bool CheckShaderCompileErrors(GLuint shader, std::string type) {
  GLint success;
  GLchar infoLog[1024];
  if (shader == 0) {
    throw LEngineException("Shader passed has id of 0");
  }
  // Assuming that this is an individual shader (vert, frag, geo)
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      std::stringstream ss;
      ss << "| ERROR::::SHADER-COMPILATION-ERROR of type: " << type
         << ", ID IS: " << shader << " |\n";

      ss << infoLog
         << "\n| -- --------------------------------------------------- -- |"
         << std::endl;
      LOG_INFO(ss.str());
      return false;
    }
  }
  // If you're checking for a shader program
  else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      std::stringstream ss;
      ss << "| ERROR::::PROGRAM-LINKING-ERROR of type: " << type << "|\n"
         << infoLog
         << "\n| -- --------------------------------------------------- -- |"
         << std::endl;
      LOG_INFO(ss.str());
      return false;
    }
  }
  return true;
}

GLuint RSC_GLProgram::currentlyBoundProgram = 0;

RSC_GLProgram::RSC_GLProgram() {
  mLinked = false;
  mHandleID = glCreateProgram();
  mShaderFragment = NULL;
  mShaderGeometry = NULL;
  mShaderVertex = NULL;
}
RSC_GLProgram::RSC_GLProgram(const RSC_GLProgram *r) {
  mLinked = false;
  mHandleID = glCreateProgram();

  CopyShadersFromProgram(r);
}

RSC_GLProgram::~RSC_GLProgram() {
  DetatchShaders();
  glDeleteProgram(mHandleID);
  mHandleID = 0;
}

bool RSC_GLProgram::CopyShadersFromProgram(const RSC_GLProgram *program) {
  /// Exit if already linked
  if (mLinked) {
    return false;
  }
  glGetError();

  DetatchShaders();

  if (AddShader(program->mShaderVertex) == false) {
    std::string str = "Vertex shader could not be added";
    LOG_ERROR(str);
  }
  if (AddShader(program->mShaderFragment) == false) {
    std::string str = "Fragment shader could not be added";
    LOG_ERROR(str);
  }
  LinkProgram();

  return true;
}

bool RSC_GLProgram::AddShader(const RSC_GLShader *shader) {
  if (shader == NULL) {
    return false;
  }
  if (shader->GetShaderID() == 0) {
    LOG_INFO("ERROR: RSC_GLProgram: Shader passed has invalid handle of 0");
    return false;
  }
  switch (shader->GetShaderType()) {
    case SHADER_FRAGMENT:
      if (mShaderFragment != NULL) {
        return false;
      }
      mShaderFragment = shader;
      break;

    case SHADER_VERTEX:
      if (mShaderVertex != NULL) {
        return false;
      }
      mShaderVertex = shader;
      break;

    case SHADER_GEOMETRY:
      if (mShaderGeometry != NULL) {
        return false;
      }
      mShaderGeometry = shader;
      break;
  }
  glAttachShader(mHandleID, shader->GetShaderID());
  auto error = glGetError();
  if (error != GL_NO_ERROR) {
    if (error == GL_INVALID_VALUE) {
      std::string str =
          "Passed shader or program is not valid value generated by OpenGL";
      LOG_ERROR(str);
      // throw LEngineShaderProgramException(str, this);
    } else if (error == GL_INVALID_OPERATION) {
      std::string str =
          "Could not attach shader, has it been already attached?";
      LOG_ERROR(str);
    }
    return false;
  }

  return true;
}

void RSC_GLProgram::DetatchShaders() {
  if (mShaderFragment != NULL) {
    glDetachShader(mHandleID, mShaderFragment->GetShaderID());
    mShaderFragment = NULL;
  }

  if (mShaderVertex != NULL) {
    glDetachShader(mHandleID, mShaderVertex->GetShaderID());
    mShaderVertex = NULL;
  }

  if (mShaderGeometry != NULL) {
    glDetachShader(mHandleID, mShaderGeometry->GetShaderID());
    mShaderGeometry = NULL;
  }
}

bool RSC_GLProgram::LinkProgram() {
  const RSC_GLShader *shader;
  for (int i = 0; i < 3; i++) {
    if (i == 0) {
      shader = mShaderVertex;
    } else if (i == 1) {
      shader = mShaderFragment;
    } else if (i == 2) {
      shader = mShaderGeometry;
    }
    if (shader == NULL) {
      continue;
    }

    GLuint shaderID = shader->GetShaderID();

    GLint compiled = 0;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);

    if (compiled == 0) {
      GLint infoLength = 0;
      glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLength);
      if (infoLength > 0) {
        std::unique_ptr<char> infoLog(new char[infoLength]);
        glGetShaderInfoLog(shader->GetShaderID(), infoLength, NULL,
                           infoLog.get());
        std::string str(infoLog.get(), infoLength);
        LOG_INFO(str);
      }
    }
  }

  glLinkProgram(mHandleID);
  mLinked = true;
  return CheckShaderCompileErrors(mHandleID, "PROGRAM");
}

void RSC_GLProgram::Bind() const {
  if (currentlyBoundProgram != mHandleID) {
    glUseProgram(mHandleID);
    currentlyBoundProgram = mHandleID;
  }
}

GLuint RSC_GLProgram::GetUniformBlockHandle(const std::string &name) {
  GLuint returnVal = glGetUniformBlockIndex(mHandleID, name.c_str());
  if (returnVal == GL_INVALID_INDEX) {
    // not really an error, may have been compiled out
    std::stringstream ss;
    ss << "RSC_GLProgram::GetUniformBlockHandle; shader program with ID "
       << mHandleID << " doesn't have an active Uniform Block named " << name;
    LOG_TRACE(ss.str());
    throw LEngineShaderProgramException(ss.str(), this);
  }
  return returnVal;
}

GLint RSC_GLProgram::GetUniformLocation(const std::string &name) const {
  GLint returnVal = glGetUniformLocation(mHandleID, name.c_str());
  if (returnVal == -1) {
    // not really an error, may have been compiled out
    std::stringstream ss;
    ss << "RSC_GLProgram::GetUniformLocation; shader program with ID "
       << mHandleID << " doesn't have an active Uniform Location named '"
       << name << "'";
    LOG_TRACE(ss.str());
    // throw LEngineShaderProgramException(ss.str(), this);
  }
  return returnVal;
}

void RSC_GLProgram::BindNULL() {
  glUseProgram(NULL);
  currentlyBoundProgram = 0;
}
GLuint RSC_GLProgram::GetBoundProgram() { return currentlyBoundProgram; }

std::unique_ptr<RSC_GLProgram> RSC_GLProgram::LoadResourceFromXML(
    const std::string &xml) {
  const RSC_GLShader *shaders[2];
  try {
    using namespace rapidxml;
    xml_document<> doc;  // character type defaults to char
    doc.parse<0>((char *)(xml.c_str()));

    // Find Specific Node
    xml_node<> *node = doc.first_node("shaderProgram");
    if (node == NULL) {
      return NULL;
    }

    xml_node<> *nodeVertex = node->first_node("vertex");
    // xml_node<> *nodeGeometry = node->first_node("geometry");
    xml_node<> *nodeFragment = node->first_node("fragment");

    if (nodeVertex != NULL) {
      std::string shaderPath = nodeVertex->first_attribute("path")->value();
      auto shader = K_ShaderMan.GetItem(shaderPath);
      if (shader == NULL) {
        std::unique_ptr<const RSC_GLShader> newShader;
        newShader.reset(RSC_GLShader::LoadResourceVertex(shaderPath).release());
        if (newShader.get() != NULL) {
          K_ShaderMan.LoadItem(shaderPath, newShader);
          shader = K_ShaderMan.GetItem(shaderPath);
        }
        if (shader == NULL) {
          std::stringstream ss;
          ss << "Couldn't load shader with path " << shaderPath;
          LOG_WARN(ss.str());
        }
      }
      shaders[0] = shader;
    }
    if (nodeFragment != NULL) {
      std::string shaderPath = nodeFragment->first_attribute("path")->value();
      auto shader = K_ShaderMan.GetItem(shaderPath);
      if (shader == NULL) {
        std::unique_ptr<const RSC_GLShader> newShader;
        newShader.reset(
            RSC_GLShader::LoadResourceFragment(shaderPath).release());
        if (newShader.get() != NULL) {
          K_ShaderMan.LoadItem(shaderPath, newShader);
          shader = K_ShaderMan.GetItem(shaderPath);
        }
        if (shader == NULL) {
          std::stringstream ss;
          ss << "Couldn't load shader with path " << shaderPath;
          LOG_WARN(ss.str());
        }
      }
      shaders[1] = shader;
    }

  } catch (rapidxml::parse_error &e) {
    LOG_ERROR(e.what());
  }
  auto program = std::make_unique<RSC_GLProgram>();

  if (shaders[0] != NULL) {
    if(program->AddShader(shaders[0]) == false){
		LOG_ERROR("0");	
	}
  }
  if (shaders[1] != NULL) {
    if(program->AddShader(shaders[1]) == false){
		LOG_ERROR("1");	
	}
  }

  program->LinkProgram();

  return program;
}

std::unique_ptr<RSC_GLProgram> RSC_GLProgram::LoadResource(
    const std::string &filePath) {
  std::unique_ptr<RSC_GLProgram> program = NULL;
  try {
    std::string fullPath = "Resources/ShaderPrograms/" + filePath;
    auto data = LoadGenericFile(fullPath);
    if (data.get()->GetData() == NULL) {
      std::stringstream ss;
      ss << "GLProgram " << fullPath << " couldn't be found.";
      LOG_ERROR(ss.str());
      return NULL;
    }
    std::string xml(data->GetData(), data->length);
    program = RSC_GLProgram::LoadResourceFromXML(xml);
  } catch (LEngineFileException e) {
    LOG_INFO(e.what());
    throw e;
  }

  return program;
}
