# SONU Digital Voting System (Concurrent Version)
 ## Project Overview
A secure, high-performance digital voting system designed for the Student Organization of Nairobi University (SONU) elections. Built in C, the system features a custom concurrency layer and a modular architecture to handle high-traffic election cycles with absolute data integrity.

 ## System Architecture & Modules
1. Core Entry & Lifecycle (main.c)
Role: The system dispatcher and application lifecycle manager.

Key Logic: Implements a central event loop with input validation to prevent "garbage" input failures. It routes users between the Student, Admin, and Public interfaces.

2. Database Engine (database_mgr.c)
Role: High-speed binary file I/O abstraction.

Concurrency: Implements Pessimistic Locking to ensure thread-safety.

Optimization: Uses In-Place Updates (via fseek) to increment vote counts, avoiding the overhead of rewriting entire files.

3. Rule Engine (voting_logic.c)
Role: Enforces election laws and defensive programming.

Security: Implements "One Person, One Vote" gates and input sanitization to prevent buffer overflows.

Integrity: Automatic ID generation using lock-aware logic to prevent duplicate Primary Keys.

4. Finalization & Reporting (tally_report.c)
Role: Data transformation and election finalization.

The "Safety Pin": Creates an election.lock file that renders the database immutable once the election is closed.

Output: Performs a Linear Search & Compare algorithm to determine winners and generates a portable final_results.txt audit trail.

5. Command Line Interface (interface.c)
Role: UX management and input sanitization.

Features: Dynamic menus that change state based on election status and protected administrative dashboards.

6. Concurrency Infrastructure (utils.c)
Role: Cross-platform synchronization.

Mechanism: A custom Spin-lock with a "Back-off" strategy (100ms polling) to balance CPU efficiency with system responsiveness.

 ## Technical Implementation Details
Concurrency & Integrity Strategy
The system is designed to handle multiple simultaneous voters without data corruption:

Atomicity: Follows a strict lock_database() → FILE* operations → unlock_database() sequence.

Platform Agnostic: Uses conditional preprocessor macros to support both Sleep() (Windows) and usleep() (Unix).

Binary Mirroring: Uses fread and fwrite to ensure data on disk is a bit-for-bit mirror of RAM structures.

## Critical Information:
- To unlock the election, just delete the election.lock file
- The admin password is hard coded: SONU_ADMIN_2026
  
## Installation & Usage
### Requirements
- You must have a C Compiler installed (GCC/MinGW for Windows, or GCC/Clang for Linux/macOS).

**For Windows:**

1. Open your project folder.

2. Double-click build_sonu.bat OR run it via the terminal:

run_sonu.bat

**Unix/Linux/macOS:**
1. Open your terminal in the project folder.

2. Make the script runnable:
chmod +x build_sonu.sh

3.Start the builder:
./build_sonu.sh

## Data Transformation Pipeline
Raw Input: User credentials and ballot selections via interface.c.

Validation: Rule checking and sanitization via voting_logic.c.

Storage: Atomic binary writes via database_mgr.c.

Information: Finalized human-readable report via tally_report.c.
