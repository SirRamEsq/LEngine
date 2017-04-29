#ifndef L_RENDER_MANAGER
#define L_RENDER_MANAGER

#include <list>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include "Defines.h"
#include "SDLInit.h"
#include "glslHelper.h"

#include "Resources/LglShader.h"

/*
Centralized Rendering:
The rendermanger will know of all renderable objects (sprites, backgrounds, tiles, and text)
it will render them in order based on their depth
it will also control the camera
RenderableSprite automatically adds itself to the appropriate sprite batch when instantiated.

RenderManager owns only SpriteBatches, the rest are owned by whatever instantiates them
*/

class Background;
class ComponentSprite;
class LImage;
class TiledImageLayer;
class TiledTileLayer;
class TiledSet;
class RenderManager;
class RenderableObjectScreen;
class RenderableObjectWorld;
class RenderCamera;

class RenderableObject{
    friend RenderManager;
    friend RenderableObjectScreen;
    friend RenderableObjectWorld;

    public:
        RenderableObject();

        //Virtual destructor; enables derived classes to be fully deleted from a base RenderableObject pointer
        virtual ~RenderableObject(){}

        //All renderable objects will render using their own shader program if one is not passed to them
        virtual void Render(L_GL_Program* program) = 0;
                void Render(                     ) {Render(shaderProgram);}

        void        SetDepth(const MAP_DEPTH& d);
        MAP_DEPTH   GetDepth();

        void        SetRotation(const double& rot)  {rotation=rot;}
        double      GetRotation()                   {return rotation;}

        void        SetScalingY(const double& s)    {scaleY=s;}
        double      GetScalingY()                   {return scaleY;}

        void        SetScalingX(const double& s)    {scaleX=s;}
        double      GetScalingX()                   {return scaleX;}

        void        SetPositionX(const double& x)   {position.x=x;}
        double      GetPositionX()                  {return position.x;}

        void        SetPositionY(const double& y)   {position.y=y;}
        double      GetPositionY()                  {return position.y;}

        void        SetRender(const bool& s)        {render=s;}
        bool        GetRender()                     {return render;}

        L_COLOR     GetColor()                      {return color;}
        void        SetColor(L_COLOR c)             {color=c;}

        void           SetShaderProgram(L_GL_Program* p){shaderProgram=p;}
        L_GL_Program*  GetShaderProgram()               {return shaderProgram;}

        //void UpdateShaderProgram();

        bool        render;
        L_COLOR     color;

        std::string type;

    protected:
        virtual void AddToRenderManager() = 0;

    private:
        MAP_DEPTH   depth;
        double      scaleX, scaleY, rotation;
        Coord2df    position;

        L_GL_Program* shaderProgram;
};
class RenderableObjectScreen : public RenderableObject{
    public:
        RenderableObjectScreen();
        virtual ~RenderableObjectScreen();

        virtual void AddToRenderManager();
};
class RenderableObjectWorld : public RenderableObject{
    public:
        RenderableObjectWorld();
        virtual ~RenderableObjectWorld();

        virtual void AddToRenderManager();
};

class LTexture;

struct RenderableBufferData{
    Vec2 vertex1;
    Vec2 vertex2;
    Vec2 vertex3;
    Vec2 vertex4;

    Vec2 vertexOrigin1;
    Vec2 vertexOrigin2;
    Vec2 vertexOrigin3;
    Vec2 vertexOrigin4;

    Vec2 texture1;
    Vec2 texture2;
    Vec2 texture3;
    Vec2 texture4;

    Vec4 color;
};
struct RenderableBufferDataSprite{
    Vec2 vertex1;
    Vec2 vertex2;
    Vec2 vertex3;
    Vec2 vertex4;

    Vec2 vertexOrigin1;
    Vec2 vertexOrigin2;
    Vec2 vertexOrigin3;
    Vec2 vertexOrigin4;

    Vec2 texture1;
    Vec2 texture2;
    Vec2 texture3;
    Vec2 texture4;

