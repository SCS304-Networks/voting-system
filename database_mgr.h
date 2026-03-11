#ifndef DATABASE_MGR_H
#define DATABASE_MGR_H

#include "common.h"
extern const char *VALID_POSITIONS[]; // Master list of all valid SONU positions

// ---------------------------

// Database Manager Functions
// These functions handle all interactions with the data files (voters.dat and candidates.dat)

// 1. function to save a new voter to the file(registration)
int save_voter(Voter v);

// 2. function to find a voter by their Registration Number
Voter load_voter(const char *reg_no);

// 3. function to save a new candidate to the file(registration)
int save_candidate(Candidate c);

// 4. function to increment the vote count for a candidate by their ID
int increment_vote(int candidate_id);

// 5. function to mark a voter as having voted (after they cast their ballot)
int mark_voter_done(const char *reg_no);

// 6. function to standardize position input (used to match user input with stored candidate positions)
const char* get_standardized_position(const char *input);

// 7. function to compare strings case-insensitively (used for position matching)
int strcase_compare(const char *s1, const char *s2);


// 8. function to get candidates by position (returns count or error code)
int get_candidates_by_position(const char* pos, Candidate* results, int max_count);

// 9. function to check if election is still open (used to prevent changes after closing)
int is_election_open();

#endif /* DATABASE_MGR_H */