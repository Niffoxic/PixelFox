@echo off
REM Args:
REM %1 = TargetDir     (where to place DLLs)
REM %2 = Configuration (Debug/Release)
REM %3 = Platform      (x64/Win32)
REM %4 = SolutionDir   (optional; if omitted we infer it)

REM --- Normalize TargetDir (trim trailing backslash so tools don't choke)
set "TD=%~1"
if "%TD:~-1%"=="\" set "TD=%TD:~0,-1%"

REM --- Find SolutionDir
set "SLDIR=%~4"
if "%SLDIR%"=="" (
    REM BAT is in PixelFoxEngine project folder; parent is solution root
    for %%I in ("%~dp0..") do set "SLDIR=%%~fI\"
)

echo ============================================
echo [Engine DLL Copy]
echo Config     = %~2
echo Platform   = %~3
echo SolutionDir= %SLDIR%
echo TargetDir  = %TD%
echo ============================================

REM Only these modules; no LIBs, only DLLs
set MODULES=PixelFoxCore PixelFoxPhysics PixelFoxMath

for %%M in (%MODULES%) do (
    echo.
    echo -- %%M
    call :CopyDLL "%%M" "%SLDIR%%%M\bin\%~3\%~2" "%SLDIR%%%M\bin\%~2\%~3" "%TD%"
)

echo.
echo [Done] PixelFoxEngine dependency DLLs copied.
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