    Vec4 color;
    Vec3 scalingRotation;
    Vec2 translate;
};

class RenderSpriteBatch;
class RenderableSprite{
    public:
        RenderableSprite(const std::string& texture, const unsigned int& w, const unsigned int& h, const MAP_DEPTH& d, const Vec2& off=Vec2(0.0f,0.0f)); //Have class auto register with a sprite batch;
        ~RenderableSprite();

        RenderableBufferDataSprite  data;
        bool                        isActive;
        std::string                 textureName;
        MAP_DEPTH                   depth;

        unsigned int        textureWidth;
        unsigned int        textureHeight;
        float               scaleX;
        float               scaleY;
        float               rotation;
        Vec2                offset;

        RenderSpriteBatch*  spriteBatch;
};

class VAOWrapper{
    public:
        //Max size is the number of objects, not the number of verticies.
        //VAOWrapper will figure out the number of vertiices from the number of objects
        VAOWrapper(const unsigned int& maxSize);
        ~VAOWrapper();
        void UpdateGPU();

        Vec2*  GetVertexArray         (){return vboVertexArray.get();         }
        Vec2*  GetTextureArray        (){return vboTextureArray.get();        }
        Vec4*  GetColorArray          (){return vboColorArray.get();          }

        GLuint GetVAOID               (){return vao;                          }


    private:
        GLuint vboVertex;
        GLuint vboTexture;
        GLuint vboColor;
        GLuint vao;

        std::unique_ptr< Vec2 [] > vboVertexArray         ;
        std::unique_ptr< Vec2 [] > vboTextureArray        ;
        std::unique_ptr< Vec4 [] > vboColorArray          ;

        unsigned int vboVertexSize;
        unsigned int vboTextureSize;
        unsigned int vboColorSize;

        const unsigned int vboMaxSize;

        //Each vertex point consists of 2 floats                                [X,Y] (vec2)
        const GLint vertexAttributeSize         = 2;
        const GLenum vertexAttributeType        = GL_FLOAT;

        //Each texture coordinate for each vertex point consists of 2 floats    [S,T] (vec2)
        const GLint textureAttributeSize        = 2;
        const GLenum textureAttributeType       = GL_FLOAT;

        //Each Color for each vertex point consists of 4 floats                 [RGBA], (vec4)
        const GLint colorAttributeSize          = 4;
        const GLenum colorAttributeType         = GL_FLOAT;
};

class VAOWrapperTile{
    //make uniform for texture width and height

    public:
        //Max size is the number of objects, not the number of verticies.
        //VAOWrapper will figure out the number of vertiices from the number of objects
        VAOWrapperTile(const unsigned int& maxSize);
        ~VAOWrapperTile();
        void UpdateGPU();

        Vec2*  GetVertexArray         (){return vboVertexArray.get();         }
        Vec4*  GetTextureArray        (){return vboTextureArray.get();        }
        Vec2*  GetAnimationArray      (){return vboAnimationArray.get();      }
        Vec4*  GetColorArray          (){return vboColorArray.get();          }

        GLuint GetVAOID               (){return vao;                          }


    private:
        GLuint vboVertex;
        GLuint vboTexture;
        GLuint vboColor;
        GLuint vboAnimation;
        GLuint vao;

        std::unique_ptr< Vec2 [] > vboVertexArray         ;
        std::unique_ptr< Vec4 [] > vboTextureArray        ;
        std::unique_ptr< Vec2 [] > vboAnimationArray      ;
        std::unique_ptr< Vec4 [] > vboColorArray          ;

        unsigned int vboVertexSize;
        unsigned int vboTextureSize;
        unsigned int vboColorSize;
        unsigned int vboAnimationSize;

        const unsigned int vboMaxSize;

        //Each vertex point consists of 2 floats                                [X,Y] (vec2)
        const GLint vertexAttributeSize         = 2;
        const GLenum vertexAttributeType        = GL_FLOAT;

