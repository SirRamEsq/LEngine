#include "Input.h"
#include "gui/imgui_LEngine.h"
#include <algorithm>

const std::string InputManager::defaultKeyMappingIniFileName = "keyini.txt";

InputManager::InputManager()
    : mouseX(0),
      mouseY(0),
      mouseWheel(0),
      mouseButtonLeft(false),
      mouseButtonRight(0),
      mouseButtonMiddle(0) {
  ReadKeyIniFile();
  eventDispatcher = NULL;
  ImGuiIO &io = ImGui::GetIO();
  io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;  // Keyboard mapping. ImGui will use those
                                       // indices to peek into the io.KeyDown[]
                                       // array.
  io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
  io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
  io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
  io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
  io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
  io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
  io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
  io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
}

void InputManager::ReadKeyIniFile() {
  sdlKeyToInput.clear();
  if (keyMappingIni.OpenReadFile("keyini.txt")) {
    keyMappingIni.ReadFileToMap();

    for (auto i = keyMappingIni.readItBeg(); i != keyMappingIni.readItEnd();
         i++) {
      std::string value = i->second;

      // Get KeySym value
      int key = StringToNumber<int>(i->first);

      // If ascii character was passed instead of KeySym value
      if (key == 0) {
        key = (int)i->first[0];
      }
      sdlKeyToInput[key] = value;
    }
  }
  keyMappingIni.CloseReadFile();
  keyMappingIni.CopyReadMapToWriteMap();
}

InputManager::~InputManager() {}

void InputManager::SimulateKeyPress(const std::string &keyName) {
  simulatedInput.push_back(std::pair<bool, std::string>(SIM_KEYDOWN, keyName));
}

void InputManager::SimulateKeyRelease(const std::string &keyName) {
  simulatedInput.push_back(std::pair<bool, std::string>(SIM_KEYUP, keyName));
}

std::shared_ptr<InputManager::KeyMapping> InputManager::SetEventDispatcher(
    EventDispatcher *e, std::shared_ptr<InputManager::KeyMapping> *mapping) {
  // will set key mapping to one specified, or generate a new one if none is
  // specified
  eventDispatcher = e;
  if (mapping == NULL) {
    keyListeners = std::shared_ptr<KeyMapping>(new KeyMapping());
  } else {
    keyListeners = *mapping;
  }

  // will return data structure to allow caller to directly set key listeners
  return keyListeners;
}

