#ifndef LENGINE_COORDINATES
#define LENGINE_COORDINATES

#include "Defines.h"

class Coord2df{
    public:
        Coord2df()                     {x=0; y=0;  }
        Coord2df(float xx, float yy) {x=xx; y=yy;}
        float x, y;

        inline Coord2df operator+ (const Coord2df& c) const{
            Coord2df newCoord;
            newCoord.x = x + c.x;
            newCoord.y = y + c.y;
            return newCoord;
        }
        inline Coord2df operator- (const Coord2df& c) const{
            Coord2df newCoord;
            newCoord.x = x - c.x;
            newCoord.y = y - c.y;
            return newCoord;
        }
		inline Coord2df Round() const{
			return Coord2df( floor(x + 0.5f), floor(y + 0.5f) );
		}

		//Conversion from this class to ImGuiVec2 is in imguiconfig.h
		//in IM_VEC2_CLASS_EXTRA macro
};

//coordinate to tile grid and vise versa
inline void CoordToGrid(int &x, int &y){
    x=x/LENGINE_DEF_TILE_W;
    y=y/LENGINE_DEF_TILE_H;
};

inline void GridToCoord(int &x, int &y){
    x=x*LENGINE_DEF_TILE_W;
    y=y*LENGINE_DEF_TILE_H;
};


struct CollisionResponse{
	CollisionResponse(const Coord2df& vec, bool collided);
	CollisionResponse(float x, float y, bool collided);
	Coord2df mVector;
	bool mCollided;
};


//forward declare
class Circle; 
class CRect{
    public:
        CRect(const int& xx,const int& yy, const int& wi, const int& he){SetValues(xx, yy, wi, he);}
        CRect(){SetValues(0,0,0,0);}
        void SetValues(const int& xx, const int& yy,const int& wi, const int& he){
            x=xx; y=yy; w=wi; h=he;
        }

        inline int  GetWidth()              const   {return w;  }
        inline void SetWidth(const int& ww)         {w=ww;      }

        inline int  GetHeight()             const   {return h;  }
        inline void SetHeight(const int& hh)        {h=hh;      }

        inline int  GetX()                  const   {return x;  }
        inline void SetX(const int& xx)             {x=xx;      }

        inline int  GetY()                  const   {return y;  }
        inline void SetY(const int& yy)             {y=yy;      }

        int GetTop     () const;
        int GetBottom  () const;
        int GetLeft    () const;
        int GetRight   () const;
		Coord2df GetCenter() const;

		///returns -1 if false, angle from center if true
		CollisionResponse Contains(const Coord2df& point);
		CollisionResponse Contains(const CRect& r);
		CollisionResponse Contains(const Circle& r);

        int x, y, w, h;
};

class FloatRect{
    public:
        FloatRect(const float& xx, const float& yy, const float& wi, const float& he){SetValues(xx, yy, wi, he);}
        FloatRect(){SetValues(0,0,0,0);}
        void SetValues(const float& xx, const float& yy, const float& wi, const float& he){
            x=xx; y=yy; w=wi; h=he;
        }

        inline float GetWidth()       const     {return w;  }
        inline void  SetWidth(const float& ww)  {w=ww;      }

        inline float GetHeight()      const     {return h;  }
        inline void  SetHeight(const float& hh) {h=hh;      }

        inline float GetX()           const     {return x;  }
        inline void  SetX(const float& xx)      {x=xx;      }

        inline float GetY()           const     {return y;  }
        inline void  SetY(const float& yy)      {y=yy;      }

        float GetTop     ()  const;
        float GetBottom  ()  const;
        float GetLeft    ()  const;
        float GetRight   ()  const;

        float x, y, w, h;
};

class Circle{
	public:
		Circle(Coord2df pos, float radius);
		Circle(float x, float y, float radius);

        float GetTop     ()  const;
        float GetBottom  ()  const;
        float GetLeft    ()  const;
        float GetRight   ()  const;
	
        inline Coord2df GetPos()           const    {return mPos;  }
        inline void  SetPos(const Coord2df& pos)    {mPos=pos;     }

        inline float GetRadius()           const    {return mRadius;  }
        inline void  SetRadius(const float& radius) {mRadius=radius;  }

		///returns -1 if false, angle from center if true
		CollisionResponse Contains(const Coord2df& point);
		CollisionResponse Contains(const CRect& r);
		CollisionResponse Contains(const Circle& r);

	private:
		Coord2df mPos;
		float mRadius;
};

//These data structures can be treated as arrays by openGL
struct LVertexPos2D  { GLfloat x;             GLfloat y;          };
struct LTexCoord     { GLfloat s;             GLfloat t;          };
struct LVertexData2D { LVertexPos2D position; LTexCoord texCoord; };

///returns -1 if false, angle from center if true
CollisionResponse CollisionRectRect(const CRect& RR, const CRect& R);
CollisionResponse CollisionRectCircle(const CRect& R, const Circle C);
CollisionResponse CollisionRectPoint(const CRect& R, const Coord2df& point);
CollisionResponse CollisionCircleCircle(const Circle& CC, const Circle& C);
CollisionResponse CollisionCirclePoint(const Circle& CC, const Coord2df& point);
#endif
