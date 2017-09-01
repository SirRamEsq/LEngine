#include "SDLInit.h"
#include "gui/imgui.h"
#include "Kernel.h"

SDLInit* SDLInit::pointertoself=NULL;
SDLInit::SDLInit(){}
SDL_Window* SDLInit::mMainWindow = NULL;
SDL_GLContext SDLInit::mMainContextGL;
TTF_Font *defaultFont = NULL;

SDLInit* SDLInit::Inst(){
    pointertoself=new SDLInit();
    return pointertoself;
}

SDL_Window* SDLInit::GetWindow(){
    return mMainWindow;
}

void SDLInit::CloseSDL(){
    TTF_CloseFont(defaultFont);
    TTF_Quit();
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
}

bool InitImgGui(SDL_Window* window){
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;                     // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDLK_a;
    io.KeyMap[ImGuiKey_C] = SDLK_c;
    io.KeyMap[ImGuiKey_V] = SDLK_v;
    io.KeyMap[ImGuiKey_X] = SDLK_x;
    io.KeyMap[ImGuiKey_Y] = SDLK_y;
    io.KeyMap[ImGuiKey_Z] = SDLK_z;

    //io.RenderDrawListsFn = ImGui_ImplSdlGL3_RenderDrawLists;   // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    //io.SetClipboardTextFn = ImGui_ImplSdlGL3_SetClipboardText;
    //io.GetClipboardTextFn = ImGui_ImplSdlGL3_GetClipboardText;
    //io.ClipboardUserData = NULL;

#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    io.ImeWindowHandle = wmInfo.info.win.window;
#else
    (void)window;
#endif

    return true;
}

bool SDLInit::InitOpenGL(){
    glViewport(0, 0, SCREEN_W, SCREEN_H);
    //glClearDepth(1.0f);                         // Depth Buffer Setup
    //glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Test To Do
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D); //Enable Texture Mapping
    glClearColor(0, 0, 0, 0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //Bottom is the height, Top is 0
    glOrtho(0.0f, CAMERA_W, CAMERA_H, 0, 1, -1); //2D
    //gluPerspective(45.0,(GLfloat)SCREEN_W/(GLfloat)SCREEN_H,0.1,100.0); //3D

     glClearColor( 1.f, 0.f, 1.f, 1.f );

    //Initialize modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glEnable( GL_COLOR_MATERIAL);

    SDL_GL_SetSwapInterval(1);

    if(glGetError()!=GL_NO_ERROR){return 0;}
    return 1;
}

void SDLInit::InitSDL(){

    if(SDL_Init(SDL_INIT_EVERYTHING)==-1){K_Log.Write("Didn't init SDL properly"); return;}

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    mMainWindow= SDL_CreateWindow(   "LEngine", //name
                                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, //x,y,w,h
                                    SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS); //flags

    mMainContextGL = SDL_GL_CreateContext(mMainWindow);

    if(InitOpenGL()==0){K_Log.Write("Didn't Initialize OpenGL"); return;}


    if(Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096)==-1){K_Log.Write("Didn't init SDL Mixer properly"); return;}
    int flags=MIX_INIT_OGG;
    int initted=Mix_Init(flags);
    if( (initted&flags) != flags) {
        std::stringstream ss;
        ss << "SDL_Mix_Init: Failed to init required ogg and mod support!" << "\n"
        << "SDL_Mix_Init: " << Mix_GetError();
        K_Log.Write(ss.str());
    }

    if(TTF_Init()==-1) {
        K_Log.Write("SDL_TTF_INIT: Couldn't init SDL_TTF!");
    }
    //defaultFont = TTF_OpenFont( "Data/Resources/Fonts/ebFonts/fourside.ttf",      12 );
    defaultFont = TTF_OpenFont( "Data/Resources/Fonts/ebFonts/lumine_hall.ttf",     60 );
    //defaultFont = TTF_OpenFont( "Data/Resources/Fonts/ebFonts/apple_kid.ttf",     32 );
    //defaultFont = TTF_OpenFont( "Data/Resources/Fonts/ebFonts/twoson.ttf",        16 );
    //defaultFont = TTF_OpenFont( "Data/Resources/Fonts/ebFonts/gasfont.ttf",       32 );
    //defaultFont = TTF_OpenFont( "Data/Resources/Fonts/XXRaytid.ttf",              28 );
    if(defaultFont==NULL){
        K_Log.Write("Couldn't load ttf");
    }

    GLenum err = glewInit();
    if (GLEW_OK != err){
        std::stringstream ss;
        ss << "Error in GLEW INIT: " << glewGetErrorString(err);
        K_Log.Write(ss.str());
    }

	if(InitImgGui(mMainWindow)!=true){
		K_Log.Write("Couldn't Initialize ImgGui");
	}

    SDL_GL_SetSwapInterval(1);
    //DO SOMETHING ABOUT UNICODE
    //SDL_EnableUNICODE(1);
}
