local container = {}
--contains all the data that Lua scripts need from cpp
container.NewLEngine = function ()
  --Initialize Container
  local LEngine = {
    _VERSION      = "v1.0",
    _DESCRIPTION  = "Module that the LEngine populates with data and sends to each instance of a script component",

	debugMode	  = false,
    depth         = 0,
    parent        = nil,
    entityID      = 0,
    name          = "",
    objType       = "",

    InitializationTable = {} --This table holds all the data that is set at instantiation
  }

  --This function is run before anything else is run in the script
  LEngine.Initialize= function (id, name, objType, depth, parent, debug)
    LEngine.parent     =parent;
    LEngine.depth      =depth;
    LEngine.entityID   =id;
    LEngine.objType    =objType;
    LEngine.name       =name;
	LEngine.debugMode 	= debug or false;

	if (CPP.interface == nil) then error("LEngine::Initialize - CPP.interface is nil") end
  end

  return LEngine
end

return container.NewLEngine;
