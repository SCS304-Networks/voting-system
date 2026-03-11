#!/bin/bash

echo "--- SONU VOTING SYSTEM BUILDER ---"
echo "Cleaning old files..."

# 'rm -f' is the Unix equivalent of 'del'. 
# The -f flag prevents errors if the files don't exist.
rm -f *.o sonu_vote

echo "Compiling Full Concurrent System..."

# Note: We remove .exe because Unix binaries usually don't have extensions.
gcc main.c interface.c database_mgr.c voting_logic.c tally_report.c utils.c -o sonu_vote

# $? checks the exit status of the previous command (gcc)
if [ $? -ne 0 ]; then
    echo "[ERROR] Compilation failed! Check your GCC installation."
    read -p "Press enter to exit..."
    exit 1
fi

echo "[SUCCESS] System compiled. Launching..."
./sonu_vote

# 'read' acts like 'pause' in Windows
read -p "Press enter to continue..."