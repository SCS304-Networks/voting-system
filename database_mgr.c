#include "common.h"
#include <ctype.h> 
#include "utils.h"

/**
 * CONFIGURATION & CONSTANTS
 * ---------------------------------------------------------
 * Defines the official framework for the SONU election.
 */

// Master list of all 16 valid SONU positions to ensure consistent naming and validation across the system
const char *VALID_POSITIONS[] = {
    "Chairperson", "Vice Chairperson", "Secretary General", 
    "Treasurer", "Organizing Secretary", "Academic Secretary",
    "Faculty of Agriculture Representative", "Faculty of Arts and Social Sciences Representative",
    "Faculty of Built Environment and Design Representative", "Faculty of Business and Management Sciences Representative",
    "Faculty of Education Representative", "Faculty of Engineering Representative", "Faculty of Law Representative",
    "Faculty of Health Sciences Representative", "Faculty of Science and Technology Representative",
    "Faculty of Veterinary Medicine Representative"
};

// Calculate the number of positions based on the size of the array incase the number of positions grows
const int NUM_POSITIONS = sizeof(VALID_POSITIONS) / sizeof(VALID_POSITIONS[0]);


/**
 * UTILITY FUNCTIONS
 * ---------------------------------------------------------
 * State management and string manipulation helpers.
 */

// Checks for the presence of 'election.lock' to determine if the system is in "Read-Only" mode
//NOTE!!: To unlock the election, just delete the election.lock file
int is_election_open() {
    FILE *fp = fopen("election.lock", "rb");

    if (fp) {
        fclose(fp);
        return 0; // Lock file exists, election is CLOSED
    }
    return 1; // No lock file, election is OPEN
}

