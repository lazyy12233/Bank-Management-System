@echo off
title Bank Management System
cd /d "C:\Users\HP\Desktop\Bank Management"
set "PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%"
echo ======================================
echo       BANK MANAGEMENT SYSTEM
echo ======================================
echo.

if not exist "backend\main.cpp" (
    echo ERROR: backend\main.cpp was not found.
    pause
    exit /b 1
)

echo Compiling Bank Management System...
"C:\msys64\ucrt64\bin\g++.exe" backend/main.cpp -o BankSystem.exe -std=c++17 -pthread -lws2_32

if errorlevel 1 (
    echo.
    echo Compilation failed. Please check the errors above.
    pause
    exit /b 1
)

echo.
echo Compilation successful!
echo Starting bank server...
echo.
echo Open your browser at http://localhost:8081
echo Keep this window open while using the website.
echo.
BankSystem.exe

echo.
echo Bank server has stopped.
pause
