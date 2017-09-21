#include "RenderManager.h"

#include "Components/CompSprite.h"
#include "Kernel.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

#include "Resources/RSC_Map.h"


////////////////
//RenderCamera//
////////////////

RenderCamera::RenderCamera(RenderManager* rm, CRect viewPort)
	: frameBufferTextureDiffuse(std::unique_ptr<RSC_Texture>(new RSC_Texture(viewPort.w, viewPort.h, 4, GL_RGBA)))
	, frameBufferTextureFinal  (std::unique_ptr<RSC_Texture>(new RSC_Texture(viewPort.w, viewPort.h, 4, GL_RGBA)))
	, dependencyRenderManager(rm){
	scale=1;
	screenSpace = FloatRect(0, 0, 1.0, 1.0);
	rotation=0;
	SetView(viewPort);

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//render buffers are a good choice when not intending on sampling from the data
	//the depth buffer is only for the framebuffer, it won't be sampled externally
	//only the color will be sampled externally
	glGenRenderbuffers(1, &mDepthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mDepthRBO); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, viewPort.w, viewPort.h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRBO);  

	//unbind
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//Does the GPU support current FBO configuration?
	GLenum status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch(status){
		case GL_FRAMEBUFFER_COMPLETE: break;
		default:
			std::stringstream ss;
			ss << "RenderCamera framebuffer failed, status: " << status << std::endl;
			K_Log.Write(ss.str(), Log::SEVERITY::FATAL);
			break;
	}

	dependencyRenderManager->AddCamera( this);
}

RenderCamera::~RenderCamera(){
	glDeleteFramebuffers(1, &FBO);
	glDeleteRenderbuffers(1, &mDepthRBO);
	//should this be called? what if this fbo is currently bound?
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	dependencyRenderManager->RemoveCamera(this);
}


void RenderCamera::SetScaling (float s)	{
	scale = s;
}

void RenderCamera::SetRotation(float r)	{
	rotation = r;
}

float RenderCamera::GetScaling() const{
	return scale;
}

float RenderCamera::GetRotation() const{
	return rotation;
}

CRect RenderCamera::GetView()const {
	return view;
}

void RenderCamera::SetView(CRect viewPort){
	view = viewPort;
}

FloatRect RenderCamera::GetScreenSpace()const {
	return screenSpace;
}

void RenderCamera::SetScreenSpace(FloatRect screen){
	screenSpace = screen;
}

void RenderCamera::Bind(const GLuint& GlobalCameraUBO){
	Matrix4 T= Matrix4::IdentityMatrix();
	T=T.Translate(Vec3 (-view.x, -view.y, 0.0));

	Matrix4 R= Matrix4::IdentityMatrix();
	R=R.RotateZ(0);

	Matrix4 S= Matrix4::IdentityMatrix();
	S= S.Scale(Vec3(4,4,1));

	//not used yet, should be used
	Matrix4 modelViewMat = T * R * S;
	//Will render texture upside down
	Matrix4 projectionMat = Matrix4::OrthoGraphicProjectionMatrix(view);

	float position[4];
	//Render Using only full integers for translation to get that pixel-perfect look
	position[0]=view.x;
	position[1]=view.y;
	position[2]=view.w;
	position[3]=view.h;

	glBindBuffer(GL_UNIFORM_BUFFER, GlobalCameraUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0,				  (sizeof(float)*4),	position);
	glBufferSubData(GL_UNIFORM_BUFFER, (sizeof(float)*4), (sizeof(float)*16),  	&projectionMat.m);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//Push viewport bit
	glPushAttrib(GL_VIEWPORT_BIT);
	//Setup frame buffer render
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	//Set Render Viewport
	glViewport(0,0, frameBufferTextureDiffuse->GetWidth(), frameBufferTextureDiffuse->GetHeight());

	//Clear Background
	glClearColor(0.0,0.0,0.0,1.0);

	glClear(GL_COLOR_BUFFER_BIT);

	//Atatch buffer texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTextureDiffuse->GetOpenGLID(), 0);
}

