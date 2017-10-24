#include "RSC_Texture.h"

#include "../Defines.h"
#include "../Kernel.h"
#include "../Resolution.h"
#include "ResourceLoading.h"
#include <cstring>
#include <string>

GLuint RSC_Texture::GLBoundTexture = 0;

// This function copies the data passed to it
RSC_Texture::RSC_Texture(const unsigned char *data, unsigned int dsize,
                         const std::string &fname) {
  mFileName = fname;
  mGLID = 0;

  mTexData.data.reset(SOIL_load_image_from_memory(
      data, dsize, &mTexData.width,
      &mTexData.height,    // Data is sent into these
      &mTexData.channels,  // number of channels loaded in image
      SOIL_LOAD_RGBA));
  mBytesPerPixel = 4;  // desired number of channels for this texture
  mTextureFormat = GL_RGBA;
}

RSC_Texture::RSC_Texture(unsigned int width, unsigned int height, int BPP,
                         int format) {
  mFileName = "";
  mGLID = 0;
  int texSize = width * height * BPP;
  mTexData.width = width;
  mTexData.height = height;
  mTexData.data.reset(
      new unsigned char[texSize]);  // 0; //this will make opengl
                                    // initialze an empty image
                                    // texture

  mBytesPerPixel = BPP;
  mTextureFormat = format;
}

RSC_Texture::RSC_Texture(const SDL_Surface &source) {
  mFileName = "";
  mGLID = 0;

  mTexData.width = source.w;
  mTexData.height = source.h;

  const SDL_PixelFormat &format = *(source.format);
  int bpp = format.BytesPerPixel;
  int textureFormat;

  if (bpp == 4) {  // alpha
    if (format.Rmask == 0x000000ff) {
      textureFormat = GL_RGBA;
    } else {
      textureFormat = GL_BGRA;
    }
  } else {  // no alpha
    if (format.Rmask == 0x000000ff) {
      textureFormat = GL_RGB;
    } else {
      textureFormat = GL_BGR;
    }
  }

  int texSize = source.w * source.h * bpp;
  mTexData.data.reset(new unsigned char[texSize]);
  std::memcpy(mTexData.data.get(), source.pixels, texSize);

  mTextureFormat = textureFormat;
  mBytesPerPixel = bpp;
}

RSC_Texture::RSC_Texture(const std::string &fName) {
  mFileName = fName;
  mTexData.height = mTexData.width = 0;
  mTexData.data.reset(NULL);
  LoadFile(fName);
}

void RSC_Texture::DeleteGRSC_Texture() {
  if (mGLID != 0) {
    glDeleteTextures(1, &mGLID);
    mGLID = 0;
  }
}

void RSC_Texture::UpdateDataFromGL() {
  Bind();
  glGetTexImage(GL_TEXTURE_2D, 0, mTextureFormat, GL_UNSIGNED_BYTE,
                mTexData.data.get());
}

bool RSC_Texture::ExportTexture(const char *path) const {
  auto NotConstHack = ((RSC_Texture *)(this));
  NotConstHack->UpdateDataFromGL();

  if (SOIL_save_image(path, SOIL_SAVE_TYPE_BMP, mTexData.width, mTexData.height,
                      mBytesPerPixel, mTexData.data.get()) == 0) {
    std::string pathString(path);
    std::string errorString =
        "Texture Write to Disk Failed @ Path: " + pathString;
    LOG_INFO(errorString);
    throw Exception(errorString);
  }
}

RSC_Texture::~RSC_Texture() { DeleteGRSC_Texture(); }

void RSC_Texture::LoadFile(const std::string &fName) {
  // Generate a new image Id and bind it with the
  // current image.

  int *channels;  // stores the original image's number channels (ex. RGBA, RGB,
                  // Greyscale, etc...)
  mTexData.data.reset(SOIL_load_image(
      fName.c_str(), (int *)&mTexData.width, (int *)&mTexData.height,
      channels,        // Data is sent into these
      SOIL_LOAD_RGBA)  // MAY NOT WANT TO INVERT Y
                      );
  mBytesPerPixel = 4;
  mTextureFormat = GL_RGBA;
}

void RSC_Texture::Bind() const {
  if (mGLID == 0) {
    // Generate one new texture Id.
    glGenTextures(1, &mGLID);

    if (mGLID == 0) {  // error
      auto glError = glGetError();
      std::string glMessage =
          reinterpret_cast<const char *>(gluErrorString(glError));
      std::string exceptionMessage =
          "OpenGL cannot generate texture ID; Failed with error \"" +
          glMessage + "\"";
      if (glError == GL_INVALID_OPERATION) {
        exceptionMessage += "\nIs OpenGL Initialized?";
      }
      throw Exception(exceptionMessage);
    }

    // Make this texture active
    glBindTexture(GL_TEXTURE_2D, mGLID);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // REPEAT TEXTURES
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Sets drawing mode to GL_MODULATE
    // should only be called once
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Finally, generate the texture data in OpenGL.
    glTexImage2D(GL_TEXTURE_2D, 0, mBytesPerPixel, mTexData.width,
                 mTexData.height, 0, mTextureFormat, GL_UNSIGNED_BYTE,
                 mTexData.data.get());

    // Once the data is passed off to opengl, the current mTexData.data is just
    // a copy
  }
  if (mGLID != RSC_Texture::GLBoundTexture) {
    glBindTexture(GL_TEXTURE_2D, mGLID);
    RSC_Texture::GLBoundTexture = mGLID;
  }
}

