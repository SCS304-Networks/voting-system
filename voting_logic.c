#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "database_mgr.h"
#include "voting_logic.h"
#include "utils.h" // For locking functions for concurrency management

/*
 * Returns the next candidate ID to assign.
 * Candidate IDs are sequential and start at 1.
 */
static int get_next_candidate_id(void)
{
    // Acquire lock before accessing the database
    lock_database(); 

    /* Read existing candidates to determine the highest assigned ID. */
    FILE *fp = fopen("candidates.dat", "rb");
    if (fp == NULL) {
        /* File doesn't exist yet, no candidates registered */
        return 1;
    }

    Candidate c;
    int max_id = 0;

    while (fread(&c, sizeof(Candidate), 1, fp) == 1) {
        if (c.id > max_id) {
            max_id = c.id;
        }
    }

    fclose(fp);

    // Release lock after done accessing the database
    unlock_database();

    // Return the next ID (max_id + 1). If max_id is 0, this will return 1.
    if (max_id < 0) max_id = 0;
    return max_id + 1;
}

/*
 * Returns 1 if the string is NULL or only contains whitespace.
 * Otherwise returns 0.
 */
static int is_blank_string(const char *s)
{
    if (s == NULL) {
        return 1;
    }

    while (*s) {
        if (!isspace((unsigned char)*s)) {
            return 0;
        }
        s++;
    }
    return 1;
}


/*
 * Authenticate a voter using registration number and password.
 * Returns 1 for success, 0 for failure.
 */
int authenticate_voter(const char *reg, const char *pass)
{
    if (reg == NULL || pass == NULL) {
        return STATUS_INVALID_CREDENTIALS;
    }
    //Get the voter copy from the database
    Voter v = load_voter(reg);


    //Check if the voter was found
    // If reg_no is empty, it means load_voter reached the end of the file
    if (v.reg_no[0] == '\0') {
        return STATUS_VOTER_NOT_FOUND; // Not found
    }

    /* Compare provided password to the stored password. */
    return (strcmp(v.password, pass) == 0) ? STATUS_SUCCESS : STATUS_INVALID_CREDENTIALS;
}

/*
 * Checks whether the voter with the given registration number can vote.
 * Returns 1 if they can vote, 0 otherwise.
 */
int can_vote(const char *reg_no)
{
    // Check if reg_no is NULL to avoid passing it to load_voter
    if (reg_no == NULL) return STATUS_VOTER_NOT_FOUND;

    Voter v = load_voter(reg_no);

    // Check if voter exists first
    if (v.reg_no[0] == '\0') return STATUS_VOTER_NOT_FOUND;

    // Return STATUS_SUCCESS if they haven't voted yet
    return (v.has_voted == 0) ? STATUS_SUCCESS : STATUS_ALREADY_VOTED;
}


/*
 * Registers a new candidate.
 * Validates inputs, ensures the position is recognized, and saves the candidate.
 */
int process_registration(const char *name, const char *pos)
{

    // Check if election is even open before doing logic
    if (!is_election_open()) {
        return STATUS_ELECTION_CLOSED; // Code for Election Closed
    }

    // Input validation
    if (name == NULL || pos == NULL) {
        printf("[ERROR] Candidate name and position must be provided.\n");
        return STATUS_INVALID_INPUT; // Code for BlankInput
    }


    // Check for blank or whitespace-only strings
    if (is_blank_string(name) || is_blank_string(pos)) {
        printf("[ERROR] Candidate name and position cannot be blank.\n");
        return STATUS_INVALID_INPUT ; // Code for BlankInput
    }

    /*
     * Validate that the requested position is recognized by the system.
     * get_standardized_position() returns a standardized string (or NULL if invalid).
     */
    const char* official_pos = get_standardized_position(pos);
    

    // If the position is not valid, return an error status and print an error message
    if (official_pos == NULL) {
        
        printf("[ERROR] '%s' is not a valid SONU position or Faculty.\n", pos);
        return STATUS_INVALID_POSITION; // Code for InvalidPosition
    }

    // Additional check to prevent overly long names which could corrupt the file
    if (strlen(name) >= MAX_NAME) {
        printf("[ERROR] Candidate name must be shorter than %d characters.\n", MAX_NAME);
        return STATUS_INVALID_INPUT; // Showing this error for overly long names to prevent file corruption
    }

    Candidate c;

    // Assign a unique ID to the candidate based on the current highest ID in the database
    c.id = get_next_candidate_id();


    /* Store name safely with truncation guard. */
    strncpy(c.name, name, MAX_NAME - 1);
    c.name[MAX_NAME - 1] = '\0';

    /*
     * Use the standardized position label (e.g., "Chairperson") so all candidates
     * for a given role have the same stored position string.
     */
    strncpy(c.position, official_pos, MAX_NAME - 1);
    c.position[MAX_NAME - 1] = '\0';

    c.votes = 0;

    return save_candidate(c);
}