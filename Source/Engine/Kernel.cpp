#include "Kernel.h"
#include "gui/imgui_LEngine.h"

SDLInit*					Kernel::SDLMan;
GameStateManager			Kernel::stateMan;
InputManager				Kernel::inputMan;
CommandLineArgs				Kernel::commandLine;
AudioSubsystem				Kernel::audioSubsystem;

int			 Kernel::gameLoops;
unsigned int Kernel::nextGameTick;
int			 Kernel::returnValue;
bool 		Kernel::debugMode;

GenericContainer<RSC_Sprite>	Kernel::rscSpriteMan;
GenericContainer<RSC_Texture>	Kernel::rscTexMan;
GenericContainer<RSC_Sound>		Kernel::rscSoundMan;
GenericContainer<RSC_Music>		Kernel::rscMusicMan;
GenericContainer<RSC_Script>	Kernel::rscScriptMan;
GenericContainer<RSC_Map>		Kernel::rscMapMan;
GenericContainer<RSC_GLShader>	Kernel::rscShaderMan;
GenericContainer<RSC_GLProgram>	Kernel::rscShaderProgramMan;
GenericContainer<RSC_Font>		Kernel::rscFontMan;

ImGuiState Kernel::guiState;

Kernel::Kernel(){}
Kernel::~Kernel(){}

void Kernel::Close(){
	ErrorLog::WriteToFile("Closing...", ErrorLog::GenericLogFile);

	stateMan.Close();
	ErrorLog::CloseFiles();
	SDLMan->CloseSDL();

	rscTexMan	 .Clear();
	rscSpriteMan .Clear();
	rscMusicMan  .Clear();
	rscSoundMan  .Clear();
	rscScriptMan .Clear();
	rscMapMan	 .Clear();
	rscShaderMan	 .Clear();
	rscShaderProgramMan	 .Clear();
	rscFontMan	 .Clear();

	ImGui::Shutdown();
}

void Kernel::Inst(){
	int argc=0; char *argv[0];
	Kernel::Inst(argc, argv);
}
void Kernel::Inst(int argc, char *argv[]){
	ErrorLog::Inst();
	ErrorLog::OpenFile(ErrorLog::GenericLogFile);
	ErrorLog::WriteToFile("Starting up...", ErrorLog::GenericLogFile);

	PHYSFS_init(NULL);
	std::string searchPath="Data/";
	PHYSFS_addToSearchPath(searchPath.c_str(), 0);

    const char * physfsError = PHYSFS_getLastError();
    if(physfsError!=NULL){
        std::stringstream ss;
        ss << "Physfs Error in Kernel Inst; Error: " << physfsError;
        ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
    }

	debugMode = (commandLine.Exists(L_CMD_DEBUG) == true);

	SDLMan=SDLInit::Inst();
	SDLMan->InitSDL();
	/// \TODO remove this, have InitSDL intialize everything
	SDLMan->InitOpenGL();

	rscTexMan		.SetLoadFunction(&RSC_Texture::LoadResource   );
	rscSpriteMan	.SetLoadFunction(&RSC_Sprite::LoadResource	 );
	rscMusicMan		.SetLoadFunction(&RSC_Music::LoadResource	);
	rscSoundMan		.SetLoadFunction(&RSC_Sound::LoadResource	);
	rscScriptMan	.SetLoadFunction(&RSC_Script::LoadResource);
	rscMapMan		.SetLoadFunction(&RSC_MapImpl::LoadResource   );
	rscFontMan		.SetLoadFunction(&RSC_Font::LoadResource   );

	commandLine.ParseArgs(argc, argv);

	gameLoops=0;
	nextGameTick=SDL_GetTicks() - 1;

	Kernel::stateMan.PushState(make_unique<GameStartState>(&Kernel::stateMan) );
	Kernel::stateMan.PushNextState();
}

bool Kernel::DEBUG_MODE(){
	return debugMode;
}

bool Kernel::Update(){
	ImGuiNewFrame(SDLMan->mMainWindow);

	if(debugMode){
		ImGui::ShowMetricsWindow();
	}

	nextGameTick = SDL_GetTicks() + SKIP_TICKS;
	returnValue=stateMan.Update();

	//Audio subsystem can be put on a different thread
	audioSubsystem.ProcessEvents();

	gameLoops++;

	//If we're behind, skip drawing
	//Don't skip if the max amount of frame skip has been passed
	if( (SDL_GetTicks()<nextGameTick) or (gameLoops>MAX_FRAMESKIP) ){
		//game render
		stateMan.Draw();
		glFinish();
	}
	SDL_GL_SwapWindow(SDLMan->GetWindow());

	return returnValue;
}

bool Kernel::Run(){
	gameLoops=0;

	//loop seems to be locked to 60fps no matter what?
	while(SDL_GetTicks()>nextGameTick) {
		if(Update() != true){
			return false;
		}
	}
	return true;
}

