set BUILD=build
if "%1" == "rebuild" (set BUILD=rebuild)

devenv /%BUILD% release ..\visualc\ttermpro.sln
devenv /%BUILD% release ..\..\ttssh2\ttssh.sln
devenv /%BUILD% release ..\..\TTProxy\TTProxy.sln

if "%1"=="plugins" devenv /%BUILD% release ..\visualc\TTXSamples.sln

