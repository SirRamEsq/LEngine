#include "RSC_GLShader.h"
#include "../Kernel.h"
#include <math.h>
#include <memory>

LEngineShaderException::LEngineShaderException(const std::string& w, const RSC_GLShader* shad)
    : LEngineException(w), shader(shad){

}

LEngineShaderProgramException::LEngineShaderProgramException(const std::string& w, const RSC_GLProgram* prog)
    : LEngineException(w), program(prog){

}

std::string RSC_GLShader::LoadShaderFromFile(const std::string& filepath){
    std::string   filePath;
    std::string   fileString;

    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    K_Log.Write("L_GL_SHADER: LOADSHADERFROMFILE, reads files directly, may want it to use some kind of abstract file loader");
    try{
        //open files
        file.open(filepath.c_str());
        std::stringstream ss;

        //read the file buffer's contents into the string stream
        ss << file.rdbuf();

        //close the file
        file.close();

        fileString =   ss.str();
    }
    catch (std::ifstream::failure e){
        K_Log.Write("[C++] L_GL_SHADER(); FILE_NOT_SUCCESFULLY_READ");
        return "";
    }
    return fileString;
}

RSC_GLShader::RSC_GLShader(std::string glslCode, L_GL_SHADER_TYPE type){
    mUsable=false;

    //New shader ID
    mShaderType = type;
    mHandleID   = glCreateShader(mShaderType);

    //Define memory locations for these two variables so that glShaderSource can take their addresses
    const GLchar* code   = glslCode.c_str();
    const GLint   length = glslCode.length();

    //Compile
    glShaderSource(mHandleID, 1, &code, &length);
    glCompileShader(mHandleID);

    //Check if Usable
    mUsable=CheckShaderCompileErrors(mHandleID, "SHADER");
	if(mUsable == false){
		std::stringstream ss;
		ss << "Shader '" << mHandleID << "' of type '" << mShaderType << "' is unusable\n"
			<< "Source Code;\n" << glslCode; 
		K_Log.Write(ss.str(), Log::SEVERITY::ERROR);
	}
}

bool RSC_GLShader::IsUsable(){
	return mUsable;
}

RSC_GLShader::~RSC_GLShader(){
    //Free up the shader from openGl
    glDeleteShader(mHandleID);
    mHandleID=0;
}

//Checks for compile errors in either shader or program
bool CheckShaderCompileErrors(GLuint shader, std::string type){
    GLint success;
    GLchar infoLog[1024];
    if(shader==0){
        throw LEngineException("Shader passed has id of 0");
    }
    //Assuming that this is an individual shader (vert, frag, geo)
    if(type != "PROGRAM"){
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::stringstream ss;
            ss << "| ERROR::::SHADER-COMPILATION-ERROR of type: " << type << ", ID IS: " << shader << " |\n";

			ss
            << infoLog
            << "\n| -- --------------------------------------------------- -- |" << std::endl;
            K_Log.Write(ss.str());
            return false;
        }
    }
    //If you're checking for a shader program
    else{
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::stringstream ss;
            ss << "| ERROR::::PROGRAM-LINKING-ERROR of type: " << type << "|\n" << infoLog << "\n| -- --------------------------------------------------- -- |" << std::endl;
            K_Log.Write(ss.str());
            return false;
        }
    }
    return true;
}


GLuint RSC_GLProgram::currentlyBoundProgram=0;

RSC_GLProgram::RSC_GLProgram(){
    mHandleID=glCreateProgram();
    mShaderFragment=NULL;
    mShaderGeometry=NULL;
    mShaderVertex=NULL;
}
RSC_GLProgram::~RSC_GLProgram(){
    glDeleteProgram(mHandleID);
    mHandleID=0;
}

bool RSC_GLProgram::AddShader(const RSC_GLShader* shader){
    if(shader->GetShaderID()==0){
        K_Log.Write("ERROR: RSC_GLProgram: Shader passed has invalid handle of 0");
        return false;
    }
    switch (shader->GetShaderType()){
        case SHADER_FRAGMENT:
            if(mShaderFragment!=NULL){return false;}
            mShaderFragment=shader;
            break;

        case SHADER_VERTEX:
            if(mShaderVertex!=NULL){return false;}
            mShaderVertex=shader;
            break;

        case SHADER_GEOMETRY:
            if(mShaderGeometry!=NULL){return false;}
            mShaderGeometry=shader;
            break;
    }
    glAttachShader(mHandleID, shader->GetShaderID());

    return true;
}


void RSC_GLProgram::DeleteShaders(){
    if(mShaderFragment!=NULL){
        glDetachShader(mHandleID, mShaderFragment->GetShaderID());
        mShaderFragment=NULL;
    }

    if(mShaderVertex!=NULL){
        glDetachShader(mHandleID, mShaderVertex->GetShaderID());
        mShaderVertex=NULL;
    }

    if(mShaderGeometry!=NULL){
        glDetachShader(mHandleID, mShaderGeometry->GetShaderID());
        mShaderGeometry=NULL;
    }
}

bool RSC_GLProgram::LinkProgram(){
    const RSC_GLShader* shader;
    for(int i=0; i<3; i++){
        if     (i==0){shader=mShaderVertex;}
        else if(i==1){shader=mShaderFragment;}
        else if(i==2){shader=mShaderGeometry;}
        if(shader==NULL){continue;}

        GLuint shaderID=shader->GetShaderID();

        GLint compiled=0;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);

        if(compiled==0){
            GLint infoLength=0;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLength);
            if(infoLength > 0){
                std::unique_ptr<char> infoLog( new char[infoLength] );
                glGetShaderInfoLog(shader->GetShaderID(), infoLength, NULL, infoLog.get());
                std::string str(infoLog.get(), infoLength);
                K_Log.Write(str);
            }
        }
    }

    glLinkProgram(mHandleID);
    return CheckShaderCompileErrors(mHandleID, "PROGRAM");
}

void RSC_GLProgram::Bind() const {
    if(currentlyBoundProgram!=mHandleID){
        glUseProgram(mHandleID);
        currentlyBoundProgram=mHandleID;
    }
}

GLuint RSC_GLProgram::GetUniformBlockHandle(const std::string& name) const{
    GLuint returnVal=glGetUniformBlockIndex(mHandleID, name.c_str());
    if(returnVal==GL_INVALID_INDEX){
        std::stringstream ss;
        ss << "[C++] RSC_GLProgram::GetUniformBlockHandle; shader program with ID " << mHandleID
            << " doesn't have an active Uniform Block named " << name;
        K_Log.Write(ss.str(), Log::SEVERITY::INFO, Log::typeDefault);
        throw LEngineShaderProgramException(ss.str(), this);
    }
    return returnVal;
}

GLint RSC_GLProgram::GetUniformLocation(const std::string& name) const {
    GLint returnVal=glGetUniformLocation(mHandleID, name.c_str());
    if(returnVal==-1){
        std::stringstream ss;
        ss << "[C++] RSC_GLProgram::GetUniformLocation; shader program with ID " << mHandleID
            << " doesn't have an active Uniform Location named '" << name << "'";
        K_Log.Write(ss.str());
        throw LEngineShaderProgramException(ss.str(), this);
    }
    return returnVal;
}

void RSC_GLProgram::BindNULL(){
    glUseProgram(NULL);
    currentlyBoundProgram=0;
}
GLuint RSC_GLProgram::GetBoundProgram(){
    return currentlyBoundProgram;
}
