#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//constants to define the max length for password, name and registration number
#define MAX_NAME 100 
#define MAX_PASS 20
#define MAX_REG 20
#define MAX_POSITIONS 16
#define MAX_CANDIDATES 200

// List of valid positions in the election
extern const char *VALID_POSITIONS[];

// Number of valid positions
extern const int NUM_POSITIONS;

//status codes for Responses
#define STATUS_ELECTION_ONGOING 0
#define STATUS_SUCCESS 1
#define STATUS_VOTER_ALREADY_EXISTS 2
#define STATUS_ELECTION_CLOSED 3
#define STATUS_FILE_ERROR -4
#define STATUS_INVALID_CREDENTIALS 5
#define STATUS_CANDIDATE_NOT_FOUND 6
#define STATUS_VOTER_NOT_FOUND 7
#define STATUS_INVALID_POSITION 8
#define STATUS_NO_CANDIDATES_FOUND 9
#define STATUS_ALREADY_VOTED 10
#define STATUS_INVALID_INPUT 11
#define STATUS_UNKNOWN_ERROR 99

// #define STATUS_ERROR 2


// This is what a Candidate looks like
typedef struct {
    int id; //they have an id for easy reference
    char name[MAX_NAME]; //their name
    char position[MAX_NAME]; //the position they're vying for
    int votes; //number of votes they have received
} Candidate;

// This defines what a Voter looks like
typedef struct {
    char reg_no[MAX_REG]; //their registration number
    char password[MAX_PASS]; //the password they use to access their voting account
    int has_voted; // 0 for No, 1 for Yes
} Voter;

#endif