#include "LglShader.h"
#include <math.h>
#include <memory>

LEngineShaderException::LEngineShaderException(const std::string& w, const L_GL_Shader* shad)
    : LEngineException(w), shader(shad){

}

LEngineShaderProgramException::LEngineShaderProgramException(const std::string& w, const L_GL_Program* prog)
    : LEngineException(w), program(prog){

}

std::string L_GL_Shader::LoadShaderFromFile(const std::string& filepath){
    std::string   filePath;
    std::string   fileString;

    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    ErrorLog::WriteToFile("L_GL_SHADER: LOADSHADERFROMFILE, reads files directly, may want it to use some kind of abstract file loader",
                        ErrorLog::GenericLogFile);
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
        ErrorLog::WriteToFile("[C++] L_GL_SHADER(); FILE_NOT_SUCCESFULLY_READ", ErrorLog::GenericLogFile);
        return "";
    }
    return fileString;
}

L_GL_Shader::L_GL_Shader(std::string glslCode, L_GL_SHADER_TYPE type){
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
}
L_GL_Shader::~L_GL_Shader(){
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
            ss << "| ERROR::::SHADER-COMPILATION-ERROR of type: " << type << ", ID IS: " << shader << " |\n"
            << infoLog
            << "\n| -- --------------------------------------------------- -- |" << std::endl;
            ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
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
            ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
            return false;
        }
    }
    return true;
}


GLuint L_GL_Program::currentlyBoundProgram=0;

L_GL_Program::L_GL_Program(){
    mHandleID=glCreateProgram();
    mShaderFragment=NULL;
    mShaderGeometry=NULL;
    mShaderVertex=NULL;
}
L_GL_Program::~L_GL_Program(){
    glDeleteProgram(mHandleID);
    mHandleID=0;
}

bool L_GL_Program::AddShader(const L_GL_Shader* shader){
    if(shader->GetShaderID()==0){
        ErrorLog::WriteToFile("ERROR: L_GL_Program: Shader passed has invalid handle of 0", ErrorLog::GenericLogFile);
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


void L_GL_Program::DeleteShaders(){
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

bool L_GL_Program::LinkProgram(){
    const L_GL_Shader* shader;
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
                std::unique_ptr<char> infoLog( new char (infoLength));
                glGetShaderInfoLog(shader->GetShaderID(), infoLength, NULL, infoLog.get());
                std::string str(infoLog.get(), infoLength);
                ErrorLog::WriteToFile(str, ErrorLog::GenericLogFile);
            }
        }
    }

    glLinkProgram(mHandleID);
    return CheckShaderCompileErrors(mHandleID, "PROGRAM");
}

void L_GL_Program::Bind(){
    if(currentlyBoundProgram!=mHandleID){
        glUseProgram(mHandleID);
        currentlyBoundProgram=mHandleID;
    }
}

GLuint L_GL_Program::GetUniformBlockHandle(const std::string& name){
    GLuint returnVal=glGetUniformBlockIndex(mHandleID, name.c_str());
    if(returnVal==GL_INVALID_INDEX){
        std::stringstream ss;
        ss << "[C++] L_GL_Program::GetUniformBlockHandle; shader program with ID " << mHandleID
            << " doesn't have an active Uniform Block named " << name;
        ErrorLog::WriteToFile(ss.str(), ErrorLog::SEVERITY::INFO, ErrorLog::GenericLogFile);
        throw LEngineShaderProgramException(ss.str(), this);
    }
    return returnVal;
}

GLint L_GL_Program::GetUniformLocation(const std::string& name){
    GLint returnVal=glGetUniformLocation(mHandleID, name.c_str());
    if(returnVal==-1){
        std::stringstream ss;
        ss << "[C++] L_GL_Program::GetUniformLocation; shader program with ID " << mHandleID
            << " doesn't have an active Uniform Location named '" << name << "'";
        ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
        throw LEngineShaderProgramException(ss.str(), this);
    }
    return returnVal;
}

void L_GL_Program::BindNULL(){
    glUseProgram(NULL);
    currentlyBoundProgram=0;
}
GLuint L_GL_Program::GetBoundProgram(){
    return currentlyBoundProgram;
}
