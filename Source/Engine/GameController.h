#ifndef L_ENGINE_CONTROLLER
#define L_ENGINE_CONTROLLER

#include "SDLInit.h"

#include <functional>
#include <unordered_map>

class GameController {
 public:
  typedef std::function<void()> HatCallback;
  // Parameter for distance from center
  typedef std::function<void(int)> AxisCallback;
  typedef std::function<void()> ButtonCallback;

  GameController(SDL_GameController* pad);
  ~GameController();

  void OnAxisEvent(int axisIndex, AxisCallback cb);
  void OnButtonPressEvent(int button, ButtonCallback cb);
  void OnButtonReleaseEvent(int button, ButtonCallback cb);
  void OnHatEvent(int hatIndex, int hatDir, HatCallback cb);

  void AxisEvent(int axis, int distance);
  void ButtonPressEvent(int button);
  void ButtonReleaseEvent(int button);
  void HatEvent(int hat, int hatDir);

 private:
  std::unordered_map<int, AxisCallback> mAxisCallbacks;
  std::unordered_map<int, ButtonCallback> mButtonPressCallbacks;
  std::unordered_map<int, ButtonCallback> mButtonReleaseCallbacks;
  std::unordered_map<int, std::unordered_map<int, HatCallback> > mHatCallbacks;

  SDL_GameController* mSDLpad;
};

#endif

