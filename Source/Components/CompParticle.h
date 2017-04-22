#ifndef L_ENGINE_COMP_PARTICLE
#define L_ENGINE_COMP_PARTICLE

#include <map>

#include "../BaseComponent.h"
#include "../Defines.h"
#include "../Resources/LSprite.h"
#include "../RenderManager.h"
#include "../Random.h"

#include "CompPosition.h"

typedef int PARTICLE_SHAPE;
    const PARTICLE_SHAPE PARTICLE_SQUARE   = 1;
    const PARTICLE_SHAPE PARTICLE_RING     = 2;
    const PARTICLE_SHAPE PARTICLE_CIRCLE   = 4;

typedef int PARTICLE_EFFECT;
    const PARTICLE_EFFECT PARTICLE_NONE       = 0;
    const PARTICLE_EFFECT PARTICLE_FADE_IN    = 1;
    const PARTICLE_EFFECT PARTICLE_FADE_OUT   = 2;
    const PARTICLE_EFFECT PARTICLE_SHRINK     = 4;
    const PARTICLE_EFFECT PARTICLE_EXPAND     = 8;

enum PARTICLE_CREATOR_STATE{
    PARTICLE_CREATOR_STARTING=1,
    PARTICLE_CREATOR_SUSTAINING=2,
    PARTICLE_CREATOR_STOPPING=4,
    PARTICLE_CREATOR_STOPPED=8
};

struct ParticleVertexData{
    Vec2 position;
    Vec2 texture;
    Vec2 velocity;
    Vec2 acceleration;
    Vec4 color;
    Vec2 lifeTime; //lifeTime.x is the timeStamp at which the particle was created, lifeTime.y is the timeStamp to be destroyed at
    Vec2 scaling; //used mainly for fancy effects like shrink / expand
};

class ComponentParticle;
class ParticleCreator : public RenderableObjectWorld{
    friend ComponentParticle;
    public:
        ParticleCreator (const unsigned int& particleLife, const bool& useSprite, const std::string& logFile);
        ~ParticleCreator();

        void Render(L_GL_Program* program);

        //void SetSprite  (LSprite* spr);

        void SetVelocity        (const Coord2df& velocityMin,  const Coord2df& velocityMax);
        void SetAcceleration    (const Coord2df& accelMin,     const Coord2df& accelMax   );
        //This position is relative the the position componentŔ
        void SetPosition        (const Coord2df& posMin,       const Coord2df& posMax     );

        void SetScalingX(const float& xscaleMin, const float& xscaleMax);
        void SetScalingY(const float& yscaleMin, const float& yscaleMax);
        //void SetRotation(float rotMin, float rotMax);
        void SetColor   (const float& rMin, const float& gMin, const float& bMin, const float& aMin,
                         const float& rMax, const float& gMax, const float& bMax, const float& aMax);

        //void SetAnimationSpeed(float speedMin, float speedMax);

        void SetParticlesPerFrame(const float& particles);

        void SetFragmentShaderCode(const std::string& code);
        void SetVertexShaderCode  (const std::string& code);
        void SetShape (const PARTICLE_SHAPE& shape);
        void SetEffect(const PARTICLE_EFFECT& effect);

        //Starts the particle Creator, changed parameters do not take effect till stopping and starting the creator
        void Start();
        void Stop();

        void Update();

    private:
        void WriteData(const unsigned int& writeLocation, const unsigned int& writeSize);

        bool active;

        PARTICLE_CREATOR_STATE mState;
        PARTICLE_SHAPE         mShape;
        PARTICLE_EFFECT        mEffect;

        L_GL_Shader* GenerateVertexShader  ();
        L_GL_Shader* GenerateFragmentShader();

        std::unique_ptr<L_GL_Program> mShaderProgram;
        RandomGenerator mRandom;

        unsigned int mParticleLifeSpan; //Time until particles end
        unsigned int mLifeSpan;         //Time until particle Creator end

        LSprite* mSprite;
        bool     mSpriteUsage;

        L_COLOR mDefaultColorMin;
        L_COLOR mDefaultColorMax;

        Coord2df mDefaultVelocityMin;
        Coord2df mDefaultAccelerationMin;
        Coord2df mDefaultPositionMin;

        //These values will be added or subtracted randomly from the defaults for each particle
        Coord2df mDefaultVelocityMax;
        Coord2df mDefaultAccelerationMax;
        Coord2df mDefaultPositionMax;

        float mXScalingMin, mYScalingMin;
        float mXScalingMax, mYScalingMax;
        float mRotationMin, mRotationMax;

        float mAnimationSpeedMin, mAnimationSpeedMax;

        float        mParticlesPerFrame;
        float        mCurrentParticleIndex;
        //The following variables are derived from 'mParticleIndex'
        unsigned int mParticlesToRender;
        unsigned int mMaxParticles;
        //Describes which ParticleVertexData to read/write from, not exactly which byte;
        unsigned int mParticleBufferReadLocation;
        unsigned int mParticleBufferWriteLocation;

        bool         mUseSprite;
        unsigned int vboBufferSize;
        unsigned int mTime;
        GLint        mTimeUniformLocation;
        const int VerticiesPerParticle;

        GLuint vbo;
        GLuint vao;

        std::unique_ptr< ParticleVertexData [] > vboData;
        ComponentPosition* myPos;

        //For custom shader code from scripting language
        std::string mScriptShaderCodeFragment;
        std::string mScriptShaderCodeVertex;

        //For Error Log
        const std::string logFileName;
};

class ComponentParticle : public BaseComponent{
    public:
         ComponentParticle(EID id,  ComponentPosition* pos, const std::string& logFile);
        ~ComponentParticle();

        void Update     ();
        void HandleEvent(const Event* event);

        ParticleCreator* AddParticleCreator    (const unsigned int& creatorLife, const unsigned int& particleLife);
        void             DeleteParticleCreators();

    private:
        std::vector< std::unique_ptr<ParticleCreator> > mParticleCreators;

        ComponentPosition* myPos;
};

class ComponentParticleManager : public BaseComponentManager{
    public:
        ComponentParticleManager();
        ~ComponentParticleManager();

        void Update();
        void AddComponent(EID id);
        void HandleEvent(const Event* event){}

    private:

};

extern const std::string PARTICLE_SHADER_VERTEX_DECLARATIONS;
extern const std::string PARTICLE_SHADER_VERTEX_MAIN_BEGIN;
extern const std::string PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_BEGIN;
extern const std::string PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_EFFECT_SHRINK;
extern const std::string PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_EFFECT_EXPAND;
extern const std::string PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_END;
extern const std::string PARTICLE_SHADER_VERTEX_MAIN_END;


extern const std::string PARTICLE_SHADER_FRAGMENT_DECLARATIONS;
extern const std::string PARTICLE_SHADER_FRAGMENT_MAIN_BEGIN;

extern const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_BEGIN;

extern const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_SHAPE_CIRCLE;
extern const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_SHAPE_RING;
extern const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_SHAPE_SQUARE;

extern const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_EFFECT_FADE_IN;
extern const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_EFFECT_FADE_OUT;


extern const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_END;

extern const std::string PARTICLE_SHADER_FRAGMENT_MAIN_END;


#endif // L_ENGINE_COMP_PARTICLE
