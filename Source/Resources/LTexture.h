#ifndef L_ENGINE_TEX_MAN
#define L_ENGINE_TEX_MAN

#include <string>
#include <memory>

#include "SDL2/SDL.h"
#include <SOIL/SOIL.h>

#include "../Errorlog.h"
#include "../Defines.h"

class LTexture{
    public:
        class Exception : public LEngineException{using LEngineException::LEngineException;};

        void SetColorKey(unsigned char Red, unsigned char Green, unsigned char Blue, bool useCurrentAlpha=false);

        int GetWidth()       const {return mTexData.width;}
        int GetHeight()      const {return mTexData.height;}
        GLuint GetOpenGLID() const {return mGLID;}

        //Set as active texture in OpenGL
        void Bind() const;

        std::string GetName() const {return mFileName;};

        //This function copies the data passed to it
        LTexture(const unsigned char* data, unsigned int dsize, const std::string& fname);
        LTexture(const SDL_Surface& source);
        LTexture(const std::string& fName);
        LTexture(unsigned int width, unsigned int height, int BPP, int format);
        ~LTexture();

        uint8_t GetPixelAlpha (const int& x, const int& y) const;

        bool ExportTexture    (const char* path) const;

        int RenderToTexture (const CRect &area, LTexture* otherTexture, LOrigin origin=L_ORIGIN_CENTER) const;
        void BlitArea       (const CRect &area, LOrigin origin=L_ORIGIN_CENTER) const;

        void Clear();
        void UpdateDataFromGL();

        static void BindNull();

        static std::unique_ptr<LTexture> LoadResource(const std::string& fname);

    private:
        void DeleteGLTexture();
        void LoadFile(const std::string& fName);

        // Structure that contains the information about the texture.
        struct TextureData{
            int   width;
            int   height;
            // array containing raw data
            //unsigned char* data;
            std::unique_ptr<unsigned char[]> data;
            int channels;//stores the original image's number channels (ex. RGBA, RGB, Greyscale, etc...)
        };
        int mBytesPerPixel, mTextureFormat;
        TextureData mTexData;

        // The openGL id of this texture
        mutable GLuint mGLID;
        static GLuint GLBoundTexture;

        std::string mFileName;
};

#endif
