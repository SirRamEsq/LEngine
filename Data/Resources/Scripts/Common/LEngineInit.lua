--contains all the data that Lua scripts need from cpp
local NewLEngine = function ()
  --Initialize Container
  local LEngine = {
    _VERSION      = "v0.9",
    _DESCRIPTION  = "Module that the LEngine populates with data and sends to each instance of a script component",

    depth         = 0,
    parent        = nil,
    entityID      = 0,
    name          = "",
    objType       = "",

    InitializationTable = {} --This table holds all the data that is set at instantiation
  }

  --This function is run before anything else is run in the script
  LEngine.Initialize= function (id, name, objType, depth, parent)
    LEngine.parent     =parent;
    LEngine.depth      =depth;
    LEngine.entityID   =id;
    LEngine.objType    =objType;
    LEngine.name       =name;
  end

  return LEngine
end

return NewLEngine;
