#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h" // Required: Defines Candidate and Voter structs
#include "utils.h"  // For locking functions

/**
 * Function: secure_files
 * Responsibility: Ensures no more votes can be cast once tallying starts.
 * This is done by creating a lock file "election.lock" that signals the election is closed.
 */

int secure_files() {
    // Create the lock file: "election.lock" to signal that the election is closed
    FILE *lock = fopen("election.lock", "w");
    
    if (lock) {// If we successfully created the lock file, we can write a message inside it for clarity
        fprintf(lock, "Election closed for tallying.");
        fclose(lock);
        printf("[SECURITY] Election Locked. No further votes will be accepted.\n");
    } else {
        printf("[ERROR] Failed to lock the election.\n");
        return STATUS_FILE_ERROR;
    }

    return STATUS_SUCCESS;
}


/**
 * Function: calculate_results
 * Responsibility: Reads all candidates from candidates.dat and finds the 
 * max votes per position to determine winners.
 */
int calculate_results(Candidate *winners_out) {

    // Acquire lock before accessing the database
    lock_database(); 

    FILE *fp = fopen("candidates.dat", "rb");
    if (fp == NULL) {
        printf("Error: Could not open candidates database.\n");
        return STATUS_FILE_ERROR; // File error
    }

    Candidate c;
    int position_count = 0; // To track how many unique positions we have winners for

    printf("\n--- Calculating Final Results ---\n");

    while (fread(&c, sizeof(Candidate), 1, fp)) {

        // Check if we already tracked a leader for this position
        int found = 0;
        
        for (int i = 0; i < position_count; i++) {
            if (strcmp(winners_out[i].position, c.position) == 0) {
                found = 1;
                if (c.votes > winners_out[i].votes) {
                    winners_out[i] = c; // New leader for this position
                } else if (c.votes == winners_out[i].votes && winners_out[i].votes > 0) {
                    printf("[!] TIE DETECTED for %s between %s and %s\n", 
                        c.position, winners_out[i].name, c.name);
                }
                break;
            }
        }

        // If it's a new position, add this candidate as the current leader
        if (!found && position_count < MAX_POSITIONS) {
            winners_out[position_count] = c;
            position_count++;
        }
    }

    fclose(fp);

    // Release lock after done accessing the database
    unlock_database(); 

    if (position_count == 0) return STATUS_NO_CANDIDATES_FOUND; 

    return position_count; // Return how many positions have winners
}

/**
 * Function: print_and_save_final_report
 * Responsibility: Prints a formatted "SONU Election Results" table and saves them to a file.
 */
int print_and_save_final_report() {

    // Ensure election is closed first
    FILE *lock = fopen("election.lock", "rb");
    if (!lock) {
        printf("[ERROR] Cannot generate report: Election is still open.\n");
        return STATUS_ELECTION_ONGOING;
    }
    fclose(lock);


    Candidate c;
    Candidate winners[MAX_POSITIONS] = {0};
    int position_count = calculate_results(winners);

    //  Handle exceptions from the calculation phase
    if (position_count < 0) {

        // Error already printed inside calculate_results function
        return position_count; 
    }

    // Open file for saving results (overwrite each time)
    FILE *out = fopen("final_results.txt", "w");
    if (!out) {
        printf("[ERROR] Could not create final_results.txt.\n");
        return STATUS_FILE_ERROR;
    }

    // Header for terminal and file
    printf("\n====================================================\n");
    printf("             SONU ELECTION FINAL RESULTS            \n");
    printf("====================================================\n");
    fprintf(out, "====================================================\n");
    fprintf(out, "             SONU ELECTION FINAL RESULTS            \n");
    fprintf(out, "====================================================\n");

    printf("%-5s | %-20s | %-40s | %-5s\n", "ID", "Name", "Position", "Votes");
    printf("----------------------------------------------------\n");
    fprintf(out, "%-5s | %-20s | %-40s | %-5s\n", "ID", "Name", "Position", "Votes");
    fprintf(out, "----------------------------------------------------\n");

    // Print & save winners
    for (int i = 0; i < position_count; i++) {
        printf("%-5d | %-20s | %-40s | %-5d\n",
            winners[i].id, winners[i].name, winners[i].position, winners[i].votes);
        fprintf(out, "%-5d | %-20s | %-40s | %-5d\n",
            winners[i].id, winners[i].name, winners[i].position, winners[i].votes);
    }

    printf("====================================================\n");
    fprintf(out, "====================================================\n");

    fclose(out);
    printf("[INFO] Final results saved to final_results.txt\n");

    return STATUS_SUCCESS;
}

void print_results_from_file() {
    FILE *fp = fopen("final_results.txt", "r");
    if (!fp) {
        printf("[ERROR] No results file found. Make sure the election is closed and results have been generated.\n");
        return;
    }

    char line[256];
    printf("\n--- SONU ELECTION FINAL RESULTS (From File) ---\n");
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);  // prints each line exactly as it is in the file
    }

    fclose(fp);
}
