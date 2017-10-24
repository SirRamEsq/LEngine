#include "CommandLineArgs.h"
#include "GameStates/GameRunning.h"
#include "Kernel.h"

int main(int argc, char *argv[]) {
  //	Kernel* k=Kernel::Instance();
  //	k->Inst(argc, argv);

  Kernel::Inst(argc, argv);

  while (Kernel::Run()) {
  }
  Kernel::Close();

  return 0;
}

/*! \mainpage LEngine
 *
 * \section intro_sec Introduction
 *
 * 2D Game Engine!
 *
 * \section script_documentation Scripting Documentation
 *
 * \subsection LuaInterface Lua Interface
 * 		Any Engine function in lua can be accessed by
 * CPP.interface:<funcName> <br>
 * 		LuaInterface functions can be found at LuaInterface::ExposeCPP
 * <br>
 * 		if the engine returns a C++ type, you can reference the Expose
 * Interface function to see what functions that data type has exposed
 * 		\see LuaInterface::ExposeCPP()
 */
