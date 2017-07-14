#include "RenderManager.h"

#include "Components/CompSprite.h"
#include "Kernel.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

#include "Resources/RSC_Map.h"


////////////////
//RenderCamera//
////////////////

RenderCamera::RenderCamera(RenderManager* rm)
	: frameBufferTextureDiffuse(std::unique_ptr<LTexture>(new LTexture(SCREEN_W, SCREEN_H, 4, GL_RGBA)))
	, frameBufferTextureFinal  (std::unique_ptr<LTexture>(new LTexture(SCREEN_W, SCREEN_H, 4, GL_RGBA)))
	, dependencyRenderManager(rm){
	scale=1;
	rotation=0;
	view.x=0;
	view.y=0;
	view.w=CAMERA_W;
	view.h=CAMERA_H;

	glGenFramebuffers(1, &FBO);

	dependencyRenderManager->AddCamera( this);
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

	Matrix4 view_mat = T * R * S;
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
	glClearColor(1.0,1.0,1.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	//Atatch buffer texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTextureDiffuse->GetOpenGLID(), 0);
}
RenderCamera::~RenderCamera(){
	Kernel::stateMan.GetCurrentState()->renderMan.RemoveCamera(this);
}

void RenderCamera::RenderFrameBufferTextureFinal(){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	L_GL_Program::BindNULL();
	glBindVertexArray (NULL);

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

RenderManager::RenderManager()
	: CameraDataBindingIndex(1), ProgramDataBindingIndex(2), timeElapsed(0){
	listChange=false;
	nextTextID=0;

	/// \TODO - RenderMan shader init code needs moved somewhere now that RenderManager isn't a singleton. maybe into it's own Generic Resource container attached to the Kernel?

	shaderFragmentNameSpriteBatch  = "Data/Resources/Shaders/fragmentSpriteMain.glsl";
	shaderVertexNameSpriteBatch    = "Data/Resources/Shaders/vertexSpriteMain.glsl";

	std::unique_ptr<const L_GL_Shader> shaderFragmentSpriteBatch (new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderFragmentNameSpriteBatch), SHADER_FRAGMENT));
	std::unique_ptr<const L_GL_Shader> shaderVertexSpriteBatch	 (new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderVertexNameSpriteBatch),   SHADER_VERTEX ));

	if(shaderFragmentSpriteBatch->GetShaderID()!=0) {K_ShaderMan.LoadItem(shaderFragmentNameSpriteBatch, shaderFragmentSpriteBatch);}
	if(shaderVertexSpriteBatch->GetShaderID()!=0)	{K_ShaderMan.LoadItem(shaderVertexNameSpriteBatch, shaderVertexSpriteBatch);}

	shaderProgramSpriteBatch.AddShader(K_ShaderMan.GetItem(shaderFragmentNameSpriteBatch));
	shaderProgramSpriteBatch.AddShader(K_ShaderMan.GetItem(shaderVertexNameSpriteBatch));
	shaderProgramSpriteBatch.LinkProgram();
	shaderProgramSpriteBatch.Bind();



	shaderFragmentNameTileLayer  = "Data/Resources/Shaders/fragmentTileMain.glsl";
	shaderVertexNameTileLayer	 = "Data/Resources/Shaders/vertexTileMain.glsl";

	std::unique_ptr<const L_GL_Shader> shaderFragmentTileLayer(new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderFragmentNameTileLayer), SHADER_FRAGMENT));
	std::unique_ptr<const L_GL_Shader> shaderVertexTileLayer  (new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderVertexNameTileLayer),	 SHADER_VERTEX ));

	if(shaderFragmentTileLayer->GetShaderID()!=0) {K_ShaderMan.LoadItem(shaderFragmentNameTileLayer, shaderFragmentTileLayer);}
	if(shaderVertexTileLayer->GetShaderID()!=0)   {K_ShaderMan.LoadItem(shaderVertexNameTileLayer, shaderVertexTileLayer);}

	shaderProgramTileLayer.AddShader(K_ShaderMan.GetItem(shaderFragmentNameTileLayer));
	shaderProgramTileLayer.AddShader(K_ShaderMan.GetItem(shaderVertexNameTileLayer));
	shaderProgramTileLayer.LinkProgram();
	shaderProgramTileLayer.Bind();


	shaderFragmentNameImage  = "Data/Resources/Shaders/fragmentImage.glsl";
	shaderVertexNameImage	 = "Data/Resources/Shaders/vertexImage.glsl";

	std::unique_ptr<const L_GL_Shader> shaderFragmentImage(new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderFragmentNameImage), SHADER_FRAGMENT));
	std::unique_ptr<const L_GL_Shader> shaderVertexImage  (new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderVertexNameImage),	 SHADER_VERTEX ));

	if(shaderFragmentImage->GetShaderID()!=0) {K_ShaderMan.LoadItem(shaderFragmentNameImage, shaderFragmentImage);}
	if(shaderVertexImage->GetShaderID()!=0)   {K_ShaderMan.LoadItem(shaderVertexNameImage, shaderVertexImage);}

	shaderProgramImage.AddShader(K_ShaderMan.GetItem(shaderFragmentNameImage));
	shaderProgramImage.AddShader(K_ShaderMan.GetItem(shaderVertexNameImage));
	shaderProgramImage.LinkProgram();
	shaderProgramImage.Bind();



	shaderFragmentNameLight  = "Data/Resources/Shaders/fragmentLightMain.glsl";
	shaderVertexNameLight	 = "Data/Resources/Shaders/vertexLightMain.glsl";

	std::unique_ptr<const L_GL_Shader> shaderFragmentLight(new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderFragmentNameLight), SHADER_FRAGMENT));
	std::unique_ptr<const L_GL_Shader> shaderVertexLight  (new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderVertexNameLight),	 SHADER_VERTEX ));

	if(shaderFragmentLight->GetShaderID()!=0) {K_ShaderMan.LoadItem(shaderFragmentNameLight, shaderFragmentLight);}
	if(shaderVertexLight->GetShaderID()!=0)   {K_ShaderMan.LoadItem(shaderVertexNameLight, shaderVertexLight);}

	shaderProgramLight.AddShader(K_ShaderMan.GetItem(shaderFragmentNameLight));
	shaderProgramLight.AddShader(K_ShaderMan.GetItem(shaderVertexNameLight));
	shaderProgramLight.LinkProgram();
	shaderProgramLight.Bind();


	//this line keeps from reinitialize the GPU data if it's already been set
	//All this init code needs moved/changed now that RenderManager isn't a singleton
	if(GlobalCameraUBO!=0){return;}

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


	if(GlobalProgramUBO!=0){return;}
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

	//Link uniform buffer location with each program
	//Sprite batch program
	GLuint programHandle			= shaderProgramSpriteBatch.GetHandle();
	GLuint programUniformBlockHandle= shaderProgramSpriteBatch.GetUniformBlockHandle("CameraData");
	//Bind program GPU buffer to the index
	glUniformBlockBinding(programHandle, programUniformBlockHandle, CameraDataBindingIndex);
	try{
		GLuint programUniformBlockHandleProgramData= shaderProgramSpriteBatch.GetUniformBlockHandle("ProgramData");
		glUniformBlockBinding(programHandle, programUniformBlockHandleProgramData, ProgramDataBindingIndex);
	}
	catch(LEngineShaderProgramException e){
		//if the program doesn't use the program data block, the compiled code won't have one
	}


	//Tile layer shader shader setup
	programHandle			 = shaderProgramTileLayer.GetHandle();
	programUniformBlockHandle= shaderProgramTileLayer.GetUniformBlockHandle("CameraData");
	//Bind program GPU buffer to the index
	glUniformBlockBinding(programHandle, programUniformBlockHandle, CameraDataBindingIndex);
	try{
		GLuint programUniformBlockHandleProgramData= shaderProgramTileLayer.GetUniformBlockHandle("ProgramData");
		glUniformBlockBinding(programHandle, programUniformBlockHandleProgramData, ProgramDataBindingIndex);
	}
	catch(LEngineShaderProgramException e){
		//if the program doesn't use the program data block, the compiled code won't have one
	}


	//Image layer shader shader setup
	programHandle			 = shaderProgramImage.GetHandle();
	programUniformBlockHandle= shaderProgramImage.GetUniformBlockHandle("CameraData");
	//Bind program GPU buffer to the index
	glUniformBlockBinding(programHandle, programUniformBlockHandle, CameraDataBindingIndex);
	try{
		GLuint programUniformBlockHandleProgramData= shaderProgramImage.GetUniformBlockHandle("ProgramData");
		glUniformBlockBinding(programHandle, programUniformBlockHandleProgramData, ProgramDataBindingIndex);
	}
	catch(LEngineShaderProgramException e){
		//if the program doesn't use the program data block the compiled code won't have one
	}

	//Light shader shader setup
	programHandle			 = shaderProgramLight.GetHandle();
	programUniformBlockHandle= shaderProgramLight.GetUniformBlockHandle("CameraData");
	//Bind program GPU buffer to the index
	glUniformBlockBinding(programHandle, programUniformBlockHandle, CameraDataBindingIndex);
	try{
		GLuint programUniformBlockHandleProgramData= shaderProgramLight.GetUniformBlockHandle("ProgramData");
		glUniformBlockBinding(programHandle, programUniformBlockHandleProgramData, ProgramDataBindingIndex);
	}
	catch(LEngineShaderProgramException e){
		//if the program doesn't use the program data block, the compiled code won't have one
	}

}

