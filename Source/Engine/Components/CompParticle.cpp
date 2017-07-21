#include "CompParticle.h"
#include <math.h>

/*
The CPU will determine how many Particles to render based on its state (STARTING, STOPPING, SUSTAINING, etc...)
and by how much time has passed since having been started
It will also insert 'new' particles (new particles being freshly seeded random values) into the GPU buffer (is treated as a ring buffer)
Each Particle Creator will have two draw calls, one for the head of the buffer and one for the tail.
*/

////////////////////
//Particle Creator//
////////////////////

ParticleCreator::ParticleCreator(RenderManager* rm, const unsigned int& particleLife, const bool& useSprite, const std::string& logFile)
    : dependencyRenderManager(rm), RenderableObjectWorld(dependencyRenderManager), mParticleLifeSpan(particleLife), mUseSprite(useSprite), VerticiesPerParticle(4), logFileName(logFile){

    mScriptShaderCodeVertex="";
    mScriptShaderCodeFragment="";
    mShape=PARTICLE_SQUARE;
    mEffect=PARTICLE_NONE;
    mTime=0;
    mParticlesPerFrame=1;
    mSprite=NULL;
    mParticlesToRender=0;

    mParticlesToRender=0;
    mRandom.SetSeed(mRandom.GenerateSeed());

    L_COLOR c (1.0f,1.0f,1.0f,1.0f);
    mDefaultColorMax=c;
    mDefaultColorMin=c;
    mState=PARTICLE_CREATOR_STOPPED;
    vboBufferSize=0;

    mParticleBufferReadLocation=0;
    mParticleBufferWriteLocation=0;
    AddToRenderManager();
}

ParticleCreator::~ParticleCreator(){

}

void ParticleCreator::Start(){
    mTime=0;
    mCurrentParticleIndex=0;
    mParticlesToRender=0;
    //Only one vertex per object; the rest are created in a geometry shader;
    //Getting ceiling of this rounded number
    mMaxParticles=ceil( (mParticleLifeSpan * mParticlesPerFrame));

    //Get size of VBO in bytes
    vboBufferSize = mMaxParticles * sizeof ( ParticleVertexData ) * VerticiesPerParticle;

    //Assign smart pointer to new data buffer
    vboData.reset(new ParticleVertexData [mMaxParticles * VerticiesPerParticle] );



    //Generate and set shader program
    mShaderProgram.reset(new RSC_GLProgram);

    mShaderProgram->AddShader( GenerateVertexShader() );
    mShaderProgram->AddShader( GenerateFragmentShader() );

    mShaderProgram->LinkProgram();
    mShaderProgram->Bind();
    SetShaderProgram(mShaderProgram.get());

    //Set up time uniform (which is a float)
    mTimeUniformLocation = mShaderProgram->GetUniformLocation("time");
    glUniform1f(mTimeUniformLocation, ((float)(mTime)) );



    //Vertex VBO
	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
                            //     Size of Buffer                           Pointer to data
	glBufferData (GL_ARRAY_BUFFER, vboBufferSize,    vboData.get(), GL_STATIC_DRAW);

    unsigned int stride=sizeof(ParticleVertexData);

    //Generate VAO
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);

	//Bind Position to 0
	glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);

	//Bind Texture to 1
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, stride, (void*)8);

	//Bind Velocity to 2
	glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, stride, (void*)16);

	//Bind Acceleration to 3
	glVertexAttribPointer (3, 2, GL_FLOAT, GL_FALSE, stride, (void*)24);

	//Bind Color to 4
	glVertexAttribPointer (4, 4, GL_FLOAT, GL_FALSE, stride, (void*)32);

	//Bind LifeTime to 5
	glVertexAttribPointer (5, 2, GL_FLOAT, GL_FALSE, stride, (void*)48);

	//Bind Scaling to 6
	glVertexAttribPointer (6, 2, GL_FLOAT, GL_FALSE, stride, (void*)56);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);
	glEnableVertexAttribArray (2);
	glEnableVertexAttribArray (3);
	glEnableVertexAttribArray (4);
	glEnableVertexAttribArray (5);
	glEnableVertexAttribArray (6);

	//Set up Camera UBO
	dependencyRenderManager->AssignCameraUBO(mShaderProgram.get());

    //The particle creator will begin generating particles, but this state indicates that
    //it hasn't hit the maximum number of particles yet
    mState=PARTICLE_CREATOR_STARTING;

    mRandom.SetSeed(mRandom.GenerateSeed());

    //Write entire buffer
    //WriteData(0, mMaxParticles);
}

