set BUILD=build
if "%1" == "rebuild" (set BUILD=rebuild)

devenv /%BUILD% release ..\visualc\ttermpro.sln
devenv /%BUILD% release ..\..\ttssh2\ttssh.sln
devenv /%BUILD% release ..\..\TTProxy\TTProxy.sln
