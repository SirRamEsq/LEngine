#ifndef L_ENGINE_RENDERABLE_OBJECT
#define L_ENGINE_RENDERABLE_OBJECT

#include "../Defines.h"
#include "../Resources/LglShader.h"

#include <string>

class RenderManager;
class RenderableObjectScreen;
class RenderableObjectWorld;

class RenderableObject{
	friend RenderManager;
	friend RenderableObjectScreen;
	friend RenderableObjectWorld;

	public:
		enum TYPE{
			Generic		= 0,
			Screen		= 1,
			World		= 2,
			SpriteBatch = 3,
			TileLayer	= 4,
			Image		= 5
		};

		RenderableObject(TYPE t = TYPE::Generic);

		//Virtual destructor; enables derived classes to be fully deleted from a base RenderableObject pointer
		virtual ~RenderableObject(){}

		//All renderable objects will render using their own shader program if one is not passed to them
		virtual void Render(L_GL_Program* program) = 0;
				void Render(					 ) {Render(shaderProgram);}

		void		SetDepth(const MAP_DEPTH& d);
		MAP_DEPTH	GetDepth();

		void		SetRotation(const double& rot)	{rotation=rot;}
		double		GetRotation()					{return rotation;}

		void		SetScalingY(const double& s)	{scaleY=s;}
		double		GetScalingY()					{return scaleY;}

		void		SetScalingX(const double& s)	{scaleX=s;}
		double		GetScalingX()					{return scaleX;}

		void		SetPositionX(const double& x)	{position.x=x;}
		double		GetPositionX()					{return position.x;}

		void		SetPositionY(const double& y)	{position.y=y;}
		double		GetPositionY()					{return position.y;}

		void		SetRender(const bool& s)		{render=s;}
		bool		GetRender()						{return render;}

		L_COLOR		GetColor()						{return color;}
		void		SetColor(L_COLOR c)				{color=c;}

		void		   SetShaderProgram(L_GL_Program* p){shaderProgram=p;}
		L_GL_Program*  GetShaderProgram()				{return shaderProgram;}

		//void UpdateShaderProgram();

		bool		render;
		L_COLOR		color;

		TYPE type;

	protected:
		virtual void AddToRenderManager() = 0;

	private:
		MAP_DEPTH	depth;
		double		scaleX, scaleY, rotation;
		Coord2df	position;

		L_GL_Program* shaderProgram;
};

class RenderableObjectScreen : public RenderableObject{
	public:
		RenderableObjectScreen(TYPE t = TYPE::Screen);
		virtual ~RenderableObjectScreen();

		virtual void AddToRenderManager();
};

class RenderableObjectWorld : public RenderableObject{
	public:
		RenderableObjectWorld(TYPE t = TYPE::World);
		virtual ~RenderableObjectWorld();

		virtual void AddToRenderManager();
};

#endif // L_ENGINE_RENDERABLE_OBJECT