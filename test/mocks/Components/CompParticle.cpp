#include "CompParticle.h"
#include <math.h>
#include "../Kernel.h"

/*
The CPU will determine how many Particles to render based on its state (STARTING, STOPPING, SUSTAINING, etc...)
and by how much time has passed since having been started
It will also insert 'new' particles (new particles being freshly seeded random values) into the GPU buffer (is treated as a ring buffer)
Each Particle Creator will have two draw calls, one for the head of the buffer and one for the tail.
*/

////////////////////
//Particle Creator//
////////////////////

ParticleCreator::ParticleCreator(const unsigned int& particleLife, const bool& useSprite, const std::string& logFile)
    : /*RenderableObjectWorld(),*/ mParticleLifeSpan(particleLife), mUseSprite(useSprite), VerticiesPerParticle(4), logFileName(logFile){

}

ParticleCreator::~ParticleCreator(){

}

void ParticleCreator::Start(){

}

void ParticleCreator::SetVertexShaderCode(const std::string& code){
}

void ParticleCreator::SetFragmentShaderCode(const std::string& code){
}

void ParticleCreator::SetShape(const PARTICLE_SHAPE& shape){
}

void ParticleCreator::SetEffect(const PARTICLE_EFFECT& effect){
}

L_GL_Shader* ParticleCreator::GenerateFragmentShader(){

}

L_GL_Shader* ParticleCreator::GenerateVertexShader(){

}

void ParticleCreator::SetParticlesPerFrame(const float& particles){

}

void ParticleCreator::SetColor(const float& rMin, const float& gMin, const float& bMin, const float& aMin,
                                const float& rMax, const float& gMax, const float& bMax, const float& aMax){

}

void ParticleCreator::SetPosition(const Coord2df& positionMin, const Coord2df& positionMax){

}

void ParticleCreator::SetVelocity(const Coord2df& velocityMin, const Coord2df& velocityMax){
}

void ParticleCreator::SetAcceleration(const Coord2df& accelMin, const Coord2df& accelMax){
}

void ParticleCreator::SetScalingX(const float& xscaleMin, const float& xscaleMax){
}
void ParticleCreator::SetScalingY(const float& yscaleMin, const float& yscaleMax){
}


//Location and Size is in particles, not bytes or verticies
void ParticleCreator::WriteData(const unsigned int& writeLocation, const unsigned int& writeSize){

}

void ParticleCreator::Stop(){
}

void ParticleCreator::Update(){

}

void ParticleCreator::Render(L_GL_Program* program){

}

//////////////////////
//Component Particle//
//////////////////////

ComponentParticle::ComponentParticle(EID id, Coord2df pos, const std::string& logName)
    : BaseComponent(id, logName){
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
    mParticleCreators.push_back(std::unique_ptr<ParticleCreator> (new ParticleCreator (particleLife, false, logFileName)));
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

ComponentParticleManager::ComponentParticleManager() : BaseComponentManager("LOG_COMP_PARTICLE"){
}

ComponentParticleManager::~ComponentParticleManager(){
}

void ComponentParticleManager::AddComponent(EID id){
    compMapIt i=componentList.find(id);
    if(i!=componentList.end()){return;}
    ComponentParticle* par=new ComponentParticle(id, Coord2df(5,5), logFileName);
    componentList[id]=par;
}

