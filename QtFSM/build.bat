@echo off
echo Building QtFSM...

REM converting current path to WSL path
REM This assumes the script is run from the project root
wsl bash -c "cd \"$(wslpath '%CD%')\" && mkdir -p build && cd build && cmake .. && make -j$(nproc)"

if %errorlevel% neq 0 (
    echo.
    echo -----------------------------------
    echo Build FAILED! 
    echo -----------------------------------
    pause
    exit /b %errorlevel%
)

echo.
echo -----------------------------------
echo Build SUCCESSFUL!
echo Binary is at: build/QtFSM
echo -----------------------------------
pause
