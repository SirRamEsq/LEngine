#ifndef L_ENGINE_COMP_CAMERA
#define L_ENGINE_COMP_CAMERA

#include "../../../Source/BaseComponent.h"
#include <set>

class ComponentCamera : public BaseComponent{
    public:
        ComponentCamera(EID id, const std::string& logFile);
        ~ComponentCamera();

        void Update();
        void HandleEvent(const Event* event);

        void SetAsMainCamera();

        const CRect& GetViewport();

    private:
        CRect mCamera;
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
