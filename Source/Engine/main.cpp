#include "CommandLineArgs.h"
#include "GameStates/GameRunning.h"
#include "Kernel.h"

int main(int argc, char *argv[]) {
  //	Kernel* k=Kernel::Instance();
  //	k->Inst(argc, argv);

  Kernel::Inst(argc, argv);

  try {
    while (Kernel::Run()) {
    }
  } catch (ExitException e) {
    std::cout << e.what();
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
 * All Lua scripts must support 3 requirements<br>
 *  1. The Script MUST return a Function<br>
 *  2. The Function Returned Must Accept a Table as a parameter (even if it
 * doesn't use the table)<br>
 *  3. The Function Returned Must Return a Table<br>
 *
 * These requirements are in place so that scripts can be compositioned
 * together and generate a table
 * with functionality build from smaller parts
 *
 * \subsection Prefabs Prefabs:
 * Prefabs define what scripts are run and in what order they are run.
 * Different prefabs can be set for object, tile, and image layers (as well as
 * individual objects).
 *
 * \subsection LuaInterface Lua Interface:
 * 		Any Engine function in lua can be accessed by
 * CPP.interface:<funcName> <br>
 * 		LuaInterface functions can be found at LuaInterface::ExposeCPP
 * <br>
 * 		if the engine returns a C++ type, you can reference the Expose
 * Interface function to see what functions that data type has exposed
 *
 * \subsection Tiled Tiled:
 * Tiled is used as the Engine's map editor.<br>
 * Tiled Properties beginning with and underscore '_' are reserved by the
 * engine.<br>
 * For a list of properties that the engine looks for in different
 * tile layers, see tiledProperties
 * 		\see LuaInterface::ExposeCPP()
 * 		\see tiledProperties
 */