        //Each texture coordinate for each vertex point consists of 4 floats    [S,T, texW,texH] (vec2)
        const GLint textureAttributeSize        = 4;
        const GLenum textureAttributeType       = GL_FLOAT;

        //Each Color for each vertex point consists of 4 floats                 [RGBA], (vec4)
        const GLint colorAttributeSize          = 4;
        const GLenum colorAttributeType         = GL_FLOAT;

        //Each animation consists of animation speed and max frames             [aniSpeed, aniFrames], (vec2)
        const GLint animationAttributeSize          = 2;
        const GLenum animationAttributeType         = GL_FLOAT;
};

class VAOWrapperSprite{
    public:
        //Max size is the number of objects, not the number of verticies.
        //VAOWrapper will figure out the number of vertiices from the number of objects
        VAOWrapperSprite(const unsigned int& maxSize);
        ~VAOWrapperSprite();
        void UpdateGPU();

        Vec2*  GetVertexArray         (){return vboVertexArray.get();         }
        Vec2*  GetTranslateArray      (){return vboTranslateArray.get();      }
        Vec2*  GetTextureArray        (){return vboTextureArray.get();        }
        Vec4*  GetColorArray          (){return vboColorArray.get();          }
        Vec3*  GetScalingRotationArray(){return vboScalingRotationArray.get();}

        GLuint GetVAOID               (){return vao;                          }


    private:
        GLuint vboVertex;
        GLuint vboTranslate;
        GLuint vboTexture;
        GLuint vboColor;
        GLuint vboScalingRotation;
        GLuint vao;

        std::unique_ptr< Vec2 [] > vboVertexArray         ;
        std::unique_ptr< Vec2 [] > vboTranslateArray         ;
        std::unique_ptr< Vec2 [] > vboTextureArray        ;
        std::unique_ptr< Vec4 [] > vboColorArray          ;
        std::unique_ptr< Vec3 [] > vboScalingRotationArray;

        unsigned int vboVertexSize;
        unsigned int vboTextureSize;
        unsigned int vboColorSize;
        unsigned int vboTranslateSize;
        unsigned int vboScalingRotationSize;

        const unsigned int vboMaxSize;

        //Each vertex point consists of 2 floats                                [X,Y] (vec2)
        const GLint vertexAttributeSize         = 2;
        const GLenum vertexAttributeType        = GL_FLOAT;

        //Each texture coordinate for each vertex point consists of 2 floats    [S,T] (vec2)
        const GLint textureAttributeSize        = 2;
        const GLenum textureAttributeType       = GL_FLOAT;

        //Each Color for each vertex point consists of 4 floats                 [RGBA], (vec4)
        const GLint colorAttributeSize          = 4;
        const GLenum colorAttributeType         = GL_FLOAT;

        //Each ScalingRotation for each vertex point consists of 3 floats       [Sx, Sy, rot], (vec3)
        const GLint  scalingRotationAttributeSize         = 3;
        const GLenum scalingRotationAttributeType         = GL_FLOAT;

        //Each ScalingRotation for each vertex point consists of 2 floats       [X, Y], (vec2)
        const GLint  translateAttributeSize         = 2;
        const GLenum translateAttributeType         = GL_FLOAT;
};

class RenderSpriteBatch : public RenderableObjectWorld{
    //Class constructor is protected, only meant to be initialized by RenderManager
    //Class does NOT own RenderableSprites;
    friend class RenderManager;

    public:
        void Render(L_GL_Program* program);

        void    AddSprite      (RenderableSprite* sprite);
        void    DeleteSprite   (RenderableSprite* sprite);
        bool    CanAddSprites  (const int& numSprites);

    protected:
        RenderSpriteBatch(const std::string& tex, const unsigned int& maxSize);

    private:
        VAOWrapperSprite vao;
        const unsigned int maxSprites;
              unsigned int currentSize;

        const std::string textureName;
        const LTexture*   texture;

        std::set<RenderableSprite*> sprites;
};

class LTexture;
class RenderText : public RenderableObjectScreen{
    public:
        RenderText(int xv, int yv, std::string t, bool abso=true);
        ~RenderText();

