@echo off
REM Args:
REM %1 = TargetDir     (Physics output folder)
REM %2 = Configuration (Debug/Release)
REM %3 = Platform      (x64/Win32)
REM %4 = SolutionDir

REM --- Normalize TargetDir
set "TD=%~1"
if "%TD:~-1%"=="\" set "TD=%TD:~0,-1%"

REM --- Resolve SolutionDir (auto if not passed)
set "SLDIR=%~4"
if "%SLDIR%"=="" (
    for %%I in ("%~dp0..") do set "SLDIR=%%~fI\"
)

echo ============================================
echo [Physics DLL Copy]
echo Config     = %~2
echo Platform   = %~3
echo SolutionDir= %SLDIR%
echo TargetDir  = %TD%
echo ============================================

REM --- Modules PixelFoxPhysics depends on ---
set MODULES=PixelFoxCore PixelFoxMath

for %%M in (%MODULES%) do (
    echo.
    echo -- %%M
    call :CopyDLL "%%M" "%SLDIR%%%M\bin\%~3\%~2" "%SLDIR%%%M\bin\%~2\%~3" "%TD%"
)

echo.
echo [Done] Copied dependent DLLs to PixelFoxPhysics bin.
exit /b 0


:CopyDLL
REM %1=name  %2=cand1(bin\<plat>\<cfg>)  %3=cand2(bin\<cfg>\<plat>)  %4=dest
echo    [find] %~2
echo    [find] %~3

if exist "%~2\*.dll" (
    if not exist "%~4" mkdir "%~4" >nul 2>&1
    echo    [copy] "%~2\*.dll" -> "%~4"
    xcopy /Y /Q "%~2\*.dll" "%~4" >nul
    echo    [ok]
    goto :eof
)

if exist "%~3\*.dll" (
    if not exist "%~4" mkdir "%~4" >nul 2>&1
    echo    [copy] "%~3\*.dll" -> "%~4"
    xcopy /Y /Q "%~3\*.dll" "%~4" >nul
    echo    [ok]
    goto :eof
)

echo    [warn] no DLLs found for %~1
goto :eof
