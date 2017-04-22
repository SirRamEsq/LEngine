local Timing = {
  _VERSION      = "v1",
  _DESCRIPTION  = "Module for keeping track of time"
}

Timing.alarms={}
Timing.timers={}

Timing.Alarm={}
--[[Timing.Alarm.__tostring= function (self)
  local string= "[Timing.Alarm]";
  string= string .. "\n        Current Time is " .. tostring(self.properties.currentTime) .. " with a max of " .. tostring(self.properties.maxTime);
  string= string .. "\n        function address is " .. tostring(self.properties.callBackFunction);
  string= string .. "\n        Alarm Repeat is set to " .. tostring(self.properties.repeatAlarm);
  string= string .. "\n        Alarm activation is set to " .. tostring(self.properties.activated);

  return string;
end]]--
Timing.Alarm.constructor= function (self, max, callback, repeatA)
  local newAlarm={};

  newAlarm.properties={
    maxTime=max or 100,
    currentTime=0,
    callBackFunction=callback or nil,
    repeatAlarm=repeatA or false,
    activated=true
  }
  newAlarm.constructor          = Timing.Alarm.constructor;
  newAlarm.SetCallBackFunction  = Timing.Alarm.SetCallBackFunction
  newAlarm.Restart              = Timing.Alarm.Restart;
  newAlarm.Update               = Timing.Alarm.Update;
  newAlarm.SetMaxTime           = Timing.Alarm.SetMaxTime;
  newAlarm.Disable              = Timing.Alarm.Disable;

  setmetatable(newAlarm, Timing.Alarm);

  return newAlarm;
end
setmetatable(Timing.Alarm,  {__call = Timing.Alarm.constructor, __tostring = Timing.Alarm.__tostring})

Timing.Alarm.SetMaxTime= function(self, time)
  self.properties.maxTime=time;
end

Timing.Alarm.SetCallBackFunction = function(self, func)
  self.properties.callBackFunction=func;
end

Timing.Alarm.Restart= function(self)
  self.properties.currentTime=0;
  self.properties.activated=true;
end

Timing.Alarm.Disable= function(self)
  self.properties.activated=false;
end

Timing.Alarm.Update= function(self)
  if(not self.properties.activated)then return end

  self.properties.currentTime= self.properties.currentTime + 1;

  if(self.properties.currentTime >= self.properties.maxTime)then
    self:Restart();
    if(not self.properties.repeatAlarm)then
      self.properties.activated=false;
    end

    self.properties.callBackFunction(self);
  end
end



Timing.Timer={}--[[
Timing.Timer.__tostring= function (self)
  local string= "[Timing.Timer]";
  string= string .. "\n        Current Time is " .. tostring(self.properties.currentTime);

  return string;
end--]]
Timing.Timer.constructor= function (self)
  local newTimer={};

  newTimer.properties={
    currentTime=0,
    timeFunctionMap=nil
  }
  newTimer.constructor        = Timing.Timer.constructor;
  setmetatable(newTimer, Timing.Timer);

  return newTimer;
end
setmetatable(Timing.Timer,  {__call = Timing.Timer.constructor, __tostring = Timing.Timer.__tostring})

Timing.Timer.Update= function(self)
  self.properties.currentTime= self.properties.currentTime + 1;
  local currentTime=self.properties.currentTime;

  for k,v in pairs(self.properties.timeFunctionMap) do
    if(k==currentTime)then v(); end
  end

end

Timing.Timer.GetTime= function(self)
  return self.properties.currentTime;
end

Timing.Timer.Reset= function(self)
  self.properties.currentTime=0;
end

Timing.Timer.AddFunction= function(self, time, func)
  self.properties.timeFunctionMap[time]=func;
end


--Set max time and start alarm
Timing.SetAlarm = function (self, alarm, time, callback, repeatA)
  --Sets max time to either the function argument or what it was previously
  local alarmKey= alarm or 0;
  local selectedAlarm=self.alarms[alarmKey];
  if(selectedAlarm==nil)then
    self.alarms[alarmKey]=Timing.Alarm(time, callback, repeatA);
    selectedAlarm=self.alarms[alarmKey];
  end
  return selectedAlarm;
end

Timing.GetAlarm = function (self, alarm)
  return self.alarms[alarm];
end

Timing.SetTimer = function (self, timer)
  --Sets max time to either the function argument or what it was previously
  local timerKey= timer or 0;
  local selectedTimer=self.timers[timerKey];
  if(selectedTimer==nil)then
    self.timers[timerKey]=Timing.Timer(time, callback, repeatA);
    selectedTimer=self.timers[timerKey];
  end
  return selectedAlarm;
end

Timing.GetTimer = function (self, timer)
  return self.timers[timer]
end

Timing.Update = function (self)
  for k,v in pairs(self.alarms) do
    v:Update();
  end
  for k,v in pairs(self.timers) do
    v:Update();
  end
end


--return container
return Timing
