#ifndef L_RENDER_MANAGER
#define L_RENDER_MANAGER

#include <list>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include "Defines.h"
#include "SDLInit.h"
#include "glslHelper.h"

#include "Resources/RSC_GLShader.h"
#include "Rendering/RenderPrimitives.h"
#include "Rendering/RenderableObject.h"
#include "Rendering/RenderSpriteBatch.h"
#include "Rendering/RenderTileLayer.h"
#include "Rendering/RenderImageLayer.h"

/*
Centralized Rendering:
The rendermanger will know of all renderable objects (sprites, backgrounds, tiles, and text)
it will render them in order based on their depth
it will also control the camera
RenderableSprite automatically adds itself to the appropriate sprite batch when instantiated.

RenderManager owns only SpriteBatches, the rest are owned by whatever instantiates them
*/


class ComponentCamera;
class RenderCamera{
	friend class RenderManager;
	friend class ComponentCamera;

	public:
		RenderCamera(RenderManager* rm);
		~RenderCamera();

		void	SetScaling (double s)	{scale=s;		 }
		void	SetRotation(double r)	{rotation=r;	 }
		double	GetScaling ()			{return scale;	 }
		double	GetRotation()			{return rotation;}

		int			   GetX		  (){return view.x;}
		int			   GetY		  (){return view.y;}
		const CRect&   GetViewRect(){return view;  }

		void		  Bind			   (const GLuint& GlobalCameraUBO);

		void UpdatePosition();
		void RenderFrameBufferTextureFinal();

	protected:
		CRect view;

		RSC_Texture* GetFrameBufferTextureDiffuse(){return frameBufferTextureDiffuse.get();}
		RSC_Texture* GetFrameBufferTextureFinal(){return frameBufferTextureFinal.get();}

	private:
		Vec2 translation;

		//Cameras can have different shaders!!!

		double rotation;
		double scale;

		MAP_DEPTH nearClippingPlane;
		MAP_DEPTH farClippingPlane;

		double normalizedViewportX; //between 0.0f and 1.0f
		double normalizedViewportY; //determines where the FBO will be rendered to.

		GLuint FBO; //frame buffer object id

		bool mActive;

		std::unique_ptr<RSC_Texture>	 frameBufferTextureDiffuse;
		std::unique_ptr<RSC_Texture>	 frameBufferTextureFinal;

		RenderManager* dependencyRenderManager;
};

class RenderManager{
	friend RenderableObjectScreen;
	friend RenderableObjectWorld;
	friend RenderCamera;
	public:
		RenderManager();

		void Render();

		void OrderOBJs();

		void AssignCameraUBO(RSC_GLProgram* program);

		 //returns sprite batch that supports 'textureName' and has room for at least
		 //numSprites more room
		RenderSpriteBatch* GetSpriteBatch(const std::string& textureName, const MAP_DEPTH& depth, const int& numSprites);
		MAP_DEPTH nextTextID;

	protected:
		bool AddCamera(RenderCamera* cam);
		void RemoveCamera(RenderCamera* cam);

		void AddObjectScreen	(RenderableObjectScreen* obj);
		void RemoveObjectScreen (RenderableObjectScreen* obj);

		void AddObjectWorld		(RenderableObjectWorld* obj);
		void RemoveObjectWorld	(RenderableObjectWorld* obj);

	private:
		bool								listChange;
		std::list<RenderableObjectWorld*>	objectsWorld;
		std::list<RenderableObjectScreen*>	objectsScreen;
		std::set<RenderCamera*>				mCameras;

		std::string shaderFragmentNameSpriteBatch;
		std::string shaderVertexNameSpriteBatch;
		RSC_GLProgram shaderProgramSpriteBatch;

		std::string shaderFragmentNameTileLayer;
		std::string shaderVertexNameTileLayer;
		RSC_GLProgram shaderProgramTileLayer;

		std::string shaderFragmentNameImage;
		std::string shaderVertexNameImage;
		RSC_GLProgram shaderProgramImage;

		std::string shaderFragmentNameLight;
		std::string shaderVertexNameLight;
		RSC_GLProgram shaderProgramLight;

		static GLuint GlobalCameraUBO;
		const GLuint CameraDataBindingIndex;

		static GLuint GlobalProgramUBO;
		const GLuint ProgramDataBindingIndex;

		int spriteBatchMaxSize;
		std::map<MAP_DEPTH, std::map< std::string, std::vector < std::unique_ptr <RenderSpriteBatch> > > >	spriteBatchMap; //map each sprite batch to a texture name and depth value

		//Time since creation of RenderManager
		unsigned int timeElapsed=0;
};

#endif
