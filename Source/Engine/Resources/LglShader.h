#ifndef L_ENGINE_GL_SHADER
#define L_ENGINE_GL_SHADER

#include <GL/glew.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "../Errorlog.h"
#include "../Exceptions.h"

//forward Declare

class L_GL_Shader;
class L_GL_Program;

class LEngineShaderException : public LEngineException {
    public:
        LEngineShaderException(const std::string& w, const L_GL_Shader* shad);

        const L_GL_Shader* shader;
};

class LEngineShaderProgramException : public LEngineException {
    public:
        LEngineShaderProgramException(const std::string& w, const L_GL_Program* prog);

        const L_GL_Program* program;
};

bool CheckShaderCompileErrors(GLuint shader, std::string type);

enum L_GL_SHADER_TYPE{
    SHADER_FRAGMENT=GL_FRAGMENT_SHADER,
    SHADER_VERTEX=GL_VERTEX_SHADER,
    SHADER_GEOMETRY=GL_GEOMETRY_SHADER
};

//Will automatically allocate, compile, and free opengl Shader memory
class L_GL_Shader{
    public:
        L_GL_Shader(std::string filepath, L_GL_SHADER_TYPE type);
        ~L_GL_Shader();

        L_GL_SHADER_TYPE    GetShaderType   () const {return mShaderType;  }
        GLuint              GetShaderID     () const {return mHandleID;    }

        static std::string LoadShaderFromFile(const std::string& filepath);

    private:
        L_GL_SHADER_TYPE mShaderType;

        GLuint mHandleID;
        bool mUsable;
};

//This class owns nothing
class L_GL_Program{
    public:
        L_GL_Program();
        ~L_GL_Program();

        bool AddShader(const L_GL_Shader* shader);
        bool LinkProgram();

        GLuint GetHandle(){return mHandleID;}

        //throws LEngineProgramException if name doesn't exist
        GLuint GetUniformBlockHandle(const std::string& name);

        //throws LEngineProgramException if name doesn't exist
        GLint GetUniformLocation(const std::string& name);

        void DeleteShaders();

        void Bind();

        static void   BindNULL();
        static GLuint GetBoundProgram();

    private:
        const L_GL_Shader* mShaderFragment;
        const L_GL_Shader* mShaderVertex;
        const L_GL_Shader* mShaderGeometry;

        GLuint mHandleID;
        static GLuint currentlyBoundProgram;
};


#endif // L_ENGINE_GL_SHADER
