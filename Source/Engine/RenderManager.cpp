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
	: frameBufferTextureDiffuse(std::unique_ptr<RSC_Texture>(new RSC_Texture(SCREEN_W, SCREEN_H, 4, GL_RGBA)))
	, frameBufferTextureFinal  (std::unique_ptr<RSC_Texture>(new RSC_Texture(SCREEN_W, SCREEN_H, 4, GL_RGBA)))
	, dependencyRenderManager(rm){
	scale=1;
	rotation=0;
	SetView(viewPort);
	//view.x=0;
	//view.y=0;
	//view.w=CAMERA_W;
	//view.h=CAMERA_H;

	glGenFramebuffers(1, &FBO);

	dependencyRenderManager->AddCamera( this);
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

void RenderCamera::Bind(const GLuint& GlobalCameraUBO){
	//For the purpose of scaling and rotating the viewport, I may want to replace this vec4 with a projection matrix;
	/*Matrix4 T =translate (identity_mat4 (), vec3 (-view.x, -view.y, 0.0));
	Matrix4 R = rotate_z_deg (identity_mat4 (), 0.0f);
	Matrix4 view_mat = R * T;*/

	Matrix4 T= Matrix4::IdentityMatrix();
	T=T.Translate(Vec3 (-view.x, -view.y, 0.0));

	Matrix4 R= Matrix4::IdentityMatrix();
	R=R.RotateZ(0);

	Matrix4 S= Matrix4::IdentityMatrix();
	S= S.Scale(Vec3(4,4,1));


	//Create orthographic matrix
	// http://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/orthographic-projection-matrix

	//down directions is negative y; up direction is positive y
	//y gets flipped twice, once here, and once when applying the framebuffer
	float rightSide   = CAMERA_W;// - 1.0f;
	float leftSide	  = 0;
	float bottomSide  = 0;
	float topSide	  = CAMERA_H;// - 1.0f;
	float zFar		  = 1.1f;
	float zNear		  = 0.1f;

	float normalizedDeviceSpaceX =	2.0f / (rightSide  - leftSide);
	float normalizedDeviceSpaceY =	2.0f / (topSide    - bottomSide);
	float normalizedDeviceSpaceZ =	2.0f / (zFar	   - zNear);

	//view_mat is not currentl used, but could be; keep this commented out for now until you know what to do with it
	//Matrix4 view_mat = T * R * S;
	Matrix4 proj_mat = Matrix4::IdentityMatrix();

	proj_mat.m[ 0] = normalizedDeviceSpaceX;
	proj_mat.m[ 5] = normalizedDeviceSpaceY;
	proj_mat.m[10] = normalizedDeviceSpaceZ;
	proj_mat.m[12] = -(rightSide+leftSide)/(rightSide-leftSide);
	proj_mat.m[13] = -(topSide+bottomSide)/(topSide-bottomSide);
	proj_mat.m[14] = (zNear+zFar)/(zNear-zFar);
	proj_mat.m[15] = 1.0f;

	/*proj_mat.m[ 0] =	normalizedDeviceSpaceX;
	proj_mat.m[ 5] =  normalizedDeviceSpaceY;
	proj_mat.m[10] = -2.0f;

	proj_mat.m[12] = -1.0f;
	proj_mat.m[13] =  -1.0f;
	proj_mat.m[14] = 1.0f;
	proj_mat.m[15] =  1.0f;*/

	float position[4];
	//Render Using only full integers for translation to get that pixel-perfect look
	position[0]=view.x;
	position[1]=view.y;
	position[2]=CAMERA_W;
	position[3]=CAMERA_H;

	glBindBuffer(GL_UNIFORM_BUFFER, GlobalCameraUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0,				  (sizeof(float)*4),	 position);
	glBufferSubData(GL_UNIFORM_BUFFER, (sizeof(float)*4), (sizeof(float)*16),  proj_mat.m);
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

RenderCamera::~RenderCamera(){
	Kernel::stateMan.GetCurrentState()->renderMan.RemoveCamera(this);
}

void RenderCamera::RenderFrameBufferTextureFinal(){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	RSC_GLProgram::BindNULL();
	glBindVertexArray (0);

	//Back to initial viewport
	glPopAttrib();

	frameBufferTextureFinal.get()->Bind();
	float Left=		(float)0.0f		/ (float)frameBufferTextureFinal->GetWidth();
	float Right=	(float)SCREEN_W / (float)frameBufferTextureFinal->GetWidth();
	float Top=		(float)0.0f		/ (float)frameBufferTextureFinal->GetHeight();
	float Bottom=	(float)SCREEN_H / (float)frameBufferTextureFinal->GetHeight();

	glBegin(GL_QUADS);
		glTexCoord2f(Left,	Top);		glVertex3i(0,		 0,		   0);
		glTexCoord2f(Right, Top);		glVertex3i(CAMERA_W, 0,		   0);
		glTexCoord2f(Right, Bottom);	glVertex3i(CAMERA_W, CAMERA_H, 0);
		glTexCoord2f(Left,	Bottom);	glVertex3i(0,		 CAMERA_H, 0);
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
		ErrorLog::WriteToFile(ss.str(), ErrorLog::SEVERITY::FATAL);
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

	glLoadIdentity();
	//New Rendering

	if(!(mCameras).empty()){
		//World objects are rendered after the camera sets its matrix (this includes sprite batches)
		auto currentCamera = mCameras.begin();
		(*currentCamera)->Bind(GlobalCameraUBO);
		for(auto i=objectsWorld.begin(); i!=objectsWorld.end(); i++){
			if((*i)->render){(*i)->Render(*currentCamera);}
		}

		//Need better way to handle light
		Kernel::stateMan.GetCurrentState()->comLightMan.Render((*currentCamera)->GetFrameBufferTextureDiffuse(), (*currentCamera)->GetFrameBufferTextureFinal(), defaultProgramLight);

		(*currentCamera)->RenderFrameBufferTextureFinal();
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
	std::unique_ptr<const RSC_GLShader> vertShader(new RSC_GLShader(RSC_GLShader::LoadShaderFromFile(vertName),   SHADER_VERTEX ));

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
		std::string shaderFrag  = "Data/Resources/Shaders/fragmentSpriteMain.glsl";
		std::string shaderVert    = "Data/Resources/Shaders/vertexSpriteMain.glsl";
		auto shaderProgram = LoadShaderProgram(shaderVert, shaderFrag);
		LinkShaderProgram(shaderProgram.get());

		std::unique_ptr<const RSC_GLProgram> constProgramSprite = std::move(shaderProgram);
		K_ShaderProgramMan.LoadItem(defaultProgramSpriteName, constProgramSprite); 

		defaultProgramSprite = K_ShaderProgramMan.GetItem(defaultProgramSpriteName);
	}

	if(defaultProgramTile == NULL){
		std::string shaderFrag  = "Data/Resources/Shaders/fragmentTileMain.glsl";
		std::string shaderVert	 = "Data/Resources/Shaders/vertexTileMain.glsl";
		auto shaderProgram = LoadShaderProgram(shaderVert, shaderFrag);
		LinkShaderProgram(shaderProgram.get());

		std::unique_ptr<const RSC_GLProgram> constProgramSprite = std::move(shaderProgram);
		K_ShaderProgramMan.LoadItem(defaultProgramTileName, constProgramSprite); 

		defaultProgramTile = K_ShaderProgramMan.GetItem(defaultProgramTileName);
	}

	if(defaultProgramImage == NULL){
		std::string shaderFrag  = "Data/Resources/Shaders/fragmentImage.glsl";
		std::string shaderVert	 = "Data/Resources/Shaders/vertexImage.glsl";
		auto shaderProgram = LoadShaderProgram(shaderVert, shaderFrag);
		LinkShaderProgram(shaderProgram.get());

		std::unique_ptr<const RSC_GLProgram> constProgramSprite = std::move(shaderProgram);
		K_ShaderProgramMan.LoadItem(defaultProgramImageName, constProgramSprite); 

		defaultProgramImage = K_ShaderProgramMan.GetItem(defaultProgramImageName);
	}

	if(defaultProgramLight == NULL){
		std::string shaderFrag  = "Data/Resources/Shaders/fragmentLightMain.glsl";
		std::string shaderVert	 = "Data/Resources/Shaders/vertexLightMain.glsl";
		auto shaderProgram = LoadShaderProgram(shaderVert, shaderFrag);
		LinkShaderProgram(shaderProgram.get());

		std::unique_ptr<const RSC_GLProgram> constProgramSprite = std::move(shaderProgram);
		K_ShaderProgramMan.LoadItem(defaultProgramLightName, constProgramSprite); 

		defaultProgramLight = K_ShaderProgramMan.GetItem(defaultProgramLightName);
	}
}
