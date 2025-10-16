@echo off
REM Args:
REM %1 = TargetDir     (Math's output folder)
REM %2 = Configuration (Debug/Release)
REM %3 = Platform      (x64/Win32)
REM %4 = SolutionDir   (optional; inferred if omitted)

REM --- Normalize TargetDir (strip trailing backslash)
set "TD=%~1"
if "%TD:~-1%"=="\" set "TD=%TD:~0,-1%"

REM --- Resolve SolutionDir
set "SLDIR=%~4"
if "%SLDIR%"=="" (
    for %%I in ("%~dp0..") do set "SLDIR=%%~fI\"
)

echo ============================================
echo [Math DLL Copy]
echo Config     = %~2
echo Platform   = %~3
echo SolutionDir= %SLDIR%
echo TargetDir  = %TD%
echo ============================================

REM Only depend on PixelFoxCore (DLLs only)
call :CopyDLL "PixelFoxCore" "%SLDIR%PixelFoxCore\bin\%~3\%~2" "%SLDIR%PixelFoxCore\bin\%~2\%~3" "%TD%"

echo.
echo [Done] PixelFoxCore DLL(s) copied to PixelFoxMath bin.
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