        void SetColorI(int r, int g, int b){
            color.r=r;
            color.g=g;
            color.b=b;
            del=true;
        }
        void SetColor(SDL_Color c){
            color=c;
            del=true;
        }

        SDL_Color GetColor(){return color;}

        void ChangeText(std::string newText){
            del=true;
            text=newText;
        }
        void ChangePosition(int xx, int yy){
            SetX(xx);
            SetY(yy);
        }
        void SetX(int xx){x=xx;}
        void SetY(int yy){y=yy;}
        void SetW(int ww){w=ww;}
        void SetH(int hh){h=hh;}

        int GetX(){return x;}
        int GetY(){return y;}
        int GetW(){return w;}
        int GetH(){return h;}

        void Render(L_GL_Program* program);

        void DeleteTexture();

    private:
        std::string text;
        int x, y, w, h;
        bool absolute, del;
        SDL_Color color;
        LTexture* mTexture;
        GLuint VBOID;
        GLuint IBOID;
};
class RenderLine : public RenderableObjectScreen{
    public:
        RenderLine(int x1, int y1, int x2, int y2, bool absolute=true);
        ~RenderLine();

        void SetColorI(int r, int g, int b){
            color.r=r;
            color.g=g;
            color.b=b;
        }
        inline void SetColor(SDL_Color c){
            color=c;
        }

        inline SDL_Color GetColor(){return color;}

        void ChangePosition(int x1, int y1, int x2, int y2){
            SetX1(x1);
            SetY1(y1);
            SetX2(x2);
            SetY2(y2);
        }
        void SetX1(int xx){mX1=xx;}
        void SetY1(int yy){mY1=yy;}
        void SetX2(int xx){mX2=xx;}
        void SetY2(int yy){mY2=yy;}

        int GetX1(){return mX1;}
        int GetY1(){return mY1;}
        int GetX2(){return mX2;}
        int GetY2(){return mY2;}

        void Render(L_GL_Program* program);

    private:
        int mX1, mY1, mX2, mY2;
        bool mAbsolute;
        SDL_Color color;
};

class RenderBox : public RenderableObjectScreen{
    public:
        RenderBox(int x1, int y1, int x2, int y2, bool absolute=true);
        ~RenderBox();

        void SetColorI(int r, int g, int b){
            color.r=r;
            color.g=g;
            color.b=b;
        }
        inline void SetColor(SDL_Color c){
            color=c;
        }

        inline SDL_Color GetColor(){return color;}

        void ChangePosition(int x1, int y1, int x2, int y2){
            SetX1(x1);
            SetY1(y1);
            SetX2(x2);
            SetY2(y2);
        }
        void SetX1(int xx){mX1=xx;}
        void SetY1(int yy){mY1=yy;}
        void SetX2(int xx){mX2=xx;}
        void SetY2(int yy){mY2=yy;}

        int GetX1(){return mX1;}
        int GetY1(){return mY1;}
        int GetX2(){return mX2;}
        int GetY2(){return mY2;}

        void Render(L_GL_Program* program);

    private:
        int mX1, mY1, mX2, mY2;
        bool mAbsolute;
        SDL_Color color;
};

class RenderTiledTileLayer : public RenderableObjectWorld{
    public:
        RenderTiledTileLayer(const TiledTileLayer* l);
        ~RenderTiledTileLayer();

        void BuildVAO();
        void BuildVAOArea(CRect area);

        void Render(L_GL_Program* program);
        const TiledTileLayer* layer;

    private:
        void BuildVAOTile(unsigned int x, unsigned int y);

        unsigned int animatedRefreshRateTimer;
        bool animated;

        const TiledSet*  tiledSet;
        VAOWrapperTile vao;
};

class RenderTiledImageLayer : public RenderableObjectWorld{
    public:
        RenderTiledImageLayer(TiledImageLayer* l);
        ~RenderTiledImageLayer();

        void BuildVAO();