void Kernel::ImGuiCreateDeviceObjects(){
    // Backup GL state
    GLint last_texture, last_array_buffer, last_vertex_array;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

	if(guiState.shaderHandle.get()  == NULL){
		const GLchar *vertex_shader =
			"#version 300 es\n"
			"precision mediump float;\n"
			"uniform mat4 ProjMtx;\n"
			"in vec2 Position;\n"
			"in vec2 UV;\n"
			"in vec4 Color;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"	Frag_UV = UV;\n"
			"	Frag_Color = Color;\n"
			"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const GLchar* fragment_shader =
			"#version 300 es\n"
			"precision mediump float;\n"
			"uniform sampler2D Texture;\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
			"}\n";

		guiState.vertHandle = make_unique<RSC_GLShader>(vertex_shader, SHADER_VERTEX);
		guiState.fragHandle = make_unique<RSC_GLShader>(fragment_shader, SHADER_FRAGMENT);
		guiState.shaderHandle = make_unique<RSC_GLProgram>();

		if(guiState.vertHandle->IsUsable() == false){
			ErrorLog::WriteToFile("Couldn't load ImGui Vertex Shader", ErrorLog::GenericLogFile);
			throw LEngineException("Imgui No Vertex Shader");
		}
		if(guiState.fragHandle->IsUsable() == false){
			ErrorLog::WriteToFile("Couldn't load ImGui Fragment Shader", ErrorLog::GenericLogFile);
			throw LEngineException("Imgui No Fragment Shader");
		}
		
		guiState.shaderHandle->AddShader(guiState.vertHandle.get());
		guiState.shaderHandle->AddShader(guiState.fragHandle.get());
		guiState.shaderHandle->LinkProgram();
		guiState.shaderHandle->Bind();

		guiState.attribLocationTex = glGetUniformLocation(guiState.shaderHandle->GetHandle(), "Texture");
		guiState.attribLocationProjMtx = glGetUniformLocation(guiState.shaderHandle->GetHandle(), "ProjMtx");
		guiState.attribLocationPosition = glGetAttribLocation(guiState.shaderHandle->GetHandle(), "Position");
		guiState.attribLocationUV = glGetAttribLocation(guiState.shaderHandle->GetHandle(), "UV");
		guiState.attribLocationColor = glGetAttribLocation(guiState.shaderHandle->GetHandle(), "Color");

		glGenBuffers(1, &guiState.vboHandle);
		glGenBuffers(1, &guiState.elementsHandle);

		glGenVertexArrays(1, &guiState.vaoHandle);
		glBindVertexArray(guiState.vaoHandle);
		glBindBuffer(GL_ARRAY_BUFFER, guiState.vboHandle);
		glEnableVertexAttribArray(guiState.attribLocationPosition);
		glEnableVertexAttribArray(guiState.attribLocationUV);
		glEnableVertexAttribArray(guiState.attribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
		glVertexAttribPointer(guiState.attribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(guiState.attribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
		glVertexAttribPointer(guiState.attribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

	}

	if(guiState.fontTexture == 0){
		ImGuiCreateFontsTexture();
	}

    // Restore modified GL state
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindVertexArray(last_vertex_array);
}

void Kernel::ImGuiCreateFontsTexture(){
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &guiState.fontTexture);
    glBindTexture(GL_TEXTURE_2D, guiState.fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)guiState.fontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);
}

void Kernel::ImGuiNewFrame(SDL_Window* window){
    if (!guiState.fontTexture){
        ImGuiCreateDeviceObjects();
	}

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_GL_GetDrawableSize(window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);
	guiState.projectionMatrix = Matrix4::OrthoGraphicProjectionMatrix(Coord2df(w,h));

    // Setup time step
    Uint32	time = SDL_GetTicks();
    double current_time = time / 1000.0;
    io.DeltaTime = guiState.time > 0.0 ? (float)(current_time - guiState.time) : (float)(1.0f / 60.0f);
    guiState.time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
    int mx, my;
    Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
        io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    else
        io.MousePos = ImVec2(-1, -1);

    io.MouseDown[0] = guiState.mousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[1] = guiState.mousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2] = guiState.mousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    guiState.mousePressed[0] = guiState.mousePressed[1] = guiState.mousePressed[2] = false;

    io.MouseWheel = guiState.mouseWheel;
    guiState.mouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    SDL_ShowCursor(io.MouseDrawCursor ? 0 : 1);

    // Start the frame
    ImGui::NewFrame();
}

void Kernel::ImGuiInvalidateFontTexture(){
	//Delete Font texture and it will be reloaded next frame
	if(Kernel::guiState.fontTexture != 0 ){
		glDeleteTextures(1, &Kernel::guiState.fontTexture);
		Kernel::guiState.fontTexture = 0;
	}
}