void ParticleCreator::SetVertexShaderCode(const std::string& code){
    mScriptShaderCodeVertex=code;
}

void ParticleCreator::SetFragmentShaderCode(const std::string& code){
    mScriptShaderCodeFragment=code;
}

void ParticleCreator::SetShape(const PARTICLE_SHAPE& shape){
    mShape=shape;
}

void ParticleCreator::SetEffect(const PARTICLE_EFFECT& effect){
    mEffect=effect;
}

RSC_GLShader* ParticleCreator::GenerateFragmentShader(){
    //Particle Shape
    //default is square
    std::string particleShape=PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_SHAPE_SQUARE;
    switch(mShape){
        case PARTICLE_CIRCLE:   particleShape=PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_SHAPE_CIRCLE; break;
        case PARTICLE_RING:     particleShape=PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_SHAPE_RING;   break;
    }

    //Particle Effects
    //default is nothing
    std::string particleEffect="";
    switch(mEffect){
        case PARTICLE_FADE_IN:   particleEffect=PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_EFFECT_FADE_IN;    break;
        case PARTICLE_FADE_OUT:  particleEffect=PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_EFFECT_FADE_OUT;   break;
    }

    //Create Script Generated Shader Code
    std::stringstream scriptCodeFragment;
    if(mScriptShaderCodeFragment==""){
        scriptCodeFragment
        << PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_BEGIN
        << particleShape
        << particleEffect
        << PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_END;
    }
    else{
        scriptCodeFragment << mScriptShaderCodeFragment;
    }
    std::stringstream ss;
    ss
    << PARTICLE_SHADER_FRAGMENT_DECLARATIONS
    << PARTICLE_SHADER_FRAGMENT_MAIN_BEGIN
    << scriptCodeFragment.str()
    << PARTICLE_SHADER_FRAGMENT_MAIN_END
    ;

    std::unique_ptr<RSC_GLShader> shader (new RSC_GLShader(ss.str(), SHADER_FRAGMENT));

    ss << "\n\n\n";
    ErrorLog::WriteToFile(ss.str(), logFileName);

    return shader.release();
}

RSC_GLShader* ParticleCreator::GenerateVertexShader(){
    std::stringstream ss;
    ss
    << PARTICLE_SHADER_VERTEX_DECLARATIONS
    << PARTICLE_SHADER_VERTEX_MAIN_BEGIN
    << PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_BEGIN
    << PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_EFFECT_SHRINK
    //<< PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_EFFECT_EXPAND
    << PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_END
    << PARTICLE_SHADER_VERTEX_MAIN_END
    ;
    std::unique_ptr<RSC_GLShader> shader (new RSC_GLShader(ss.str(), SHADER_VERTEX));

    ss << "\n\n\n";
    ErrorLog::WriteToFile(ss.str(), logFileName);

    return shader.release();
}

void ParticleCreator::SetParticlesPerFrame(const float& particles){
    if(particles>0){
        mParticlesPerFrame=particles;
        return;
    }

    mParticlesPerFrame=0;
}

void ParticleCreator::SetColor(const float& rMin, const float& gMin, const float& bMin, const float& aMin,
                                const float& rMax, const float& gMax, const float& bMax, const float& aMax){
    L_COLOR cMin(rMin,gMin,bMin,aMax);
    L_COLOR cMax(rMax,gMax,bMax,aMax);

    mDefaultColorMin=cMin;
    mDefaultColorMax=cMax;
}