        void Render(L_GL_Program* program);
        const TiledImageLayer* layer;

    private:
        //unsigned int animatedRefreshRateTimer;
        //bool animated;

        VAOWrapper vao;
};

class ComponentCamera;
class RenderCamera{
    friend class RenderManager;
    friend class ComponentCamera;

    public:
        RenderCamera();
        ~RenderCamera();

        void    SetScaling (double s)   {scale=s;        }
        void    SetRotation(double r)   {rotation=r;     }
        double  GetScaling ()           {return scale;   }
        double  GetRotation()           {return rotation;}

        int            GetX       (){return view.x;}
        int            GetY       (){return view.y;}
        const CRect&   GetViewRect(){return view;  }

        void          Bind             (const GLuint& GlobalCameraUBO);

        void UpdatePosition();
        void RenderFrameBufferTextureFinal();

    protected:
        CRect view;

        LTexture* GetFrameBufferTextureDiffuse(){return frameBufferTextureDiffuse.get();}
        LTexture* GetFrameBufferTextureFinal(){return frameBufferTextureFinal.get();}

    private:
        Vec2 translation;

        //Cameras can have different shaders!!!

        double rotation;
        double scale;

        MAP_DEPTH nearClippingPlane;
        MAP_DEPTH farClippingPlane;

        double normalizedViewportX; //between 0.0f and 1.0f
        double normalizedViewportY; //determines where the FBO will be rendered to.

        GLuint FBO; //frame buffer object id

        bool mActive;

        std::unique_ptr<LTexture>    frameBufferTextureDiffuse;
        std::unique_ptr<LTexture>    frameBufferTextureFinal;
};

class RenderManager{
    friend RenderableObjectScreen;
    friend RenderableObjectWorld;
    friend RenderCamera;
    public:
        RenderManager();

        void Render();

        void OrderOBJs();
        void MoveCameraX    (int x);
        void MoveCameraY    (int y);
        void MoveCameraXAbs (int x);
        void MoveCameraYAbs (int y);

        void AssignCameraUBO(L_GL_Program* program);

        RenderSpriteBatch* GetSpriteBatch(const std::string& textureName, const MAP_DEPTH& depth, const int& numSprites); //returns sprite batch that supports 'textureName' and has room for at least numSprites more room

        MAP_DEPTH nextTextID;

    protected:
        bool AddCamera(RenderCamera* cam);
        void RemoveCamera(RenderCamera* cam);

        void AddObjectScreen    (RenderableObjectScreen* obj);
        void RemoveObjectScreen (RenderableObjectScreen* obj);

        void AddObjectWorld     (RenderableObjectWorld* obj);
        void RemoveObjectWorld  (RenderableObjectWorld* obj);

    private:
        bool                                listChange;
        std::list<RenderableObjectWorld*>   objectsWorld;
        std::list<RenderableObjectScreen*>  objectsScreen;
        std::set<RenderCamera*>             mCameras;

        std::string shaderFragmentNameSpriteBatch;
        std::string shaderVertexNameSpriteBatch;
        L_GL_Program shaderProgramSpriteBatch;

        std::string shaderFragmentNameTileLayer;
        std::string shaderVertexNameTileLayer;
        L_GL_Program shaderProgramTileLayer;

        std::string shaderFragmentNameImage;
        std::string shaderVertexNameImage;
        L_GL_Program shaderProgramImage;

        std::string shaderFragmentNameLight;
        std::string shaderVertexNameLight;
        L_GL_Program shaderProgramLight;

        static GLuint GlobalCameraUBO;
        const GLuint CameraDataBindingIndex;

        static GLuint GlobalProgramUBO;
        const GLuint ProgramDataBindingIndex;

        int spriteBatchMaxSize;
        std::map<MAP_DEPTH, std::map< std::string, std::vector < std::unique_ptr <RenderSpriteBatch> > > >  spriteBatchMap; //map each sprite batch to a texture name and depth value

        //Time since creation of RenderManager
        unsigned int timeElapsed=0;
};

#endif
