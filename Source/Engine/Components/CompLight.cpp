#include "CompLight.h"
#include "../Kernel.h"


///////////////
//LightSource//
///////////////

LightSource::LightSource(const float& iStart, const float& iEnd, const float& rad, const float& n, const Vec2& off)
    : intensityStart(iStart), intensityEnd(iEnd), radius(rad), noise(n), offset(off){
    render=true;
    radius=150;
}

//////////////////
//ComponentLight//
//////////////////

ComponentLight::ComponentLight (EID id, ComponentPosition* pos, const std::string& logName) : BaseComponent(id, logName){
    myPos = pos;
    numberOfLoadedLights=0;

    //just for testing sake;
    NewLightSource(1.0f, 0.1f, 5.0f);
}
ComponentLight::~ComponentLight (){}

bool ComponentLight::LightExists(const int& id){
    return id<numberOfLoadedLights;
}

void ComponentLight::Update(){}
/*
void ComponentLight::Render(const CRect& drawArea){
    RSC_Texture* texture=K_TextureMan.GetItem("SpotLight.png");
    if(texture==NULL){ErrorLog::WriteToFile("CARP"); return;}
    //texture->BlitArea(CRect(32,0,texture->GetWidth(),texture->GetHeight()), L_ORIGIN_CENTER);
    texture->Bind();

    float radius=128;
    Coord2d pos=myPos->GetPositionWorldInt();
    pos.x -= drawArea.x;
    pos.y -= drawArea.y;
    if( (pos.x<0) or (pos.y<0) or (pos.x>drawArea.GetRight()) or (pos.y>drawArea.GetBottom()) ){
        return;
    }
    //Use shader here! :D

    CRect area(0,0,texture->GetWidth(),texture->GetHeight());
    area.heightTruncation=false;
    float Left=     (float)area.GetLeft()   / (float)texture->GetWidth();
    float Right=    (float)area.GetRight()  / (float)texture->GetWidth();
    float Top=      (float)area.GetTop()    / (float)texture->GetHeight();
    float Bottom=   (float)area.GetBottom() / (float)texture->GetHeight();

    glBegin(GL_QUADS);
        glTexCoord2f(Left,  Top);		glVertex3i(-radius + pos.x,  -radius + pos.y,      0);
        glTexCoord2f(Right, Top);	    glVertex3i( radius + pos.x,  -radius + pos.y,      0);
        glTexCoord2f(Right, Bottom);	glVertex3i( radius + pos.x,   radius + pos.y,      0);
        glTexCoord2f(Left,  Bottom);	glVertex3i(-radius + pos.x,   radius + pos.y,      0);
    glEnd();
}*/

int ComponentLight::NewLightSource (const float& intensityStart, const float& intensityEnd, const float& rad, const float& noise, const Vec2& offset){
    LightSource light(intensityStart, intensityEnd, rad, noise, offset);
    lightSources.push_back(light);

    int returnValue=numberOfLoadedLights;
    numberOfLoadedLights++;

    return returnValue;
}

//////////////////////////
//ComponentLightManager//
//////////////////////////

ComponentLightManager::ComponentLightManager(EventDispatcher* e) : BaseComponentManager("LOG_COMP_LIGHT", e), vao(MAX_LIGHTS) {
    glGenFramebuffers(1, &FBO);
    BuildVAOFirst();
}

void ComponentLightManager::AddComponent(EID id, EID parent){
    auto i=componentList.find(id);
    if(i!=componentList.end()){return;}
    auto light = make_unique<ComponentLight>(id, (ComponentPosition*)Kernel::stateMan.GetCurrentState()->comPosMan.GetComponent(id), logFileName);
    componentList[id] = std::move(light);
}


void ComponentLightManager::Update(){
	//Update all entities
    for(auto i=componentList.begin(); i!=componentList.end(); i++){
		UpdateComponent(i->second.get());
    }

	//Reset all 'updatedThisFrame' bits
	for(auto i = componentList.begin(); i != componentList.end(); i++){
		i->second->updatedThisFrame = false;
	}

    BuildVAO();
}

