@echo off
REM Args:
REM %1 = TargetDir     (tests output)
REM %2 = Configuration (Debug/Release)
REM %3 = Platform      (x64/Win32)
REM %4 = SolutionDir   (optional; inferred if missing)
REM %5 = ProjectDir    (where libs should go)

REM --- Normalize TargetDir / ProjectDir (strip trailing '\')
set "TD=%~1"
if "%TD:~-1%"=="\" set "TD=%TD:~0,-1%"
set "PD=%~5"
if "%PD:~-1%"=="\" set "PD=%PD:~0,-1%"

REM --- Resolve SolutionDir
set "SLDIR=%~4"
if "%SLDIR%"=="" (
    for %%I in ("%~dp0..") do set "SLDIR=%%~fI\"
)

echo ============================================
echo [Tests Dependency Copy]
echo Config     = %~2
echo Platform   = %~3
echo SolutionDir= %SLDIR%
echo TargetDir  = %TD%
echo ProjectDir = %PD%
echo ============================================

REM Copy from these modules
set MODULES=PixelFoxEngine PixelFoxCore PixelFoxPhysics PixelFoxMath

for %%M in (%MODULES%) do (
    echo.
    echo -- %%M
    call :CopyType "%%M" "dll" "%SLDIR%%%M\bin\%~3\%~2" "%SLDIR%%%M\bin\%~2\%~3" "%TD%"
    call :CopyType "%%M" "lib" "%SLDIR%%%M\bin\%~3\%~2" "%SLDIR%%%M\bin\%~2\%~3" "%PD%"
)

echo.
echo [Done] All test dependencies copied.
exit /b 0

:CopyType
REM %1=name  %2=ext  %3=cand1(bin\<plat>\<cfg>)  %4=cand2(bin\<cfg>\<plat>)  %5=dest
echo    [find] %~3\*.%~2
echo    [find] %~4\*.%~2
if exist "%~3\*.%~2" (
    if not exist "%~5" mkdir "%~5" >nul 2>&1
    echo    [copy] "%~3\*.%~2" -> "%~5"
    xcopy /Y /Q "%~3\*.%~2" "%~5" >nul
    echo    [ok]
    goto :eof
)
if exist "%~4\*.%~2" (
    if not exist "%~5" mkdir "%~5" >nul 2>&1
    echo    [copy] "%~4\*.%~2" -> "%~5"
    xcopy /Y /Q "%~4\*.%~2" "%~5" >nul
    echo    [ok]
    goto :eof
)
echo    [warn] none found for %~1 (.%~2)
goto :eof
