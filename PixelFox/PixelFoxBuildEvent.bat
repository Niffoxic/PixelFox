@echo off
rem %1=TargetDir %2=Config %3=Platform %4=ProjectDir %5=SolutionDir

echo ==== PixelFox copy (Debug) ====
echo SolutionDir=%~5
echo TargetDir=%~1
echo ProjectDir=%~4
echo Config=%~2
echo Platform=%~3
echo ===============================

set MODULES=PixelFoxCore PixelFoxEngine PixelFoxMath PixelFoxPhysics

for %%M in (%MODULES%) do (
  echo.
  echo -- %%M
  call :CopyType "%%M" "dll" "%~5%%M\bin\%~3\%~2" "%~5%%M\bin\%~2\%~3" "%~1"
  call :CopyType "%%M" "lib" "%~5%%M\bin\%~3\%~2" "%~5%%M\bin\%~2\%~3" "%~4"
)

exit /b 0

:CopyType
echo    [DEBUG] looking for *.%~2 in:
echo           %~3
echo           %~4
if exist "%~3\*.%~2" (
  echo    [COPY] %~1 *.%~2 from "%~3" -> "%~5"
  if not exist "%~5" mkdir "%~5" >nul 2>&1
  xcopy /Y /Q "%~3\*.%~2" "%~5" >nul
  echo    [OK]
  goto :eof
)
if exist "%~4\*.%~2" (
  echo    [COPY] %~1 *.%~2 from "%~4" -> "%~5"
  if not exist "%~5" mkdir "%~5" >nul 2>&1
  xcopy /Y /Q "%~4\*.%~2" "%~5" >nul
  echo    [OK]
  goto :eof
)
echo    [WARN] none found for %~1 (.%~2)
goto :eof
