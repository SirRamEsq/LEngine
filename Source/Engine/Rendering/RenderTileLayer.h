#ifndef LENGINE_RENDERABLE_OBJECT_TLAYER
#define LENGINE_RENDERABLE_OBJECT_TLAYER

#include "RenderableObject.h"
#include "../Resources/RSC_Map.h"
#include "../glslHelper.h"

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

class RenderTileLayer : public RenderableObjectWorld{
    public:
        RenderTileLayer(RenderManager* rm, const TiledTileLayer* l);
        ~RenderTileLayer();

        void BuildVAO();
        void BuildVAOArea(Rect area);

        void Render(const RenderCamera* camera, const RSC_GLProgram* program);
        const TiledTileLayer* layer;

    private:
        void BuildVAOTile(unsigned int x, unsigned int y);

        unsigned int animatedRefreshRateTimer;
        bool animated;

        const TiledSet*  tiledSet;
        VAOWrapperTile vao;
};

#endif // LENGINE_RENDERABLE_OBJECT_TLAYER
