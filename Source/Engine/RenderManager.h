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

#include "gui/imgui.h"

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
		RenderCamera(RenderManager* rm, CRect viewPort);
		~RenderCamera();

		void	SetScaling (float s);
		void	SetRotation(float r);
		float	GetScaling () const;
		float	GetRotation() const;

		void  Bind (const GLuint& GlobalCameraUBO);

		CRect GetView()const;
		void SetView(CRect viewPort);

		void RenderFrameBufferTextureFinal();

	protected:
		///Resolution to display
		CRect view;

		RSC_Texture* GetFrameBufferTextureDiffuse(){return frameBufferTextureDiffuse.get();}
		RSC_Texture* GetFrameBufferTextureFinal(){return frameBufferTextureFinal.get();}

	private:
		float rotation;
		float scale;

		MAP_DEPTH nearClippingPlane;
		MAP_DEPTH farClippingPlane;

		double normalizedViewportX; //between 0.0f and 1.0f
		double normalizedViewportY; //determines where the FBO will be rendered to.
		///Coordinates on the screen that the image will be rendered to
		FloatRect normalizedViewport;

		///frame buffer object id
		GLuint FBO; 

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
		/**
		 * Will add camera to list of cameras to be rendered
		 * is meant to be called by a newly created camera
		 */
		bool AddCamera(RenderCamera* cam);
		/**
		 * Will remove camera from list of cameras to be rendered
		 * is meant to be called by a camera destructor
		 */
		void RemoveCamera(RenderCamera* cam);

		/**
		 * Will add object to list of object to be rendered
		 * is meant to be called by a newly created object to screen coordinates
		 */
		void AddObjectScreen	(RenderableObjectScreen* obj);
		void RemoveObjectScreen (RenderableObjectScreen* obj);

		/**
		 * Will add object to list of object to be rendered
		 * is meant to be called by a newly created object to World coordinates
		 */
		void AddObjectWorld		(RenderableObjectWorld* obj);
		void RemoveObjectWorld	(RenderableObjectWorld* obj);

	private:
		std::unique_ptr<RSC_GLProgram> LoadShaderProgram(const std::string& vertexName, const std::string& fragName);
		void LinkShaderProgram(RSC_GLProgram* program);

		/**
		 * Will Load default shaders for Tile, Light Sprite, Image if not already loaded
		 * will assign default shaders to member data
		 */
		void LoadDefaultShaders();

		void ImGuiRender(ImDrawData* drawData);

		bool								listChange;
		/// \TODO decide if these should be lists or sets
		std::list<RenderableObjectWorld*>	objectsWorld;
		std::list<RenderableObjectScreen*>	objectsScreen;
		std::set<RenderCamera*>				mCameras;

		static const std::string defaultProgramTileName;
		static const std::string defaultProgramSpriteName;
		static const std::string defaultProgramLightName;
		static const std::string defaultProgramImageName;

		const RSC_GLProgram* defaultProgramTile;
		const RSC_GLProgram* defaultProgramSprite;
		const RSC_GLProgram* defaultProgramLight;
		const RSC_GLProgram* defaultProgramImage;

		static GLuint GlobalCameraUBO;
		static const GLuint CameraDataBindingIndex;

		static GLuint GlobalProgramUBO;
		static const GLuint ProgramDataBindingIndex;

		int spriteBatchMaxSize;
		std::map<MAP_DEPTH, std::map< std::string, std::vector < std::unique_ptr <RenderSpriteBatch> > > >	spriteBatchMap; //map each sprite batch to a texture name and depth value

		//Time since creation of RenderManager
		unsigned int timeElapsed=0;
};

#endif