void RenderManager::OrderOBJs(){
	//Negative Depth is closer to the screen
	objectsScreen.sort(&LOrderOBJs);
	objectsWorld.sort(&LOrderOBJs);
	listChange=false;
}

void RenderManager::Render(){
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
			if((*i)->render){(*i)->Render();}
		}

		Coord2d pos((*currentCamera)->GetX(), (*currentCamera)->GetY());
		Kernel::stateMan.GetCurrentState()->comLightMan.Render((*currentCamera)->GetFrameBufferTextureDiffuse(), (*currentCamera)->GetFrameBufferTextureFinal(), pos, &shaderProgramLight);

		(*currentCamera)->RenderFrameBufferTextureFinal();
	}

	//Screen objects are rendered after others, but do not use the camera's matrix
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	L_GL_Program::BindNULL();
	glBindVertexArray (NULL);

	for(auto i=objectsScreen.begin(); i!=objectsScreen.end(); i++){
		if((*i)->render){(*i)->Render();}
	}
}

void RenderManager::AssignCameraUBO(L_GL_Program* program){
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
	objectsWorld.push_back(obj);
	listChange=true;

	//Set Correct Shader
	if(obj->type == RenderableObject::TYPE::SpriteBatch){
		obj->SetShaderProgram(&shaderProgramSpriteBatch);
	}
	if(obj->type == RenderableObject::TYPE::TileLayer){
		obj->SetShaderProgram(&shaderProgramTileLayer);
		//obj->BuildVAO();
	}
	if(obj->type == RenderableObject::TYPE::Image){
		obj->SetShaderProgram(&shaderProgramImage);
		//obj->BuildVAO();
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

