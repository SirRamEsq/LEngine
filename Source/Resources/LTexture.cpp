#include "LTexture.h"

#include <cstring>

GLuint LTexture::GLBoundTexture=0;

//This function copies the data passed to it
LTexture::LTexture(const unsigned char* data, unsigned int dsize, const std::string& fname){
    mFileName=fname;
    mGLID=0;

	mTexData.data.reset(SOIL_load_image_from_memory(data,
                                    dsize,
                                    &mTexData.width, &mTexData.height, //Data is sent into these
                                    &mTexData.channels,//number of channels loaded in image
                                    SOIL_LOAD_RGBA)
                        );
    mBytesPerPixel=4;//desired number of channels for this texture
    mTextureFormat=GL_RGBA;
}

LTexture::LTexture(unsigned int width, unsigned int height, int BPP, int format){
    mFileName="";
    mGLID=0;
    int texSize=width*height*BPP;
    mTexData.width=width;
    mTexData.height=height;
    mTexData.data.reset(new unsigned char [texSize]);//0; //this will make opengl initialze an empty image texture

    mBytesPerPixel=BPP;
    mTextureFormat=format;
}

LTexture::LTexture(const SDL_Surface& source){
    mFileName="";
    mGLID=0;

	mTexData.width=source.w;
	mTexData.height=source.h;

    const SDL_PixelFormat& format=*(source.format);
    int bpp=format.BytesPerPixel;
    int textureFormat;

    if (bpp == 4) {   // alpha
        if (format.Rmask == 0x000000ff){
            textureFormat = GL_RGBA;
        }
        else{
            textureFormat = GL_BGRA;
        }
    }
    else {             // no alpha
        if (format.Rmask == 0x000000ff){
            textureFormat = GL_RGB;
        }
        else{
            textureFormat = GL_BGR;
        }
    }

    int texSize=source.w * source.h * bpp;
    mTexData.data.reset(new unsigned char[texSize]);
    std::memcpy(mTexData.data.get(), source.pixels, texSize);


    mTextureFormat=textureFormat;
    mBytesPerPixel=bpp;
}

LTexture::LTexture(const std::string& fName){
    mFileName=fName;
    mTexData.height = mTexData.width = 0;
	mTexData.data.reset(NULL);
    LoadFile(fName);
}

void LTexture::DeleteGLTexture(){
    if(mGLID!=0){
        glDeleteTextures(1, &mGLID);
        mGLID = 0;
    }
}

void LTexture::UpdateDataFromGL(){
    Bind();
    glGetTexImage(GL_TEXTURE_2D,0,mTextureFormat,GL_UNSIGNED_BYTE, mTexData.data.get());
}

bool LTexture::ExportTexture(const char* path) const {
    //UpdateDataFromGL();
    if(SOIL_save_image(path,SOIL_SAVE_TYPE_BMP,mTexData.width,mTexData.height,mBytesPerPixel,mTexData.data.get())==0){
        ErrorLog::WriteToFile("Texture Write to Disk Failed", ErrorLog::GenericLogFile);
    }
}

LTexture::~LTexture(){
    DeleteGLTexture();
}

void LTexture::LoadFile(const std::string& fName){
	// Generate a new image Id and bind it with the
	// current image.

	int* channels; //stores the original image's number channels (ex. RGBA, RGB, Greyscale, etc...)
	mTexData.data.reset(SOIL_load_image(fName.c_str(),
                                (int*)&mTexData.width, (int*)&mTexData.height, channels, //Data is sent into these
                                SOIL_LOAD_RGBA) //MAY NOT WANT TO INVERT Y
                        );
    mBytesPerPixel=4;
    mTextureFormat=GL_RGBA;
}

void LTexture::Bind() const {
    if(mGLID==0){
		// Generate one new texture Id.
		glGenTextures(1,&mGLID);

		// Make this texture active
		glBindTexture(GL_TEXTURE_2D,mGLID);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		//REPEAT TEXTURES
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Sets drawing mode to GL_MODULATE
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		// Finally, generate the texture data in OpenGL.
		glTexImage2D(GL_TEXTURE_2D, 0, mBytesPerPixel, mTexData.width, mTexData.height,
				0,mTextureFormat,GL_UNSIGNED_BYTE,mTexData.data.get());

        //Once the data is passed off to opengl, the current mTexData.data is just a copy
    }
    if(mGLID!=LTexture::GLBoundTexture){
        glBindTexture(GL_TEXTURE_2D,mGLID);
        LTexture::GLBoundTexture=mGLID;
    }
}