void RenderCamera::RenderFrameBufferTextureFinal(){
	RenderFrameBufferTexture(frameBufferTextureFinal.get());
}
void RenderCamera::RenderFrameBufferTextureDiffuse(){
	RenderFrameBufferTexture(frameBufferTextureDiffuse.get());
}
void RenderCamera::RenderFrameBufferTexture(const RSC_Texture* tex){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	RSC_GLProgram::BindNULL();
	glBindVertexArray (0);

	//Back to initial viewport
	glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, SCREEN_W, SCREEN_H, 0, 0, 1); //2D
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	tex->Bind();
	float Left=		0;
	float Right=	1;
	//texture is upside down, invert top and bottom
	float Top=		1;
	float Bottom=	0;


	glBegin(GL_QUADS);
		glTexCoord2f(Left,	Top);
		glVertex3i(0,0,0);

		glTexCoord2f(Right, Top);
		glVertex3i(SCREEN_W,0,0);

		glTexCoord2f(Right, Bottom);
		glVertex3i(SCREEN_W,SCREEN_H,0);

		glTexCoord2f(Left,	Bottom);
		glVertex3i(0,SCREEN_H,0);
	glEnd();
}


/////////////////
//RenderManager//
/////////////////
bool LOrderOBJs(RenderableObject* r1, RenderableObject* r2){
	if(r1->GetDepth() <= r2->GetDepth()){return false;}
	return true;
}


GLuint RenderManager::GlobalCameraUBO=0;
GLuint RenderManager::GlobalProgramUBO=0;

const GLuint RenderManager::CameraDataBindingIndex = 1;
const GLuint RenderManager::ProgramDataBindingIndex = 2;
const std::string RenderManager::defaultProgramTileName = "defaultProgramTile";
const std::string RenderManager::defaultProgramSpriteName = "defaultProgramSprite";
const std::string RenderManager::defaultProgramLightName = "defaultProgramLight";
const std::string RenderManager::defaultProgramImageName = "defaultProgramImage";


RenderManager::RenderManager()
	: timeElapsed(0){

	LoadDefaultShaders();

	listChange=false;
	nextTextID=0;

	if( (defaultProgramTile == NULL) or (defaultProgramImage == NULL) or (defaultProgramLight == NULL) or (defaultProgramSprite == NULL) ){
		std::stringstream ss;
		ss << "RenderManager Couldn't load all default shaders";
		if(defaultProgramTile == NULL){ ss << "\n -Tile shader program could not be loaded";}
		if(defaultProgramSprite == NULL){ ss << "\n -Sprite shader program could not be loaded";}
		if(defaultProgramLight == NULL){ ss << "\n -Light shader program could not be loaded";}
		if(defaultProgramImage == NULL){ ss << "\n -Image shader program could not be loaded";}
		K_Log.Write(ss.str(), Log::SEVERITY::FATAL);
		throw LEngineException(ss.str());
	}
}

void RenderManager::OrderOBJs(){
	//Negative Depth is closer to the screen
	objectsScreen.sort(&LOrderOBJs);
	objectsWorld.sort(&LOrderOBJs);
	listChange=false;
}

