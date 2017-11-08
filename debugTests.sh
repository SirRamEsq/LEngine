#!/bin/bash
./makeRun_TEST.shn
gdb --args ./bin/LEngine_TEST -- --luaTestDir "Data/Resources/Scripts/Testing/Integration"