void InputManager::HandleInput() {
  ImGuiIO &io = ImGui::GetIO();
  bool imWantKeyboard = io.WantCaptureKeyboard;

  // Reset mousewheel every frame
  mouseWheel = 0.0f;
  if (remapKey != "") {
    WriteMapSetKeyToNextInput(remapKey);
    ReadKeyIniFile();
    remapKey = "";
  }

  // Process simulated Input first
  for (auto i = simulatedInput.begin(); i != simulatedInput.end(); i++) {
    if (std::get<0>(*i) == SIM_KEYUP) {
      KeyRelease(std::get<1>(*i));
    } else {
      KeyPress(std::get<1>(*i));
    }
  }
  simulatedInput.clear();

  if (eventDispatcher == NULL) {
    return;
  }
  SDL_Event event;

  KeyMapping::iterator keyMI;

  std::string keyStr;
  bool keyup;

  while (SDL_PollEvent(&event)) {
    if ((keyup = (event.type == SDL_KEYUP)) or (event.type == SDL_KEYDOWN)) {
      int keyValue = event.key.keysym.sym;
      auto keyi = sdlKeyToInput.find(keyValue);
      if (imWantKeyboard) {
        int key = event.key.keysym.sym & ~SDLK_SCANCODE_MASK;
        io.KeysDown[key] = (event.type == SDL_KEYDOWN);
        io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
        io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
        io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
        io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
		continue;
      }

      if (keyi != sdlKeyToInput.end()) {
        if (!keyup) {
          if (keyValue == SDLK_ESCAPE) {
            // SDL_StopTextInput();
          } else {
            KeyPress(keyi->second);
          }
        } else {
          KeyRelease(keyi->second);
        }
      }
    }

    else if (event.type == SDL_MOUSEWHEEL) {
      mouseWheel = event.wheel.y;
      io.MouseWheel = mouseWheel;
    }

    else if (event.type == SDL_TEXTINPUT) {
      io.AddInputCharactersUTF8(event.text.text);
      mTextInput = event.text.text;
    }

    else if (event.type == SDL_TEXTEDITING) {
      // io.AddInputCharactersUTF8(event.text.text);
    }
  }

  // MouseX and MouseY are set the mouse's x,y coordinates relative to the
  // origin of the SDLWindow
  auto mouseMask = SDL_GetMouseState(&mouseX, &mouseY);
  mouseButtonLeft = ((mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0);
  mouseButtonRight = ((mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0);
  mouseButtonMiddle = ((mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0);

  io.MouseDown[0] = GetMouseButtonLeft();
  io.MouseDown[1] = GetMouseButtonRight();
  io.MouseDown[2] = GetMouseButtonMiddle();

  io.MousePos = ImVec2(GetMousePosition());
}

std::string InputManager::GetTextInput() {
  auto temp = mTextInput;
  mTextInput.clear();
  return temp;
}

void InputManager::SendEvent(Event::MSG message, std::string keyName) {
  Event event(EID_SYSTEM, EID_STATEMAN, message, keyName);

  // Stateman is informed first of the event
  eventDispatcher->DispatchEvent(event);

  // send event to all the entities listening
  auto keyIterator = keyListeners->find(keyName);
  if (keyIterator == keyListeners->end()) {
    return;
  }

  auto listeners = &keyIterator->second;
  eventDispatcher->DispatchEvent(event, listeners);
}

void InputManager::KeyPress(const std::string &keyName) {
  SendEvent(Event::MSG::KEYDOWN, keyName);
}

void InputManager::KeyRelease(const std::string &keyName) {
  SendEvent(Event::MSG::KEYUP, keyName);
}

bool InputManager::WriteMapSetKeyToNextInput(const std::string &keyName) {
  if (keyName == "") {
    return false;
  }

  SDL_Keycode key = 0;
  // time in miliseconds
  auto timer = SDL_GetTicks();
  // Ten seconds
  Uint32 timeLimit = 10 * 1000;
  timer += timeLimit;

  SDL_Event event;
  bool breakOut = false;
  // keep while loops nested to avoid branching based on an uninitialized value
  while (timer > SDL_GetTicks()) {
    while (SDL_PollEvent(&event)) {
      if ((event.type == SDL_KEYDOWN)) {
        key = event.key.keysym.sym;
        breakOut = true;
        break;
      }
      /// \TODO add a check for a joypad button/axis/hat/etc press
    }

    if (breakOut) {
      break;
    }
  }
  if (key == 0) {
    return false;
  }
  std::stringstream ss;
  ss << key;

  keyMappingIni.WriteString(ss.str(), keyName);
  OverwriteKeyIni();
  return true;
}

void InputManager::OverwriteKeyIni() {
  keyMappingIni.OpenWriteFile("keyini.txt");
  keyMappingIni.WriteMapToFile();
  keyMappingIni.CloseWriteFile();
}

Vec2 InputManager::GetMousePosition() { return Vec2(mouseX, mouseY); }

bool InputManager::GetMouseButtonLeft() { return mouseButtonLeft; }

bool InputManager::GetMouseButtonRight() { return mouseButtonRight; }

bool InputManager::GetMouseButtonMiddle() { return mouseButtonMiddle; }

float InputManager::GetMouseWheel() { return mouseWheel; }

void InputManager::RemapKey(const std::string keyName) { remapKey = keyName; }
