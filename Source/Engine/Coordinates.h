#ifndef LENGINE_COORDINATES
#define LENGINE_COORDINATES

#include "Defines.h"

class Coord2df{
    public:
        Coord2df()                     {x=0; y=0;  }
        Coord2df(float xx, float yy) {x=xx; y=yy;}
        float x, y;

        inline Coord2df operator+ (const Coord2df& c) const{
            return Coord2df(x + c.x, y + c.y);
        }
        inline Coord2df operator- (const Coord2df& c) const{
            return Coord2df(x - c.x, y - c.y);
        }
        inline Coord2df operator+ (float f) const{
            return Coord2df(x + f, y + f);
        }
        inline Coord2df operator- (float f) const{
            return Coord2df(x - f, y - f);
        }
        inline void operator= (float f){
			x = f;
			y = f;
        }
		inline Coord2df Round() const{
			return Coord2df( floor(x + 0.5f), floor(y + 0.5f) );
		}
		inline Coord2df Truncate() const{
			return Coord2df( trunc(x), trunc(y) );
		}

		///Lua Interface Support Function
		Coord2df Add(const Coord2df& c){
			return (*this) + c;
		}
		Coord2df Subtract(const Coord2df& c){
			return (*this) - c;
		}

		//Conversion from this class to ImGuiVec2 is in imguiconfig.h
		//in IM_VEC2_CLASS_EXTRA macro
};

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

class Shape{
	public:
		Shape(Coord2df pos);
		Shape(float xx, float yy);

		virtual float GetLeft()   const = 0;
		virtual float GetRight()  const = 0;
		virtual float GetTop()    const = 0;
		virtual float GetBottom() const = 0;

		virtual Coord2df GetCenter() const = 0;

        inline float  GetX()                  const   {return x;  }
        inline void   SetX(const float& xx)           {x = xx;    }

        inline float  GetY()                  const   {return y;  }
        inline void   SetY(const float& yy)           {y = yy;    }

		inline Coord2df GetPos()				   const 	{return Coord2df(x,y);}
		inline void     SetPos(const Coord2df pos) 			{x = pos.x; y = pos.y;}

		float x, y;
};


//forward declare
class Circle; 
class CRect : public Shape{
    public:
        CRect(float xx, float yy, float ww, float hh);
		CRect();

        inline float  GetW()              const   {return w;  }
        inline void   SetW(const float& ww)          {w = ww;    }

        inline float  GetH()             const   {return h;  }
        inline void   SetH(const float& hh)         {h = hh;     }

		inline Coord2df GetSize()			  const 	{return Coord2df(w,h);}
		inline void     SetSize(Coord2df size) 			{w = size.x; h = size.y;}

        float GetTop     () const;
        float GetBottom  () const;
        float GetLeft    () const;
        float GetRight   () const;
		Coord2df GetCenter() const;

		CollisionResponse Contains(const Coord2df& point);
		CollisionResponse Contains(const CRect& r);
		CollisionResponse Contains(const Circle& r);

		///Width and height of Rect
		float w, h;
};

class Circle : public Shape{
	public:
		Circle(Coord2df pos, float radius);
		Circle(float xx, float yy, float radius);

        float GetTop     ()  const;
        float GetBottom  ()  const;
        float GetLeft    ()  const;
        float GetRight   ()  const;

		///Returns position + radius
		Coord2df GetCenter() const;
	
        inline float GetRadius()           const    {return r;  }
        inline void  SetRadius(const float& radius) {r=radius;  }

		CollisionResponse Contains(const Coord2df& point);
		CollisionResponse Contains(const CRect& r);
		CollisionResponse Contains(const Circle& r);

		///Radius
		float r;
};

CollisionResponse CollisionRectRect(const CRect& RR, const CRect& R);
CollisionResponse CollisionRectCircle(const CRect& R, const Circle C);
CollisionResponse CollisionRectPoint(const CRect& R, const Coord2df& point);
CollisionResponse CollisionCircleCircle(const Circle& CC, const Circle& C);
CollisionResponse CollisionCirclePoint(const Circle& CC, const Coord2df& point);

#endif
