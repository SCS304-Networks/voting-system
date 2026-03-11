#ifndef TALLY_REPORT_H
#define TALLY_REPORT_H

#include "common.h"   // Required: Defines Candidate and Voter structs, and any necessary constants

// ------------------------
// Election Result Handling
// ------------------------

/**
 * Function: calculate_results
 * Reads all candidates from candidates.dat to find the maximum 
 * votes per position and determine the winners.
 * @param winners_out: array of Candidate to store winners
 * @return number of positions with winners
 */
int calculate_results(Candidate *winners_out);

/**
 * Function: print_and_save_final_report
 * Generates the final report: prints to terminal and saves to final_results.txt.
 * Only allowed if election is closed (checks election.lock).
 */
int print_and_save_final_report();


/**
 * Function: print_results_from_file
 * Prints the final results directly from the file to the terminal.
 * Used by client UI to show the results.
 */
void print_results_from_file();

/**
 * Function: secure_files
 * Ensures no more votes can be cast once the tallying process starts. 
This is done by creating a lock file "election.lock" that signals the election is closed.
*/
int secure_files();

#endif // TALLY_REPORT_H