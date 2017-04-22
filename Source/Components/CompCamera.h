#ifndef L_ENGINE_COMP_CAMERA
#define L_ENGINE_COMP_CAMERA

#include "../BaseComponent.h"
#include <set>
#include "../RenderManager.h"
#include "CompPosition.h"

class ComponentCamera : public BaseComponent{
    public:
        ComponentCamera(EID id, ComponentPosition* pos, const std::string& logFile);
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
        ComponentCameraManager();
        ~ComponentCameraManager();

        void Update();
        void AddComponent(EID id);
        void HandleEvent(const Event* event){}
};

#endif // L_ENGINE_COMP_CAMERA
