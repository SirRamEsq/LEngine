#ifndef LENGINE_RENDER_PRIMITIVES
#define LENGINE_RENDER_PRIMITIVES

#include "RenderableObject.h"
#include "../SDLInit.h"

class LTexture;
class RenderText : public RenderableObjectScreen{
    public:
        RenderText(RenderManager* rm, int xv, int yv, std::string t, bool abso=true);
        ~RenderText();

        void SetColorI(int r, int g, int b){
            color.r=r;
            color.g=g;
            color.b=b;
            del=true;
        }
        void SetColor(SDL_Color c){
            color=c;
            del=true;
        }

        SDL_Color GetColor(){return color;}

        void ChangeText(std::string newText){
            del=true;
            text=newText;
        }
        void ChangePosition(int xx, int yy){
            SetX(xx);
            SetY(yy);
        }
        void SetX(int xx){x=xx;}
        void SetY(int yy){y=yy;}
        void SetW(int ww){w=ww;}
        void SetH(int hh){h=hh;}

        int GetX(){return x;}
        int GetY(){return y;}
        int GetW(){return w;}
        int GetH(){return h;}

        void Render(L_GL_Program* program);

        void DeleteTexture();

    private:
        std::string text;
        int x, y, w, h;
        bool absolute, del;
        SDL_Color color;
        LTexture* mTexture;
        GLuint VBOID;
        GLuint IBOID;
};

class RenderLine : public RenderableObjectScreen{
    public:
        RenderLine(RenderManager* rm, int x1, int y1, int x2, int y2, bool absolute=true);
        ~RenderLine();

        void SetColorI(int r, int g, int b){
            color.r=r;
            color.g=g;
            color.b=b;
        }
        inline void SetColor(SDL_Color c){
            color=c;
        }

        inline SDL_Color GetColor(){return color;}

        void ChangePosition(int x1, int y1, int x2, int y2){
            SetX1(x1);
            SetY1(y1);
            SetX2(x2);
            SetY2(y2);
        }
        void SetX1(int xx){mX1=xx;}
        void SetY1(int yy){mY1=yy;}
        void SetX2(int xx){mX2=xx;}
        void SetY2(int yy){mY2=yy;}

        int GetX1(){return mX1;}
        int GetY1(){return mY1;}
        int GetX2(){return mX2;}
        int GetY2(){return mY2;}

        void Render(L_GL_Program* program);

    private:
        int mX1, mY1, mX2, mY2;
        bool mAbsolute;
        SDL_Color color;
};

class RenderBox : public RenderableObjectScreen{
    public:
        RenderBox(RenderManager* rm, int x1, int y1, int x2, int y2, bool absolute=true);
        ~RenderBox();

        void SetColorI(int r, int g, int b){
            color.r=r;
            color.g=g;
            color.b=b;
        }
        inline void SetColor(SDL_Color c){
            color=c;
        }

        inline SDL_Color GetColor(){return color;}

        void ChangePosition(int x1, int y1, int x2, int y2){
            SetX1(x1);
            SetY1(y1);
            SetX2(x2);
            SetY2(y2);
        }
        void SetX1(int xx){mX1=xx;}
        void SetY1(int yy){mY1=yy;}
        void SetX2(int xx){mX2=xx;}
        void SetY2(int yy){mY2=yy;}

        int GetX1(){return mX1;}
        int GetY1(){return mY1;}
        int GetX2(){return mX2;}
        int GetY2(){return mY2;}

        void Render(L_GL_Program* program);

    private:
        int mX1, mY1, mX2, mY2;
        bool mAbsolute;
        SDL_Color color;
};

#endif // LENGINE_RENDER_PRIMITIVES
