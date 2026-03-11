#include "utils.h"
#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
    #define PLATFORM_SLEEP(ms) Sleep(ms)
#else
    #include <unistd.h>
    //since usleep uses microseconds, we multiply ms by 1000
    #define PLATFORM_SLEEP(ms) usleep((ms) * 1000)
#endif

// This file contains utility functions for database locking to ensure safe concurrent access.

void lock_database() {
    FILE *lock;
    int waiting_notified = 0; // Flag to ensure we only print the message once

    // Checking for the Lock File
    while ((lock = fopen(DB_LOCK, "r")) != NULL) {
        fclose(lock);

        // If this is the first time we realize we have to wait, tell the user.
        if (!waiting_notified) {
            printf("\n[INFO] Database is busy. Please wait while your request is queued...");
            fflush(stdout); // makes the message show immediately
            waiting_notified = 1;
        }

        PLATFORM_SLEEP(100); // Wait 100ms before checking again to reduce CPU usage
    }

    // If we had to wait, let them know we are now proceeding
    if (waiting_notified) {
        printf("\n[INFO] Database available. Processing now...\n");
    }

    // Step 2: Acquiring the Lock
    lock = fopen(DB_LOCK, "w");
    if (lock) {
        fprintf(lock, "LOCKED");
        fclose(lock);
    }
}

// This function removes the lock file to release the lock after database operations are complete.
void unlock_database() {
    // Step 3: Releasing the Lock
    remove(DB_LOCK);
}