void RenderManager::Render(){
	glViewport(0, 0, SCREEN_W, SCREEN_H);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
	timeElapsed += 1;

	glBindBuffer(GL_UNIFORM_BUFFER, GlobalProgramUBO);
	float values [4];
	values[0] = timeElapsed;
	values[1] = 0;
	values[2] = 0;
	values[3] = 0;

	glBufferSubData(GL_UNIFORM_BUFFER, 0, (sizeof(float)*4), &values);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	if(listChange){
		 OrderOBJs(); //Sort by Depth
	}

	//glLoadIdentity();

	if(!(mCameras).empty()){
		//World objects are rendered after the camera sets its matrix 
		auto currentCamera = mCameras.begin();
		(*currentCamera)->Bind(GlobalCameraUBO);
		for(auto i=objectsWorld.begin(); i!=objectsWorld.end(); i++){
			if((*i)->render){(*i)->Render(*currentCamera);}
		}

		//Need better way to handle light
		//Kernel::stateMan.GetCurrentState()->comLightMan.Render((*currentCamera)->GetFrameBufferTextureDiffuse(), (*currentCamera)->GetFrameBufferTextureFinal(), defaultProgramLight);

		//(*currentCamera)->RenderFrameBufferTextureFinal();
		(*currentCamera)->RenderFrameBufferTextureDiffuse();
	}

	//Screen objects are rendered after others, but do not use the camera's matrix
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	RSC_GLProgram::BindNULL();
	glBindVertexArray (0);

	for(auto i=objectsScreen.begin(); i!=objectsScreen.end(); i++){
		if((*i)->render){(*i)->Render();}
	}

	ImGui::Render();
	ImGuiRender( ImGui::GetDrawData() );

#ifdef DEBUG_MODE
	//Check for errors
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR){
		std::stringstream ss;
		std::string errorString;
		switch(err){
			case GL_INVALID_OPERATION: 	errorString = "GL_INVALID_OPERATION"; break;
			case GL_INVALID_ENUM:		errorString = "GL_INVALID_ENUM"; break;
			case GL_OUT_OF_MEMORY:		errorString = "GL_OUT_OF_MEMORY"; break;
			case GL_INVALID_VALUE:		errorString = "GL_INVALID_VALUE"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:	errorString = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
			default: errorString = "?";
		}
		ss << "GL Error: " << errorString << std::endl;
		K_Log.Write(ss.str(), Log::SEVERITY::ERROR);
	}
#endif
}

void RenderManager::AssignCameraUBO(RSC_GLProgram* program){
	//Get program and uniform buffer handles
	GLuint programHandle			= program->GetHandle();
	GLuint programUniformBlockHandle= program->GetUniformBlockHandle("CameraData");

	//Bind program GPU buffer to the index
	glUniformBlockBinding(programHandle, programUniformBlockHandle, CameraDataBindingIndex);
}

RenderSpriteBatch* RenderManager::GetSpriteBatch(const std::string& textureName, const MAP_DEPTH& depth, const int& numSprites){
	auto textureMapIt= spriteBatchMap.find(depth); //If there isn't a map for this depth value, create it and assign texturemapit to it
	if(textureMapIt==spriteBatchMap.end()){
		spriteBatchMap[depth];
		textureMapIt= spriteBatchMap.find(depth);
	}

	auto spriteBatchVectorIt = textureMapIt->second.find(textureName); //If there isn't a vector for this texture name value, then create it and assign spriteBatchVectorIt to it
	if(spriteBatchVectorIt == textureMapIt->second.end()){
	   textureMapIt->second[textureName];
	   spriteBatchVectorIt = textureMapIt->second.find(textureName);
	}

	RenderSpriteBatch* batch=NULL;
	for(auto vectorIt = spriteBatchVectorIt->second.begin(); vectorIt!= spriteBatchVectorIt->second.end(); vectorIt++){
		if((*vectorIt)->CanAddSprites(numSprites)==true){batch=(*vectorIt).get(); break;}
	}

	//If there isn't a spritebatch of appropriate size, create one and assign batch to it
	if(batch==NULL){
		//Max size 256
		spriteBatchVectorIt->second.push_back( std::unique_ptr<RenderSpriteBatch>(new RenderSpriteBatch(this, textureName, 256)));
		batch=(spriteBatchVectorIt->second.back()).get();
		batch->SetDepth(depth);
	}
	return batch;
}

void RenderManager::RemoveObjectScreen(RenderableObjectScreen* obj){
	for(auto i=objectsScreen.begin(); i!=objectsScreen.end(); i++){
		if(obj==*i){
			objectsScreen.erase(i);
			listChange=true;
			break;
		}
	}
}

void RenderManager::RemoveObjectWorld(RenderableObjectWorld* obj){
	for(auto i=objectsWorld.begin(); i!=objectsWorld.end(); i++){
		if(obj==*i){
			objectsWorld.erase(i);
			listChange=true;
			break;
		}
	}
}

