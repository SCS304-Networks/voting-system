@echo off
echo --- SONU VOTING SYSTEM BUILDER ---
echo Cleaning old files...
del *.o sonu_vote.exe 2>nul

echo Compiling Full Concurrent System...
gcc main.c interface.c database_mgr.c voting_logic.c tally_report.c utils.c -o sonu_vote.exe

if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed! Check your GCC installation.
    pause
    exit /b
)

echo [SUCCESS] System compiled. Launching...
sonu_vote.exe
pause