// Case-insensitive string comparison to handle user input variations (e.g., "chair" vs "Chair")
int strcase_compare(const char *s1, const char *s2) {
    while (*s1 && (tolower((unsigned char)*s1) == tolower((unsigned char)*s2))) {
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

/* Validates and returns the correctly capitalized version from the Master List */
const char* get_standardized_position(const char *input) {
    for (int i = 0; i < NUM_POSITIONS; i++) {
        if (strcase_compare(input, VALID_POSITIONS[i]) == 0) {
            return VALID_POSITIONS[i]; // Return the official version
        }
    }
    return NULL; // Not found
}


/**
 * VOTER DATA MANAGEMENT
 * ---------------------------------------------------------
 * Handles persistence and retrieval for the Student/Voter database.
 */


// function to save a new voter to the file(registration)
// Validates and appends the new Voter record to the binary store
int save_voter(Voter v) {
    if (!is_election_open()) return STATUS_ELECTION_CLOSED; // Election closed

    lock_database(); // Acquire lock before accessing the database to ensure safe concurrent access

    // Duplicate Check: Ensure Registration Number is unique before saving
    FILE *fp = fopen("voters.dat", "rb"); 
    if (fp != NULL) {
        Voter temp;
        while (fread(&temp, sizeof(Voter), 1, fp)) {
            if (strcmp(temp.reg_no, v.reg_no) == 0) {
                fclose(fp);
                unlock_database(); // Release lock if voter already exists
                printf("[ERROR] Voter %s is already registered.\n", v.reg_no);
                return STATUS_VOTER_ALREADY_EXISTS; // 0 = already exists
            }
        }
        fclose(fp);
    }

    // If that registration number is not found, append voter
    fp = fopen("voters.dat", "ab"); // Append Mode ('ab') ensures we don't overwrite existing voter data

    if (fp == NULL) {
        //if the file can't be opened, print an error message and return an error status
        return STATUS_FILE_ERROR;
    }

    fwrite(&v, sizeof(Voter), 1, fp);
    fclose(fp);

    unlock_database(); // Release lock after done accessing the database

    return STATUS_SUCCESS; // Success
}



// Performs a linear search in the binary file to retrieve a specific Student record
Voter load_voter(char *reg_no) {

    // We use static so the memory stays valid after the function ends

    Voter temp; //A temporary variable to hold the voter data as we read through the file

    memset(&temp, 0, sizeof(Voter)); // Initialize to zero to ensure we return an empty voter if not found

    lock_database(); // Acquire lock before accessing the database

    
    FILE *fp = fopen("voters.dat", "rb"); 
    if (fp == NULL) {
        unlock_database(); // Release lock if file cannot be opened
        return temp;
    }

    //we check each voter, one at a time until we find the one we're searching for
    while (fread(&temp, sizeof(Voter), 1, fp)) {

        //compare the registration number of the current voter with the one we're looking for
        if (strcmp(temp.reg_no, reg_no) == 0) {
            fclose(fp);
            unlock_database(); // Release lock after done accessing the database
            return temp;
        }
    }
    fclose(fp);
    unlock_database(); // Release lock after done accessing the database
    memset(&temp, 0, sizeof(Voter)); // Ensure it's empty if not found
    return temp;  // Return an empty voter if not found (caller should check reg_no field)
}

/**
 * CANDIDATE & VOTE LOGIC
 * ---------------------------------------------------------
 * CorE logic for incrementing votes and managing candidates.
 */

 // Registers a new candidate into the 'candidates.dat' binary file
int save_candidate(Candidate c) {
    if (!is_election_open()) return STATUS_ELECTION_CLOSED; // Election closed

    
    FILE *fp = fopen("candidates.dat", "ab"); 

    if (fp == NULL) {
        //print an error message if the file can't be opened
        perror("Failed to open candidates.dat");
        unlock_database(); // Release lock if file cannot be opened
        return STATUS_FILE_ERROR;
    }


    //write the candidate data to the file
    fwrite(&c, sizeof(Candidate), 1, fp); 

    //close the file after writing
    fclose(fp);


    //confirmation message
    printf("Candidate %s saved successfully.\n", c.name); 
    fflush(stdout);
    return STATUS_SUCCESS; // Success
}

// Updates a specific record in the binary file, candidates.dat using a Read-Update (rb+) stream
int increment_vote(int candidate_id) {
    if (!is_election_open()) {
        printf("[SECURITY] Attempted vote rejected. Election is CLOSED.\n");
        return STATUS_ELECTION_CLOSED; // Election Closed
    }

    lock_database(); // Acquire lock before accessing the database
    Candidate c;

    // 'rb+' allows reading and writing
    FILE *fp = fopen("candidates.dat", "rb+"); 

    //check if the file was opened successfully
    if (fp == NULL) {
        printf("Error: Could not open candidates file.\n");
        unlock_database(); // Release lock if file cannot be opened
        return STATUS_FILE_ERROR;
    }

    int found = 0;

    // loop through the file to find the candidate by ID
    while (fread(&c, sizeof(Candidate), 1, fp)) {
        if (c.id == candidate_id) {

            // increase the vote count in RAM
            c.votes++; 
            
            // Move pointer back by 1 struct size to overwrite the exact record we just read

            fseek(fp, -(long)sizeof(Candidate), SEEK_CUR);
            
            if (fwrite(&c, sizeof(Candidate), 1, fp) != 1) {
                fclose(fp);
                unlock_database(); // Release lock if writing failed
                return STATUS_FILE_ERROR; // Writing failed
            }
            found = 1;
            break; 
        }
    }

    fclose(fp);
    unlock_database(); // Release lock after done accessing the database

    if (found) {
        printf("[SUCCESS] Vote recorded for candidate ID %d.\n", candidate_id);
        return STATUS_SUCCESS;
    } else {
        printf("[ERROR] Candidate ID %d not found.\n", candidate_id);
        return STATUS_CANDIDATE_NOT_FOUND;
    }
}


// Flips the 'has_voted' boolean flag to prevent double-voting
int mark_voter_done(char *reg_no) {
    lock_database(); // Acquire lock before accessing the database
    Voter v;


    // Open in Read/Update mode (rb+)
    FILE *fp = fopen("voters.dat", "rb+"); 
    if (fp == NULL) {
        perror("Database Error: Could not open voters.dat");
        unlock_database(); // Release lock if file cannot be opened
        return STATUS_FILE_ERROR;
    }

    int found = 0;
    while (fread(&v, sizeof(Voter), 1, fp)) {
        if (strcmp(v.reg_no, reg_no) == 0) {
            v.has_voted = 1; // Flip the flag to indicate this voter has completed voting

            // Move the file pointer back to the start of this voter's record
            fseek(fp, -(long)sizeof(Voter), SEEK_CUR);
            
            // Overwrite the old record with the new one
            if (fwrite(&v, sizeof(Voter), 1, fp) != 1) {
                fclose(fp);
                unlock_database(); // Release lock if writing failed
                return STATUS_FILE_ERROR; // Writing failed
            }
            found = 1;
            break; 
        }
    }
    fclose(fp);
    unlock_database(); // Release lock after done accessing the database

    if (found) {
        printf("[LOG] Voter %s status updated to: VOTED.\n", reg_no);
        return STATUS_SUCCESS;
    } else {
        printf("[ERROR] Voter %s not found in database.\n", reg_no);
        return STATUS_VOTER_NOT_FOUND;
    }
}

// Retrieves filtered candidate subsets for ballot generation or public viewing
int get_candidates_by_position(const char* pos, Candidate* results, int max_count) {
    
    lock_database(); // Acquire lock before accessing the database

    // Open in append mode and close immediately. 
    // This creates the file if it's missing but DOES NOT erase data if it exists.
    FILE *touch = fopen("candidates.dat", "ab");
    if (touch) fclose(touch);
    
    Candidate temp;
    int count = 0;

    //open the candidates file in read-binary mode
    FILE *fp = fopen("candidates.dat", "rb");
    if (!fp) {
        unlock_database(); // Release lock if file cannot be opened
        return STATUS_FILE_ERROR;
    }

    /// Read all candidates from file
    while (fread(&temp, sizeof(Candidate), 1, fp) == 1 && count < max_count) {

        // Includes specified candidate if position matches or returns all candidates if "ALL" is requested
        if (strcmp(pos, "ALL") == 0 || strcmp(temp.position, pos) == 0) {
            results[count] = temp;
            count++;
        }
    }
    
    fclose(fp);
    unlock_database(); // Release lock after done accessing the database
    return count; // Return number of candidates found
}