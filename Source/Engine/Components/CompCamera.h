#ifndef L_ENGINE_COMP_CAMERA
#define L_ENGINE_COMP_CAMERA

#include "../BaseComponentManager.h"
#include <set>
#include "../RenderManager.h"
#include "CompPosition.h"

class ComponentCameraManager;
class ComponentCamera : public BaseComponent{
    public:
        ComponentCamera(EID id, ComponentPosition* pos, RenderManager* rm, ComponentCameraManager* manager);
        ~ComponentCamera();

        void Update();
        void HandleEvent(const Event* event);
		///For LuaInterface
		void SetViewport(CRect viewport);

        CRect GetViewport();

    private:
        RenderCamera mCamera;
        ComponentPosition* mPosition;
};


class ComponentCameraManager : public BaseComponentManager_Impl<ComponentCamera>{
    public:
        ComponentCameraManager(EventDispatcher* e);
        ~ComponentCameraManager();

		std::unique_ptr<ComponentCamera> ConstructComponent	(EID id, ComponentCamera* parent);
		void SetDependencies(ComponentPositionManager* pos, RenderManager* rm);

		ComponentPositionManager* dependencyPosition;
		RenderManager* dependencyRenderManager;
};

#endif // L_ENGINE_COMP_CAMERA
