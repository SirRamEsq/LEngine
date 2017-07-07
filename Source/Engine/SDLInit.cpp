#include "SDLInit.h"
//int SCREENW=800;
//int SCREENH=600;
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

    if(SDL_Init(SDL_INIT_EVERYTHING)==-1){ErrorLog::WriteToFile("Didn't init SDL properly", ErrorLog::GenericLogFile); return;}

    /*SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    if(SDL_SetVideoMode(SCREEN_W, SCREEN_H, SCREEN_BPP, SDL_OPENGL)==NULL){ErrorLog::WriteToFile("Didn't start window"); return;}
    if(InitOpenGL()==0){ErrorLog::WriteToFile("Didn't Initialize OpenGL"); return;}

    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);*/

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    mMainWindow= SDL_CreateWindow(   "LEngine", //name
                                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, //x,y,w,h
                                    SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS); //flags

    mMainContextGL = SDL_GL_CreateContext(mMainWindow);

    if(InitOpenGL()==0){ErrorLog::WriteToFile("Didn't Initialize OpenGL", ErrorLog::GenericLogFile); return;}


    if(Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096)==-1){ErrorLog::WriteToFile("Didn't init SDL Mixer properly", ErrorLog::GenericLogFile); return;}
    int flags=MIX_INIT_OGG;
    int initted=Mix_Init(flags);
    if(initted&flags != flags) {
        std::stringstream ss;
        ss << "SDL_Mix_Init: Failed to init required ogg and mod support!" << "\n"
        << "SDL_Mix_Init: " << Mix_GetError();
        ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
    }

    if(TTF_Init()==-1) {
        ErrorLog::WriteToFile("SDL_TTF_INIT: Couldn't init SDL_TTF!", ErrorLog::GenericLogFile);
    }
    //defaultFont = TTF_OpenFont( "Data/Resources/Fonts/ebFonts/fourside.ttf",      12 );
    defaultFont = TTF_OpenFont( "Data/Resources/Fonts/ebFonts/lumine_hall.ttf",     60 );
    //defaultFont = TTF_OpenFont( "Data/Resources/Fonts/ebFonts/apple_kid.ttf",     32 );
    //defaultFont = TTF_OpenFont( "Data/Resources/Fonts/ebFonts/twoson.ttf",        16 );
    //defaultFont = TTF_OpenFont( "Data/Resources/Fonts/ebFonts/gasfont.ttf",       32 );
    //defaultFont = TTF_OpenFont( "Data/Resources/Fonts/XXRaytid.ttf",              28 );
    if(defaultFont==NULL){
        ErrorLog::WriteToFile("Couldn't load ttf", ErrorLog::GenericLogFile);
    }

    GLenum err = glewInit();
    if (GLEW_OK != err){
        std::stringstream ss;
        ss << "Error in GLEW INIT: " << glewGetErrorString(err);
        ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
    }

    SDL_GL_SetSwapInterval(1);
    //DO SOMETHING ABOUT UNICODE
    //SDL_EnableUNICODE(1);
}