/*void ParticleCreator::SetShape(PARTICLE_SHAPE shape){
    mShape=shape;
}

void ParticleCreator::SetSprite(RSC_Sprite* spr){
    mSprite=NULL;
}
*/

void ParticleCreator::SetPosition(const Coord2df& positionMin, const Coord2df& positionMax){
    mDefaultPositionMin=positionMin;
    mDefaultPositionMax=positionMax;
}

void ParticleCreator::SetVelocity(const Coord2df& velocityMin, const Coord2df& velocityMax){
    mDefaultVelocityMin=velocityMin;
    mDefaultVelocityMax=velocityMax;
}

void ParticleCreator::SetAcceleration(const Coord2df& accelMin, const Coord2df& accelMax){
    mDefaultAccelerationMin=accelMin;
    mDefaultAccelerationMax=accelMax;
}

void ParticleCreator::SetScalingX(const float& xscaleMin, const float& xscaleMax){
    mXScalingMin=xscaleMin;
    mXScalingMax=xscaleMax;
}
void ParticleCreator::SetScalingY(const float& yscaleMin, const float& yscaleMax){
    mYScalingMin=yscaleMin;
    mYScalingMax=yscaleMax;
}

/*
void ParticleCreator::SetRotation(double rotationMin, double rotationMax){
    mRotationMin=rotationMin;
    mRotationMax=rotationMax;
}

void ParticleCreator::SetAnimationSpeed(double speedMin, double speedMax){
    mAnimationSpeedMin=speedMin;
    mAnimationSpeedMax=speedMax;
}

void ParticleCreator::SetParticlesPerFrame(float particles){
    mParticlesPerFrame=particles;
}
*/

//Location and Size is in particles, not bytes or verticies
void ParticleCreator::WriteData(const unsigned int& writeLocation, const unsigned int& writeSize){
    bool ringLoop=false; //whether there is one single contiguous memory buffer or two

    /*
    A weird side effect of using a ring buffer is that buffer[0] is always drawn first
    That means that if all of the old particles are overlapping it, you won't see buffer[0] (or any other overlapped particle)
    when its first created
    It may look like the creator stopped generating particles momentarily
    This is especially noticable if the particles are especially large or slow
    */

    unsigned int writeLocationVertex = writeLocation * 4; //translate particle to actual vertex index
    unsigned int writeSizeVertex = writeSize * 4;
    unsigned int maxParticlesVertex = mMaxParticles * 4;

    for(int i=0; i<writeSize; i++){
        //Treat buffer as Ring
        if( writeLocationVertex >= maxParticlesVertex ){
            writeLocationVertex -= maxParticlesVertex;
            ringLoop=true; //Need to update two buffers, head and tail
        }
        Vec2 position(mRandom.GenerateRandomFloatValue( mDefaultPositionMin.x,
                                                        mDefaultPositionMax.x),
                    mRandom.GenerateRandomFloatValue(   mDefaultPositionMin.y,
                                                        mDefaultPositionMax.y)
                        );

        Vec2 acceleration(mRandom.GenerateRandomFloatValue( mDefaultAccelerationMin.x,
                                                            mDefaultAccelerationMax.x),
                        mRandom.GenerateRandomFloatValue(   mDefaultAccelerationMin.y,
                                                            mDefaultAccelerationMax.y)
                        );

        Vec2 velocity(mRandom.GenerateRandomFloatValue( mDefaultVelocityMin.x,
                                                        mDefaultVelocityMax.x),
                    mRandom.GenerateRandomFloatValue(   mDefaultVelocityMin.y,
                                                        mDefaultVelocityMax.y)
                        );

        Vec2 scaling(mRandom.GenerateRandomFloatValue( mXScalingMin,
                                                        mXScalingMax),
                    mRandom.GenerateRandomFloatValue(   mYScalingMin,
                                                        mYScalingMax)
                        );


        Vec4 color( mRandom.GenerateRandomFloatValue( mDefaultColorMin.mR,
                                                    mDefaultColorMax.mR),
                    mRandom.GenerateRandomFloatValue( mDefaultColorMin.mG,
                                                    mDefaultColorMax.mG),
                    mRandom.GenerateRandomFloatValue( mDefaultColorMin.mB,
                                                    mDefaultColorMax.mB),
                    mRandom.GenerateRandomFloatValue( mDefaultColorMin.mA,
                                                    mDefaultColorMax.mA)
                            );

        for(int vert=0; vert<4; vert++){
            ParticleVertexData& data = vboData.get()[writeLocationVertex + vert];

            Coord2df pos=myPos->GetPositionWorld();
            Vec2 offset(0,0);
            Vec2 tex(0,0);
            Vec2 worldPos(pos.x, pos.y);
            if(vert==0){offset.x=-1 * scaling.x; offset.y=-1 * scaling.y;     tex.x= -1; tex.y= -1;}
            if(vert==1){offset.x= 1 * scaling.x; offset.y=-1 * scaling.y;     tex.x=  1; tex.y= -1;}
            if(vert==2){offset.x= 1 * scaling.x; offset.y= 1 * scaling.y;     tex.x=  1; tex.y=  1;}
            if(vert==3){offset.x=-1 * scaling.x; offset.y= 1 * scaling.y;     tex.x= -1; tex.y=  1;}

            data.position      = position + offset + worldPos;
            data.texture       = tex;
            data.velocity      = velocity;
            data.acceleration  = acceleration;
            data.color         = color;
            data.lifeTime.x    = mTime;
            data.lifeTime.y    = mTime + mParticleLifeSpan;
            data.scaling       = scaling;
        }
        //advance by 4 ParticleVerticies
        writeLocationVertex+=4;
    }
    //Update GPU side Data
    if(ringLoop){

    }
    else{

    }
    //For testing purposes, just update the whole dang buffer
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboBufferSize, vboData.get());
}

