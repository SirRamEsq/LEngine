#ifndef L_ENGINE_COMP_CAMERA
#define L_ENGINE_COMP_CAMERA

#include "../BaseComponent.h"
#include <set>
#include "../RenderManager.h"
#include "CompPosition.h"

class ComponentCamera : public BaseComponent{
    public:
        ComponentCamera(EID id, ComponentPosition* pos, RenderManager* rm, const std::string& logFile);
        ~ComponentCamera();

        void Update();
        void HandleEvent(const Event* event);

        void SetAsMainCamera();

        const CRect& GetViewport();

    private:
        RenderCamera mCamera;
        ComponentPosition* mPosition;
};


class ComponentCameraManager : public BaseComponentManager{
    public:
        ComponentCameraManager(EventDispatcher* e);
        ~ComponentCameraManager();

        void Update();
        void AddComponent(EID id);
        void HandleEvent(const Event* event){}
		void SetDependencies(ComponentPositionManager* pos, RenderManager* rm);

		ComponentPositionManager* dependencyPosition;
		RenderManager* dependencyRenderManager;
};

#endif // L_ENGINE_COMP_CAMERA
