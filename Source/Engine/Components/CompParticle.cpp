#include "CompParticle.h"
#include "../Kernel.h"
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
    : RenderableObjectWorld(rm), dependencyRenderManager(rm), mParticleLifeSpan(particleLife), mUseSprite(useSprite), VerticiesPerParticle(4), logFileName(logFile){

    mScriptShaderCodeVertex="";
    mScriptShaderCodeFragment="";

    mShape=PARTICLE_SQUARE;
    mEffect=PARTICLE_NONE;

    mTime=0;
    mParticlesPerFrame=1;
    mParticlesToRender=0;

    mSprite=NULL;
	mTexture=NULL;
	mAnimationFrame = 0;
	mAnimation = NULL;

	vbo = 0;
	vao = 0;

	mUsePoint = false;
	mPointCoordinates = Coord2df(0.0, 0.0);
	mPointIntensity = 18.0;

    mParticlesToRender=0;
    mRandom.SetSeed(mRandom.GenerateSeed());

    Color4f c (1.0f,1.0f,1.0f,1.0f);
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

void ParticleCreator::SetUsePoint(bool value){
	mUsePoint = value;
}

void ParticleCreator::SetPointIntensity(float value){
	mPointIntensity = value;
}

void ParticleCreator::SetPoint(const Coord2df& v){
	mPointCoordinates = v;
}

void ParticleCreator::SetRandomUV(bool value){
	mRandomUV = value;
}

bool ParticleCreator::Start(){
	//Find way to check if the shader should be recompiled
	if(mState == PARTICLE_CREATOR_STOPPING){return false;}
    mTime=0;
	mLifeSpan = mLifeSpanMax;
    mCurrentParticleIndex=0;
    mParticlesToRender=0;
    //Only one vertex per object; the rest are created in a geometry shader;
    //Getting ceiling of this rounded number
    mMaxParticles=ceil( (mParticleLifeSpan * mParticlesPerFrame));

    //Generate and set shader program
    mShaderProgram.reset(new RSC_GLProgram);

	mShaderVertex.reset(NULL);
	mShaderVertex = std::move(GenerateVertexShader());
	mShaderFragment.reset(NULL);
	mShaderFragment = std::move(GenerateFragmentShader());

    mShaderProgram->AddShader( mShaderVertex.get() );
    mShaderProgram->AddShader( mShaderFragment.get() );

    mShaderProgram->LinkProgram();
    mShaderProgram->Bind();
    SetShaderProgram(mShaderProgram.get());

    //Set up time uniform (which is a float)
	try{
		mTimeUniformLocation = mShaderProgram->GetUniformLocation("time");
	}
	catch(LEngineShaderProgramException e){
		//if uniform is compiled out of the script, it wasn't needed
	}

    glUniform1f(mTimeUniformLocation, ((float)(mTime)) );


	//Set up Camera UBO
	try{
		dependencyRenderManager->AssignCameraUBO(mShaderProgram.get());
	}
	catch(LEngineShaderProgramException e){
		//if uniform is compiled out of the script, it wasn't needed
	}

	unsigned int stride=sizeof(ParticleVertexData);
	if(vbo == 0){
		//Get size of VBO in bytes
		vboBufferSize = mMaxParticles * sizeof ( ParticleVertexData ) * VerticiesPerParticle;

		//Assign smart pointer to new data buffer
		vboData.reset(new ParticleVertexData [mMaxParticles * VerticiesPerParticle] );

		//Vertex VBO
		glGenBuffers (1, &vbo);
		glBindBuffer (GL_ARRAY_BUFFER, vbo);
								//     Size of Buffer                           Pointer to data
		glBufferData (GL_ARRAY_BUFFER, vboBufferSize,    vboData.get(), GL_STATIC_DRAW);
	}

	if(vao == 0){
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
	}

    //The particle creator will begin generating particles, but this state indicates that
    //it hasn't hit the maximum number of particles yet
    mState=PARTICLE_CREATOR_STARTING;

    mRandom.SetSeed(mRandom.GenerateSeed());

    //Write entire buffer
    //WriteData(0, mMaxParticles);
	return true;
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

std::unique_ptr<RSC_GLShader> ParticleCreator::GenerateFragmentShader(){
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
    << PARTICLE_SHADER_FRAGMENT_MAIN_BEGIN;
	if(mSprite!=NULL){
		scriptCodeFragment << PARTICLE_SHADER_FRAGMENT_MAIN_SPRITE_END;
	}
    ss << scriptCodeFragment.str()
    << PARTICLE_SHADER_FRAGMENT_MAIN_END
    ;

    std::unique_ptr<RSC_GLShader> shader (new RSC_GLShader(ss.str(), SHADER_FRAGMENT));

    ss << "\n\n\n";
    K_Log.Write(ss.str());

    return shader;
}

std::unique_ptr<RSC_GLShader> ParticleCreator::GenerateVertexShader(){
    std::stringstream ss;
    ss
    << PARTICLE_SHADER_VERTEX_DECLARATIONS;
	if(mUsePoint){
		ss << PARTICLE_SHADER_VERTEX_POINT_BEGIN(mPointIntensity);
	}
	else{
		ss << PARTICLE_SHADER_VERTEX_MAIN_BEGIN;
	}
	ss << PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_BEGIN
    << PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_EFFECT_SHRINK
    //<< PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_EFFECT_EXPAND
    << PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_END
    << PARTICLE_SHADER_VERTEX_MAIN_END
    ;
    std::unique_ptr<RSC_GLShader> shader (new RSC_GLShader(ss.str(), SHADER_VERTEX));

    ss << "\n\n\n";
    K_Log.Write(ss.str());

    return shader;
}

void ParticleCreator::SetParticlesPerFrame(const float& particles){
	/// \TODO bad things will happen if this is changed while the particle creator is running
    if(particles>0){
        mParticlesPerFrame=particles;
        return;
    }

    mParticlesPerFrame=0;
}

void ParticleCreator::SetSprite(const RSC_Sprite* sprite){
	mSprite = sprite;	
	std::string textureName = sprite->GetTextureName();
	mTexture = K_TextureMan.GetLoadItem(textureName, textureName);
}

void ParticleCreator::SetAnimation(const std::string& animationName){
	if(mSprite != NULL){
		mAnimation = mSprite->GetAnimation(animationName);
	}
}

void ParticleCreator::SetAnimationFrame(int frame){
	mAnimationFrame = frame;
	if(mAnimation != NULL){
		if (mAnimationFrame >= mAnimation->NumberOfImages()){
			mAnimationFrame = mAnimation->NumberOfImages()-1;
		}
	}
}

void ParticleCreator::SetColor(const float& rMin, const float& gMin, const float& bMin, const float& aMin,
                                const float& rMax, const float& gMax, const float& bMax, const float& aMax){
    Color4f cMin(rMin,gMin,bMin,aMax);
    Color4f cMax(rMax,gMax,bMax,aMax);

    mDefaultColorMin=cMin;
    mDefaultColorMax=cMax;
}

/*void ParticleCreator::SetShape(PARTICLE_SHAPE shape){
    mShape=shape;
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

void ParticleCreator::SetWarpQuads(bool value){
	mWarpQuads	 = value;
}

//Location and Size is in particles, not bytes or verticies
void ParticleCreator::WriteData(const unsigned int& writeLocation, const unsigned int& writeSize){
	if(writeSize == 0){return;}
    bool ringLoop=false; //whether there is one single contiguous memory buffer or two

    /*
    A weird side effect of using a ring buffer is that buffer[0] is always drawn first
    That means that if all of the old particles are overlapping it, you won't see buffer[0] (or any other overlapped particle)
    when its first created
    It may look like the creator stopped generating particles momentarily
    This is especially noticable if the particles are especially large or slow
    */

    unsigned int writeLocationVertex = writeLocation * 4; //convert to actual vertex index
    unsigned int writeSizeVertex = writeSize * 4;
    unsigned int maxParticlesVertex = mMaxParticles * 4;

    for(int i=0; i < writeSize; i++){
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

		Vec2 acceleration;

		//If using a point to move toward/away from, then store the point in the acceleration vector
		if(mUsePoint){
			//x is distance from point
			//y is angle
			//Pythogoreas theorem
			float xDifference = position.x - mPointCoordinates.x;
			float yDifference = position.y - mPointCoordinates.y;
			//square
			float xDifferenceSquare = xDifference * xDifference;
			float yDifferenceSquare = yDifference * yDifference;
			//Add together and get root to get distance
			acceleration.x = sqrt(xDifferenceSquare + yDifferenceSquare);

			//SOH CAH TOA
			acceleration.y = atan2(yDifference, xDifference);
		}
		else{
			acceleration = Vec2(mRandom.GenerateRandomFloatValue( mDefaultAccelerationMin.x,
																mDefaultAccelerationMax.x),
							mRandom.GenerateRandomFloatValue(   mDefaultAccelerationMin.y,
																mDefaultAccelerationMax.y)
							);
		}

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


        Vec4 color( mRandom.GenerateRandomFloatValue( mDefaultColorMin.r,
                                                    mDefaultColorMax.r),
                    mRandom.GenerateRandomFloatValue( mDefaultColorMin.g,
                                                    mDefaultColorMax.g),
                    mRandom.GenerateRandomFloatValue( mDefaultColorMin.b,
                                                    mDefaultColorMax.b),
                    mRandom.GenerateRandomFloatValue( mDefaultColorMin.a,
                                                    mDefaultColorMax.a)
                            );

		CRect coord (-1,-1,2,2);
		float texCoordLeft = -1;
		float texCoordRight = 1;
		float texCoordTop = -1;
		float texCoordBottom = 1;
		if((mAnimation!=NULL) and (mTexture!=NULL)){
			if(mRandomUV){
				auto coordinates = mAnimation->GetUVRandom(mAnimationFrame);
				texCoordLeft = std::get<0>(coordinates).x;
				texCoordRight = std::get<1>(coordinates).x;
				texCoordBottom = std::get<1>(coordinates).y;
				texCoordTop = std::get<0>(coordinates).y;
			}
			else{
				texCoordLeft = mAnimation->GetUVLeft(mAnimationFrame);
				texCoordRight = mAnimation->GetUVRight(mAnimationFrame);
				texCoordBottom = mAnimation->GetUVBottom(mAnimationFrame);
				texCoordTop = mAnimation->GetUVTop(mAnimationFrame);
			}
		}
		Coord2df pos=myPos->GetPositionWorld();
		Vec2 offset(0,0);
		Vec2 tex(0,0);
		Vec2 worldPos(pos.x, pos.y);
        for(int vert = 0; vert < 4; vert++){
            ParticleVertexData& data = vboData.get()[writeLocationVertex + vert];

            if(vert==0){
				offset.x=-1 * scaling.x; offset.y=-1 * scaling.y;
				tex.x = texCoordLeft;
				tex.y = texCoordTop;
			}
            if(vert==1){
				if(mWarpQuads){
					position = Vec2(
						mRandom.GenerateRandomFloatValue(mDefaultPositionMin.x,
                                                        mDefaultPositionMax.x),
						mRandom.GenerateRandomFloatValue(mDefaultPositionMin.y,
                                                        mDefaultPositionMax.y)
					);
				}
				offset.x= 1 * scaling.x; offset.y=-1 * scaling.y;
				tex.x = texCoordRight;
				tex.y = texCoordTop;
			}
            if(vert==2){
				if(mWarpQuads){
					position = Vec2(
						mRandom.GenerateRandomFloatValue(mDefaultPositionMin.x,
                                                        mDefaultPositionMax.x),
						mRandom.GenerateRandomFloatValue(mDefaultPositionMin.y,
                                                        mDefaultPositionMax.y)
					);
				}
				offset.x= 1 * scaling.x; offset.y= 1 * scaling.y;
				tex.x = texCoordRight;
				tex.y = texCoordBottom;
			}
            if(vert==3){
				if(mWarpQuads){
					position = Vec2(
						mRandom.GenerateRandomFloatValue(mDefaultPositionMin.x,
                                                        mDefaultPositionMax.x),
						mRandom.GenerateRandomFloatValue(mDefaultPositionMin.y,
                                                        mDefaultPositionMax.y)
					);
				}
				offset.x=-1 * scaling.x; offset.y= 1 * scaling.y;
				tex.x = texCoordLeft;
				tex.y = texCoordBottom;
			}

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
	if((mState == PARTICLE_CREATOR_STOPPING)or(mState == PARTICLE_CREATOR_STOPPED)){return;}

	//Render all particles if not already sustaining
	mParticlesToRender = mMaxParticles;

	//Set the read location the oldest written Location
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
	//Render all particles, ignore read position 
    else if (mState==PARTICLE_CREATOR_SUSTAINING){
        mParticlesToRender= mMaxParticles;
    }

    //Creator has stopped drawing new particles but still has remaining living particles
	//Render particles starting at read Position, then increment read position by mParticlesPerFrame
    else if (mState==PARTICLE_CREATOR_STOPPING){
        mParticlesToRender-=indexDifference;
        mParticleBufferReadLocation = floor(indexBefore) + 1;

        if(mParticlesToRender==0){
            mState=PARTICLE_CREATOR_STOPPED;
        }
    }
}

void ParticleCreator::Render(const RenderCamera* camera, const RSC_GLProgram* program){
    if(mState==PARTICLE_CREATOR_STOPPED){return;}
	if(mTexture!=NULL){
		mTexture->Bind();
	}

    program->Bind();
    glBindVertexArray (vao);
    //Push time to GPU
    float floatTime=((float)(mTime));
    glUniform1f(mTimeUniformLocation,  floatTime);

    /*
    If the state is starting or sustaining, we can just start at the the beginning of the buffer and render 'mParticlesToRender' amount of particles
    if the state is stopping, then we need to render starting at 'mParticleBufferReadLocation' and continue on by 'mParticlesToRender' amount
    Also, if the active part of the buffer is split between the end and the start of the buffer (which it likely is) we'll need two draw calls
    */

    if(mState==PARTICLE_CREATOR_STOPPING){
        if((mParticleBufferReadLocation + mParticlesToRender > mMaxParticles)){
            int headRenderStart = mParticleBufferReadLocation;
            int headRenderSize  = mMaxParticles - mParticleBufferReadLocation;

            int tailRenderStart = 0;
            int tailRenderSize  = mParticlesToRender - headRenderSize;

            glDrawArrays (GL_QUADS, headRenderStart*4, headRenderSize*4);
            glDrawArrays (GL_QUADS, tailRenderStart*4, tailRenderSize*4);

			return;
        }
        else{
            int renderStart= mParticleBufferReadLocation;
            int renderSize = mParticlesToRender;

            glDrawArrays (GL_QUADS, renderStart*4, renderSize*4);

			return;
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
	//use while loop so elements can be deleted mid loop
	auto i = mParticleCreators.begin();
    while(i != mParticleCreators.end()){
        auto pCreator = (i->get());

		//update
        pCreator->Update();

		//decrease lifespan
		//
		//if lifespan is 0, live forever
		if (pCreator->mLifeSpan == 0){
			i++;
			continue;
		}

        if(pCreator->mLifeSpan==1){
            if(pCreator->mState==PARTICLE_CREATOR_STOPPED){
				//ignore
            }
            else if(pCreator->mState==PARTICLE_CREATOR_STOPPING){
				//ignore
            }
            else{pCreator->Stop();}
        }
		else{
			pCreator->mLifeSpan--;
		}
		//iterate
		i++;
    }
}

void ComponentParticle::HandleEvent(const Event* event){

}


ParticleCreator* ComponentParticle::AddParticleCreator    (const unsigned int& creatorLife, const unsigned int& particleLife){
    mParticleCreators.push_back(make_unique<ParticleCreator>(dependencyRenderManager, particleLife, false, logFileName));
    ParticleCreator* pc=mParticleCreators[mParticleCreators.size()-1].get();
    pc->mLifeSpanMax=creatorLife;
	pc->mLifeSpan = creatorLife;
    pc->myPos=myPos;
    return pc;
}
void ComponentParticle::DeleteParticleCreators(){
	mParticleCreators.clear();
}

//////////////////////////////
//Component Particle Manager//
//////////////////////////////
ComponentParticleManager::ComponentParticleManager(EventDispatcher* e) : BaseComponentManager("LOG_COMP_PARTICLE", e){
}

ComponentParticleManager::~ComponentParticleManager(){
}

void ComponentParticleManager::AddComponent(EID id, EID parent){
    auto i=componentList.find(id);
    if(i!=componentList.end()){return;}
    auto par = make_unique<ComponentParticle>(id, (ComponentPosition*)dependencyPosition->GetComponent(id), dependencyRenderManager, logFileName);
    componentList[id] = std::move(par);
}

void ComponentParticleManager::SetDependencies(RenderManager* rm, ComponentPositionManager* pos){
	dependencyRenderManager = rm;
	dependencyPosition = pos;
}
