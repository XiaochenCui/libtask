@echo off
if EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools" (
    %comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) else if EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community" (
    %comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsx86_amd64.bat"
)
