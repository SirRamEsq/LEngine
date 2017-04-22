--DEBUG
package.path = package.path .. ";/opt/zbstudio/lualibs/mobdebug/?.lua"
--require('mobdebug').start()
  
--C++ Interfacing
  CPPInterface=nil;
    
    
function Initialize()
	--------------------
	--LUA Verion Check--
	--------------------
  
	if (_VERSION == "Lua 5.1") then
		LUA51=true;
	else
		LUA51=false;
	end
	-----------------------
	--C++ Interface setup--
	-----------------------
	CPPInterface=LEngine.CPPInterface;
  
  CPPInterface:WriteError("SUCCESS");
end

function Update()

end


function OnLuaEvent(senderEID, eventString)
  
end