void LTexture::BindNull(){
    glBindTexture(GL_TEXTURE_2D,0);
    LTexture::GLBoundTexture=0;
}

void LTexture::SetColorKey(unsigned char Red, unsigned char Green, unsigned char Blue, bool useCurrentAlpha){
	// Delete the old texture if one exists
	if( (mTexData.data.get()[3]==0) and (useCurrentAlpha) ){return;}//Exit if it already has an alpha value
	if (mGLID!=0){
		glDeleteTextures(1,&mGLID);
		mGLID = 0;
	}

	// For all the pixels that correspond to the specifed color, set the alpha channel to 0 (transparent) and reset the other
	// alphas to 255
	unsigned long Count = mTexData.width * mTexData.height * 4;
	for (unsigned long i = 0; i<Count; i+=4){
		if ( (mTexData.data.get()[i]==Red) && (mTexData.data.get()[i+1]==Green) && (mTexData.data.get()[i+2]==Blue) ){
			mTexData.data.get()[i+3] = 0;
		}
		else{
			mTexData.data.get()[i+3] = 255;
		}
	}
}

uint8_t LTexture::GetPixelAlpha(const int& x, const int& y) const{
    if(x>=mTexData.width){
        std::stringstream ss;
        ss << "[C++] LTexture:GetPixelAlpha; Texture X index out of bounds" <<
                "\n    X is: " << x <<
                "\n    W is: " << mTexData.width;
        ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);

    }
    else if(y>=mTexData.height){
        std::stringstream ss;
        ss << "[C++] LTexture:GetPixelAlpha; Texture Y index out of bounds" <<
                "\n    Y is: " << y <<
                "\n    H is: " << mTexData.height;
        ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
    }
    else{
        int index=(x+(y*mTexData.width)) * 4; //get Correct pixel index
        index+=3;//Get Alpha value of pixel
        return mTexData.data.get()[index];
    }
    return 0;
}

int LTexture::RenderToTexture(const CRect& area, LTexture* otherTexture, LOrigin origin)const {
    //Use GLtranslate before calling this function to set the x/y values

    //Create FBO
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    //Attach Texture
    otherTexture->Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, otherTexture->GetOpenGLID(), 0);

    GLint errorcode=glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(errorcode != GL_FRAMEBUFFER_COMPLETE){
        ErrorLog::WriteToFile("ERROR: Framebuffer not complete, error code: " + std::to_string(errorcode), ErrorLog::GenericLogFile);
        return false;
    }

    glViewport(0,0, otherTexture->GetWidth(),otherTexture->GetHeight());
    glMatrixMode(GL_PROJECTION); //Select projection matrix operations to set the ortho perspective
    glLoadIdentity();
    glScalef(1.f, -1.f, 1.f);
    glOrtho(0.0f, otherTexture->GetWidth(),otherTexture->GetHeight(), 0, 1, -1);

    glMatrixMode(GL_MODELVIEW); //reselect modelview matrix

    //Render
    BlitArea(area);

    //Delete FBO and cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    glViewport(0, 0, SCREEN_W, SCREEN_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, CAMERA_W, CAMERA_H, 0, 1, -1);
    glMatrixMode(GL_MODELVIEW);

    return true;
}

void LTexture::BlitArea(const CRect& are, LOrigin origin) const{
    Bind();
    CRect area=are;
    float Left=     (float)area.GetLeft()   / (float)mTexData.width;
    float Right=    (float)area.GetRight()  / (float)mTexData.width;
    float Top=      (float)area.GetTop()    / (float)mTexData.height;
    float Bottom=   (float)area.GetBottom() / (float)mTexData.height;

    glBegin(GL_QUADS);
        glTexCoord2f(Left,  Top);		glVertex3i(0,       0,      0);
        glTexCoord2f(Right, Top);	    glVertex3i(area.w,  0,      0);
        glTexCoord2f(Right, Bottom);	glVertex3i(area.w,  area.h, 0);
        glTexCoord2f(Left,  Bottom);	glVertex3i(0,       area.h, 0);
    glEnd();
}

void LTexture::Clear(){
    Bind();
    glClearColor( 1.f, 0.f, 1.f, 1.f );
    glClear(GL_COLOR_BUFFER_BIT);
}