void ParticleCreator::Stop(){
    mParticleBufferReadLocation=mParticleBufferWriteLocation + 1;

    mState=PARTICLE_CREATOR_STOPPING;
}

void ParticleCreator::Update(){
    if      (mState==PARTICLE_CREATOR_STOPPED){return;}

    //Treat as ring buffer
    if (mCurrentParticleIndex >= mMaxParticles){
        mCurrentParticleIndex -= mMaxParticles;
    }
    //Set the write location to the current particle index
    mParticleBufferWriteLocation=floor (mCurrentParticleIndex);

    //Update Index
    int indexBefore = floor (mCurrentParticleIndex);
    mCurrentParticleIndex += mParticlesPerFrame;
    int indexAfter = floor (mCurrentParticleIndex);

    //This is the number of particles that need updated this frame (starting from the write buffer) to reach the new index
    int indexDifference= abs(indexAfter - indexBefore);

    if((mState==PARTICLE_CREATOR_STARTING)or(mState==PARTICLE_CREATOR_SUSTAINING)){
        WriteData(mParticleBufferWriteLocation, indexDifference);
    }

    //Increment time
    mTime++;

    //The particle creator will begin generating particles, but this state indicates that
    //it hasn't hit the maximum number of particles yet
    if (mState==PARTICLE_CREATOR_STARTING){
        mParticlesToRender+=indexDifference;
        if(mParticlesToRender==mMaxParticles){
            mState=PARTICLE_CREATOR_SUSTAINING;
        }
    }

    //Creator has hit max number of particles and is still active
    else if (mState==PARTICLE_CREATOR_SUSTAINING){
        mParticlesToRender= mMaxParticles;
    }

    //Creator has stopped drawing new particles but still has remaining living particles
    else if (mState==PARTICLE_CREATOR_STOPPING){
        mParticlesToRender-=indexDifference;
        mParticleBufferReadLocation+=indexDifference;
        if(mParticleBufferReadLocation>=mMaxParticles){
            mParticleBufferReadLocation-=mMaxParticles;
        }

        if(mParticlesToRender==0){
            mState=PARTICLE_CREATOR_STOPPED;
        }
    }
}