void ComponentLightManager::Render(RSC_Texture* textureDiffuse, RSC_Texture* textureDestination, const RSC_GLProgram* shaderProgram){
    //Set framebuffer to clear destination Texture
    RSC_GLProgram::BindNULL();
    RSC_Texture::BindNull();
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureDestination->GetOpenGLID(), 0);
    //Clear
    glClearColor(0.25f, 0.55f, 0.85f, 1.0f );
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, NULL);

    //Render diffuse to destination, (only the areas that contain light will be processed by the shader)
    textureDiffuse->RenderToTexture(CRect(0,0, textureDestination->GetWidth(), textureDestination->GetHeight()), textureDestination);

    shaderProgram->Bind();

    //Push viewport bit
    glPushAttrib(GL_VIEWPORT_BIT);
    //Setup frame buffer render
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    //Attach Diffuse and Light textures
    K_TextureMan.LoadItem("SpotLight.png", "SpotLight.png");
    lightTexture=K_TextureMan.GetItem("SpotLight.png");
    glActiveTexture(GL_TEXTURE1);
    lightTexture->Bind();
    glActiveTexture(GL_TEXTURE2);
    textureDiffuse->Bind();

    //Map samplers to their respective texture bind points
    GLuint diffuseLocation       = glGetUniformLocation(shaderProgram->GetHandle(), "diffuse");
    GLuint lightTextureLocation  = glGetUniformLocation(shaderProgram->GetHandle(), "lightTexture");
    glUniform1i(diffuseLocation,        2);
    glUniform1i(lightTextureLocation,   1);

    //Set Render Viewport
    glViewport(0,0, textureDestination->GetWidth(), textureDestination->GetHeight());

    //Atatch buffer texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureDestination->GetOpenGLID(), 0);

    //Output destination
    /*glBindFragDataLocation(shaderProgram->GetHandle(), 0, "outputColor");
    GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);*/

    glBindVertexArray (vao.GetVAOID());
    glDrawArrays (GL_QUADS, 0, numberOfLights * 4);

    //Unbind everything
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray (NULL);
    RSC_GLProgram::BindNULL();

    //Back to initial viewport
    glPopAttrib();

    //Unbind Textures
    glActiveTexture(GL_TEXTURE2);
    RSC_Texture::BindNull();
    glActiveTexture(GL_TEXTURE1);
    RSC_Texture::BindNull();
    glActiveTexture(GL_TEXTURE0);
    RSC_Texture::BindNull();
}

void ComponentLightManager::BuildVAOFirst(){
    //Sets up texture coordinates (and currently color)
    unsigned int vertex=0;
    for(int i=0; i<MAX_LIGHTS; i++){
        //TopLeft
        vao.GetTextureArray()[vertex  ].x = 0.0f;
        vao.GetTextureArray()[vertex  ].y = 0.0f;

        //TopRight
        vao.GetTextureArray()[vertex+1].x = 1.0f;
        vao.GetTextureArray()[vertex+1].y = 0.0f;

        //BottomRight
        vao.GetTextureArray()[vertex+2].x = 1.0f;
        vao.GetTextureArray()[vertex+2].y = 1.0f;

        //BottomLeft
        vao.GetTextureArray()[vertex+3].x = 0.0f;
        vao.GetTextureArray()[vertex+3].y = 1.0f;

        float colorR=1.0f;
        float colorG=1.0f;
        float colorB=1.0f;
        float colorA=1.0f;
        //Color
        vao.GetColorArray()  [vertex  ].x = colorR;
        vao.GetColorArray()  [vertex  ].y = colorG;
        vao.GetColorArray()  [vertex  ].z = colorB;
        vao.GetColorArray()  [vertex  ].w = colorA;

        vao.GetColorArray()  [vertex+1].x = colorR;
        vao.GetColorArray()  [vertex+1].y = colorG;
        vao.GetColorArray()  [vertex+1].z = colorB;
        vao.GetColorArray()  [vertex+1].w = colorA;

        vao.GetColorArray()  [vertex+2].x = colorR;
        vao.GetColorArray()  [vertex+2].y = colorG;
        vao.GetColorArray()  [vertex+2].z = colorB;
        vao.GetColorArray()  [vertex+2].w = colorA;

        vao.GetColorArray()  [vertex+3].x = colorR;
        vao.GetColorArray()  [vertex+3].y = colorG;
        vao.GetColorArray()  [vertex+3].z = colorB;
        vao.GetColorArray()  [vertex+3].w = colorA;

        vertex+=4;
    }
}
void ComponentLightManager::BuildVAO(){
    unsigned int vertex=0;
    unsigned int lightCount=0;

    for(auto i = componentList.begin(); i != componentList.end(); i++) {
        auto lightSources=((ComponentLight*)(i->second.get()))->lightSources;
        for(auto lightSource=lightSources.begin(); lightSource!=lightSources.end(); lightSource++){
            float radius = lightSource->radius;
            auto pos = ((ComponentLight*)(i->second.get()))->myPos->GetPositionWorld();
			pos = pos.Round();

            //Will subtract camera translation in shader later on during rendering

            Vec2 topLeftVertex      (-radius + pos.x, -radius + pos.y);
            Vec2 topRightVertex     ( radius + pos.x, -radius + pos.y);
            Vec2 bottomRightVertex  ( radius + pos.x,  radius + pos.y);
            Vec2 bottomLeftVertex   (-radius + pos.x,  radius + pos.y);

            vao.GetVertexArray()[vertex  ] = topLeftVertex;
            vao.GetVertexArray()[vertex+1] = topRightVertex;
            vao.GetVertexArray()[vertex+2] = bottomRightVertex;
            vao.GetVertexArray()[vertex+3] = bottomLeftVertex;

            lightCount+=1;
            vertex+=4;
        }
    }

    numberOfLights=lightCount;
    vao.UpdateGPU();
}