void RSC_Texture::BindNull() {
  glBindTexture(GL_TEXTURE_2D, 0);
  RSC_Texture::GLBoundTexture = 0;
}

void RSC_Texture::SetColorKey(unsigned char Red, unsigned char Green,
                              unsigned char Blue, bool useCurrentAlpha) {
  // Delete the old texture if one exists
  if ((mTexData.data.get()[3] == 0) and (useCurrentAlpha)) {
    return;
  }  // Exit if it already has an alpha value
  if (mGLID != 0) {
    glDeleteTextures(1, &mGLID);
    mGLID = 0;
  }

  // For all the pixels that correspond to the specifed color, set the alpha
  // channel to 0 (transparent) and reset the other
  // alphas to 255
  unsigned long Count = mTexData.width * mTexData.height * 4;
  for (unsigned long i = 0; i < Count; i += 4) {
    if ((mTexData.data.get()[i] == Red) &&
        (mTexData.data.get()[i + 1] == Green) &&
        (mTexData.data.get()[i + 2] == Blue)) {
      mTexData.data.get()[i + 3] = 0;
    } else {
      mTexData.data.get()[i + 3] = 255;
    }
  }
}

uint8_t RSC_Texture::GetPixelAlpha(const int &x, const int &y) const {
  if (x >= mTexData.width) {
    std::stringstream ss;
    ss << "[C++] RSC_Texture:GetPixelAlpha; Texture X index out of bounds"
       << "\n    X is: " << x << "\n    W is: " << mTexData.width;
    LOG_INFO(ss.str());

  } else if (y >= mTexData.height) {
    std::stringstream ss;
    ss << "[C++] RSC_Texture:GetPixelAlpha; Texture Y index out of bounds"
       << "\n    Y is: " << y << "\n    H is: " << mTexData.height;
    LOG_INFO(ss.str());
  } else {
    int index = (x + (y * mTexData.width)) * 4;  // get Correct pixel index
    index += 3;                                  // Get Alpha value of pixel
    return mTexData.data.get()[index];
  }
  return 0;
}

int RSC_Texture::RenderToTexture(const Rect &area, RSC_Texture *otherTexture,
                                 LOrigin origin) const {
  // Use GLtranslate before calling this function to set the x/y values
  return true;
  // Create FBO
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // Attach Texture
  otherTexture->Bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         otherTexture->GetOpenGLID(), 0);

  GLint errorcode = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (errorcode != GL_FRAMEBUFFER_COMPLETE) {
    LOG_INFO("ERROR: Framebuffer not complete, error code: " + errorcode);
    return false;
  }

  glViewport(0, 0, otherTexture->GetWidth(), otherTexture->GetHeight());
  glMatrixMode(
      GL_PROJECTION);  // Select projection matrix operations to set the
                       // ortho perspective
  glLoadIdentity();
  glScalef(1.f, -1.f, 1.f);
  glOrtho(0.0f, otherTexture->GetWidth(), otherTexture->GetHeight(), 0, 1, -1);

  glMatrixMode(GL_MODELVIEW);  // reselect modelview matrix

  // Render
  BlitArea(area);

  // Delete FBO and cleanup
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fbo);

  auto resolution = Resolution::GetResolution();
  auto vp = Resolution::GetVirtualResolution();

  glViewport(0, 0, resolution.x, resolution.y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, vp.x, vp.y, 0, 1, -1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  return true;
}

void RSC_Texture::BlitArea(const Rect &are, LOrigin origin) const {
  Bind();
  Rect area = are;
  float Left = (float)area.GetLeft() / (float)mTexData.width;
  float Right = (float)area.GetRight() / (float)mTexData.width;
  float Top = (float)area.GetTop() / (float)mTexData.height;
  float Bottom = (float)area.GetBottom() / (float)mTexData.height;

  glBegin(GL_QUADS);
  glTexCoord2f(Left, Top);
  glVertex3i(0, 0, 0);
  glTexCoord2f(Right, Top);
  glVertex3i(area.w, 0, 0);
  glTexCoord2f(Right, Bottom);
  glVertex3i(area.w, area.h, 0);
  glTexCoord2f(Left, Bottom);
  glVertex3i(0, area.h, 0);
  glEnd();
}

void RSC_Texture::Clear() {
  Bind();
  glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

std::unique_ptr<RSC_Texture> RSC_Texture::LoadResource(
    const std::string &fname) {
  std::unique_ptr<RSC_Texture> texture = NULL;
  try {
    std::string fullPath = "Resources/Images/" + fname;
    auto data = LoadGenericFile(fullPath);
    if (data.get()->GetData() == NULL) {
      std::stringstream ss;
      ss << "Texture " << fullPath << " couldn't be found.";
      LOG_ERROR(ss.str());
      return NULL;
    }
    texture = std::make_unique<RSC_Texture>(
        (const unsigned char *)data.get()->GetData(), data.get()->length,
        fname);
    texture->SetColorKey(MASK_R, MASK_G, MASK_B, true);
  } catch (LEngineFileException e) {
    LOG_INFO(e.what());
    throw e;
  }

  // ensures texture has an ID
  texture->Bind();

  return texture;
}