void RenderManager::AddObjectWorld(RenderableObjectWorld* obj){
	//Maybe use visitor pattern here to achieve double dispatch (?)
	objectsWorld.push_back(obj);
	listChange=true;

	//Set Correct Shader
	if(obj->type == RenderableObject::TYPE::SpriteBatch){
		obj->SetShaderProgram(defaultProgramSprite);
	}
	if(obj->type == RenderableObject::TYPE::TileLayer){
		obj->SetShaderProgram(defaultProgramTile);
	}
	if(obj->type == RenderableObject::TYPE::Image){
		obj->SetShaderProgram(defaultProgramImage);
	}
}

void RenderManager::AddObjectScreen(RenderableObjectScreen* obj){
	objectsScreen.push_back(obj);
	listChange=true;
}

bool RenderManager::AddCamera(RenderCamera* cam){
	mCameras.insert(cam);
	return true;
}

void RenderManager::RemoveCamera(RenderCamera* cam){
	mCameras.erase(cam);
}

void RenderManager::ImGuiRender(ImDrawData* drawData){
// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiState* guiState = &Kernel::guiState;
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    // Backup GL state
    GLint last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
    glActiveTexture(GL_TEXTURE0);
    GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
    GLint last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, &last_blend_src_rgb);
    GLint last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, &last_blend_dst_rgb);
    GLint last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, &last_blend_src_alpha);
    GLint last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, &last_blend_dst_alpha);
    GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
    GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

    //Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    // Setup viewport, projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

    guiState->shaderHandle->Bind();
    glUniform1i(guiState->attribLocationTex, 0);
    glUniformMatrix4fv(guiState->attribLocationProjMtx, 1, GL_FALSE, guiState->projectionMatrix.m);
    glBindVertexArray(guiState->vaoHandle);

    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = drawData->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, guiState->vboHandle);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, guiState->elementsHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }

    // Restore modified GL state
    glUseProgram(last_program);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glActiveTexture(last_active_texture);
    glBindVertexArray(last_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

std::unique_ptr<RSC_GLProgram> RenderManager::LoadShaderProgram(const std::string& vertName, const std::string& fragName){
	std::unique_ptr<const RSC_GLShader> fragShader(make_unique<RSC_GLShader>(RSC_GLShader::LoadShaderFromFile(fragName), SHADER_FRAGMENT));
	std::unique_ptr<const RSC_GLShader> vertShader(make_unique<RSC_GLShader>(RSC_GLShader::LoadShaderFromFile(vertName), SHADER_VERTEX  ));

	if(fragShader->GetShaderID()!=0){K_ShaderMan.LoadItem(fragName, fragShader);}
	if(vertShader->GetShaderID()!=0){K_ShaderMan.LoadItem(vertName, vertShader);}

	std::unique_ptr<RSC_GLProgram> program = make_unique<RSC_GLProgram>();
	program->AddShader(K_ShaderMan.GetItem(fragName));
	program->AddShader(K_ShaderMan.GetItem(vertName));
	program->LinkProgram();
	program->Bind();

	return program;
}

void RenderManager::LinkShaderProgram(RSC_GLProgram* program){
	GLuint programHandle			= program->GetHandle();
	GLuint programUniformBlockHandle= program->GetUniformBlockHandle("CameraData");
	//Bind program GPU buffer to the index
	glUniformBlockBinding(programHandle, programUniformBlockHandle, CameraDataBindingIndex);
	try{
		GLuint programUniformBlockHandleProgramData= program->GetUniformBlockHandle("ProgramData");
		glUniformBlockBinding(programHandle, programUniformBlockHandleProgramData, ProgramDataBindingIndex);
	}
	catch(LEngineShaderProgramException e){
		//It's fine if gl can't link the uniform block
		//if the program doesn't use the program data block, the compiled code won't have one
		//which will throw an error
	}
}

void RenderManager::LoadDefaultShaders(){
	//Init gpu data if it hasn't been already
	if(GlobalCameraUBO==0){
		//Create memory location on GPU to store uniform camera data for ALL SHADER PROGRAMS
		//The memory location ID is then sent to each individual camera so that the cameras can bind
		//the needed data into the uniform buffer
		//This buffer stores a mat4 (proj matrix) and a vec2 (camera translation)
							//2 floats (padded to vec4)for vec2, 16 for matrix
		GLuint bufferSize=(sizeof(float)*4) + (sizeof(float)*16);
		glGenBuffers(1, &GlobalCameraUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, GlobalCameraUBO);
		glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);
		//Bind generated CPU buffer to the index
		glBindBufferBase(GL_UNIFORM_BUFFER, CameraDataBindingIndex, GlobalCameraUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);


		//Create memory location on GPU to store uniform data for ALL SHADER PROGRAMS
		//The memory location ID is then sent to each individual camera so that the cameras can bind
		//the needed data into the uniform buffer
			//This buffer stores the elapsed time
		bufferSize=(sizeof(float)*4);
		glGenBuffers(1, &GlobalProgramUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, GlobalProgramUBO);
		glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);
		//Bind generated CPU buffer to the index
		glBindBufferBase(GL_UNIFORM_BUFFER, ProgramDataBindingIndex, GlobalProgramUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	defaultProgramTile = K_ShaderProgramMan.GetItem(defaultProgramTileName);
	defaultProgramSprite = K_ShaderProgramMan.GetItem(defaultProgramSpriteName);
	defaultProgramLight = K_ShaderProgramMan.GetItem(defaultProgramLightName);
	defaultProgramImage = K_ShaderProgramMan.GetItem(defaultProgramImageName);

	if(defaultProgramSprite == NULL){
		std::string shaderFrag  = "fragmentSpriteMain.glsl";
		std::string shaderVert  = "vertexSpriteMain.glsl";
		auto shaderProgram = LoadShaderProgram(shaderVert, shaderFrag);
		LinkShaderProgram(shaderProgram.get());

		std::unique_ptr<const RSC_GLProgram> constProgramSprite = std::move(shaderProgram);
		K_ShaderProgramMan.LoadItem(defaultProgramSpriteName, constProgramSprite); 

		defaultProgramSprite = K_ShaderProgramMan.GetItem(defaultProgramSpriteName);
	}

	if(defaultProgramTile == NULL){
		std::string shaderFrag  = "fragmentTileMain.glsl";
		std::string shaderVert	 = "vertexTileMain.glsl";
		auto shaderProgram = LoadShaderProgram(shaderVert, shaderFrag);
		LinkShaderProgram(shaderProgram.get());

		std::unique_ptr<const RSC_GLProgram> constProgramSprite = std::move(shaderProgram);
		K_ShaderProgramMan.LoadItem(defaultProgramTileName, constProgramSprite); 

		defaultProgramTile = K_ShaderProgramMan.GetItem(defaultProgramTileName);
	}

	if(defaultProgramImage == NULL){
		std::string shaderFrag  = "fragmentImage.glsl";
		std::string shaderVert	 = "vertexImage.glsl";
		auto shaderProgram = LoadShaderProgram(shaderVert, shaderFrag);
		LinkShaderProgram(shaderProgram.get());

		std::unique_ptr<const RSC_GLProgram> constProgramSprite = std::move(shaderProgram);
		K_ShaderProgramMan.LoadItem(defaultProgramImageName, constProgramSprite); 

		defaultProgramImage = K_ShaderProgramMan.GetItem(defaultProgramImageName);
	}

	if(defaultProgramLight == NULL){
		std::string shaderFrag  = "fragmentLightMain.glsl";
		std::string shaderVert	 = "vertexLightMain.glsl";
		auto shaderProgram = LoadShaderProgram(shaderVert, shaderFrag);
		LinkShaderProgram(shaderProgram.get());

		std::unique_ptr<const RSC_GLProgram> constProgramSprite = std::move(shaderProgram);
		K_ShaderProgramMan.LoadItem(defaultProgramLightName, constProgramSprite); 

		defaultProgramLight = K_ShaderProgramMan.GetItem(defaultProgramLightName);
	}
}