void ParticleCreator::Render(RSC_GLProgram* program){
    if(mState==PARTICLE_CREATOR_STOPPED){return;}

    program->Bind();
    glBindVertexArray (vao);
    //Push time to GPU
    float floatTime=((float)(mTime));
    glUniform1f(mTimeUniformLocation,  floatTime);

    /*
    Figure out if we need two draw calls or one
    If the state is starting or sustaining, we can just start at the the beginning of the buffer and render 'mParticlesToRender' amount of particles
    if the state is stopping, then we need to render starting at 'mParticleBufferReadLocation' and continue on by 'mParticlesToRender' amount
    Also, if the active part of the buffer is split between the end and the start of the buffer (which it likely is) we'll need two draw calls
    */
    if(mState==PARTICLE_CREATOR_STOPPING){
        if((mParticleBufferReadLocation + mParticlesToRender > mMaxParticles)){
            int headRenderStart= mParticleBufferReadLocation;
            int headRenderSize = mMaxParticles - mParticleBufferReadLocation;

            int tailRenderStart= 0;
            int tailRenderSize = mParticlesToRender - headRenderSize;

            glDrawArrays (GL_QUADS, headRenderStart, headRenderSize*4);
            glDrawArrays (GL_QUADS, tailRenderStart, tailRenderSize*4);
        }
    }



    //Render starting from buffer start to mParticlesToRender
    glDrawArrays (GL_QUADS, 0, mParticlesToRender*4);
}

//////////////////////
//Component Particle//
//////////////////////

ComponentParticle::ComponentParticle(EID id, ComponentPosition* pos, RenderManager* rm, const std::string& logName)
    : BaseComponent(id, logName), dependencyRenderManager(rm){
     myPos = pos;
}

ComponentParticle::~ComponentParticle(){

}

void ComponentParticle::Update(){
    ParticleCreator*  pCreator=NULL;
    for(int i=0; i!=mParticleCreators.size(); i++){
        pCreator=(mParticleCreators[i]).get();
        pCreator->mLifeSpan--;
        if(pCreator->mLifeSpan==0){
            if(pCreator->mState==PARTICLE_CREATOR_STOPPED){

            }
            else if(pCreator->mState==PARTICLE_CREATOR_STOPPING){

            }
            else{pCreator->Stop();}
        }
        pCreator->Update();
    }
}

void ComponentParticle::HandleEvent(const Event* event){

}


ParticleCreator* ComponentParticle::AddParticleCreator    (const unsigned int& creatorLife, const unsigned int& particleLife){
    mParticleCreators.push_back(std::unique_ptr<ParticleCreator> (new ParticleCreator (dependencyRenderManager, particleLife, false, logFileName)));
    ParticleCreator* pc=mParticleCreators[mParticleCreators.size()-1].get();
    pc->mLifeSpan=creatorLife;
    pc->myPos=myPos;
    return pc;
}
void             ComponentParticle::DeleteParticleCreators(){

}

//////////////////////////////
//Component Particle Manager//
//////////////////////////////
void ComponentParticleManager::Update(){
    compMapIt i=componentList.begin();
    for(; i!=componentList.end(); i++){
        i->second->Update();
    }
}

ComponentParticleManager::ComponentParticleManager(EventDispatcher* e) : BaseComponentManager("LOG_COMP_PARTICLE", e){
}

ComponentParticleManager::~ComponentParticleManager(){
}

void ComponentParticleManager::AddComponent(EID id){
    compMapIt i=componentList.find(id);
    if(i!=componentList.end()){return;}
    ComponentParticle* par=new ComponentParticle(id, (ComponentPosition*)dependencyPosition->GetComponent(id), dependencyRenderManager, logFileName);
    componentList[id]=par;
}

void ComponentParticleManager::SetDependencies(RenderManager* rm, ComponentPositionManager* pos){
	dependencyRenderManager = rm;
	dependencyPosition = pos;
}
