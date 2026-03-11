#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "interface.h"
#include "voting_logic.h"
#include "database_mgr.h"
#include "tally_report.h"



/**
 * MAIN MENU SYSTEM
 * ---------------------------------------------------------
 * Is the first thing a user sees when they run the program.
 */
        void show_main_menu() {
            printf("\n====================================");
            printf("\n   SONU DIGITAL VOTING SYSTEM 2026  ");
            printf("\n====================================");
            printf("\n1. Student Login & Vote");
            printf("\n2. Admin Portal (Returning Officer)");
            printf("\n3. View Public Candidate List");

            // Results Announcement: Only show results if election is closed
            if (!is_election_open()) {
                printf("\n4. Results Released! View Election Results");
            } else {
                printf("\n4. View Election Results (Available after voting closes)");
            }

            printf("\n0. Exit");
            printf("\nSelection: ");
        }

/**
 * VIEWING 
 * Fetches data from candidates.dat and prints a table with all candidates.
 */

// This is a public view, so we show all candidates across all positions, but we also show their vote counts for transparency.
    void show_candidate_list() {

        Candidate candidates[MAX_CANDIDATES];

        // Fetch all candidates using function from database_mgr.c (using "ALL" as a special position filter)
        int count = get_candidates_by_position("ALL", candidates, MAX_CANDIDATES);

        printf("\n--- REGISTERED CANDIDATES LIST ---\n");

        // 1. Handle File/Database Errors
        if (count == STATUS_FILE_ERROR) {
            printf("[ERROR] Could not open candidate database.\n");
            return;
        }

        // 2. Handle Empty Database
        if (count <= 0) {
            printf("(No candidates found in the system)\n");
            return;
        }

        // 3. Print Header and Data
        printf("%-5s %-20s %-15s %-10s\n", "ID", "Name", "Position", "Votes");
        printf("------------------------------------------------------------\n");

        for (int i = 0; i < count; i++) {
            // Using the same formatting logic as the casting screen, 
            // but adds Position and Vote count for the admin/public view.
            printf("[ID: %d] %-20s [%-15s] Votes: %d\n", 
                candidates[i].id, 
                candidates[i].name, 
                candidates[i].position, 
                candidates[i].votes);
        }
        printf("------------------------------------------------------------\n");
        printf("Total Candidates: %d\n", count);

        
    }


/**
 * ADMIN UI MODULES
 */

/*This is the module that captures candidate credentials and passes them to 
the voting logic module for validation*/
    void process_registration_ui() {
        
        char name[50], position[50];

        printf("\n--- CANDIDATE REGISTRATION ---");

        // Get Candidate Name : getchar() clears the input buffer
        printf("\nEnter Candidate Name: ");
        while (getchar() != '\n'); // clear leftover newline

        //fgets is better than scanf for strings with spaces
        fgets(name, sizeof(name), stdin);
        name[strcspn(name, "\n")] = 0; // remove newline

        //String trimming incase the useraccidentally adds spaces
        while(name[0] == ' ') memmove(name, name+1, strlen(name)); // trim leading spaces

        // Get Candidate Position
        printf("Enter Position: ");
        fgets(position, sizeof(position), stdin);
        position[strcspn(position, "\n")] = 0; // remove newline
        while(position[0] == ' ') memmove(position, position+1, strlen(position)); // trim leading spaces

        // Call the Voting logic module to process this registration
        int result = process_registration(name, position);

        // Show result and return appropriate message to the admin
        if (result == STATUS_SUCCESS) {
            printf("\n[SUCCESS] Candidate registered successfully.\n");
        } else if (result == STATUS_INVALID_POSITION) {
            printf("\n[ERROR] Invalid position. Check spelling or choose a valid SONU position.\n");
        } else if (result == STATUS_ELECTION_CLOSED) {
            printf("\n[ERROR] Cannot register candidate: Election is closed.\n");
        } else if (result == STATUS_INVALID_INPUT) {
            printf("\n[ERROR] Invalid input. Name or position may be blank or too long.\n");
        } else {
            printf("\n[ERROR] Registration failed due to unknown error.\n");
        }
    }

// This is what the admin uses to feed input to the system to create new voter accounts
// It connects to the database manager to save the new voter record.
    void register_voter_ui() {
        Voter v;
        
        printf("\n--- AUTHORIZE NEW VOTER ---");
        printf("\nEnter Student Reg No (e.g., P15/1234/2026): ");
        
        // Clear input buffer safely to ensure scanf doesnt skip
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        scanf("%19s", v.reg_no);
        printf("Create Temporary Password for Student: ");
        scanf("%19s", v.password);

        // New students must start with has_voted = 0, meaning they haven't voted
        v.has_voted = 0;

        // Save to voters.dat 
        int result = save_voter(v);

        // Interpret result and display appropriate message to the admin
        if (result == STATUS_SUCCESS) {
            printf("\n[SUCCESS] Student %s is now authorized to vote.\n", v.reg_no);
        } else if (result == STATUS_VOTER_ALREADY_EXISTS) {
            printf("\n[ERROR] Student %s is already registered.\n", v.reg_no);
        } else if (result == STATUS_FILE_ERROR) {
            printf("\n[ERROR] Could not access voter database. Try again later.\n");
        } else if (result == STATUS_ELECTION_CLOSED) {
            printf("\n[ERROR] Cannot register voter: Election is closed.\n");
        } else {
            printf("\n[ERROR] Authorization failed due to unknown error.\n");
        }
    }

 /* This is the function used to display the final results after the election is closed
  It checks if the election is still open, and if not, it reads from the "final_results.txt" file and prints it to the console in a nice format using the print_results_from_file function from tally_report.c */
        void view_results_ui() {
            
            // Check if election is still open
            if (is_election_open()) {
                printf("\n[INFO] Results are not available. Election is still ongoing.\n");
                return;
            }
            // If election is closed, we can read from the final_results.txt file and print it
            print_results_from_file();
        }


 /**
  * VOTING WORKFLOW
  * ---------------------------------------------------------
  * The core interaction loop for casting ballots.
  */
 
        void casting_screen(const char *reg_no) {
            int choice_id;

            printf("\n--- OFFICIAL BALLOT FOR %s ---", reg_no);

            //Ballot Loop: We iterate through each position and present the candidates for that position, 
            //then record the vote.
            for (int i = 0; i < NUM_POSITIONS; i++) {
                printf("\n\nVoting for Position: [%s]\n", VALID_POSITIONS[i]);

                // Fetch candidates for this position
                Candidate candidates[MAX_CANDIDATES] = {0};
                int count = get_candidates_by_position(VALID_POSITIONS[i], candidates, MAX_CANDIDATES);


                // Handle potential errors from fetching candidates
                if (count <= 0) {
                    printf("  (No candidates registered for this position)\n");
                    continue;
                }

                // Display candidates for this position
                for (int j = 0; j < count; j++) {
                    printf("  [ID: %d] %-20s\n", candidates[j].id, candidates[j].name);
                }


                printf("\nEnter Candidate ID (or 0 to abstain): ");

                // Input validation: Ensure we get a valid integer and that it corresponds to a candidate ID or 0 for abstain
                if (scanf("%d", &choice_id) != 1) {

                    // If input isn't a number, skip this position to avoid infinite loop
                    while (getchar() != '\n' && getchar() != EOF); 
                    printf("  [ERROR] Non-numeric input. Skipping position.\n");
                    continue; 
                }
                while (getchar() != '\n' && getchar() != EOF); // Flush after successful scan

                if (choice_id == 0) {
                    printf("  You chose to abstain from voting for this position.\n");
                    continue;
                }

                // Check if the entered ID is valid for this position
                int valid = 0;
                for (int j = 0; j < count; j++) {
                    if (candidates[j].id == choice_id) {
                        valid = 1;
                        break;
                    }
                }
                if (!valid) {
                    printf("  [ERROR] Invalid candidate ID.\n");
                    i--; // retry the same position
                    continue;
                }

                // Increment the vote count for the chosen candidate in the candidate database
                int res = increment_vote(choice_id);
                if (res == STATUS_SUCCESS) {
                    printf("  Vote recorded successfully.\n");
                } else {
                    printf("  [ERROR] Could not record vote.\n");
                }
            }

            // After voting through all positions, we mark the voter as done to prevent multiple voting attempts.
            int done_res = mark_voter_done(reg_no);
            if (done_res == STATUS_SUCCESS) {
                printf("\n[SUCCESS] Your ballot is cast and your account is now locked.\n");
            } else {
                printf("\n[ERROR] Could not finalize your vote.\n");
            }
        }


/**
 * AUTHENTICATION & VOTER CHECKS
 * ---------------------------------------------------------
 * Is the entry point for students to log in and access the voting screen
 * It connects to the voting logic module to authenticate and check voter status before allowing access to the ballot
 */

    // This is the function that handles student login, authentication, and access control to the voting screen
        void login_screen() {
            char reg[20], pass[20];
            
            
            printf("\n--- STUDENT LOGIN ---");
            printf("\nRegistration No: ");
            scanf("%19s", reg);
            printf("Password: ");
            scanf("%19s", pass);

            //Call the voting logic module to authenticate the voter and check if they can vote before allowing access to the casting screen
            int auth_status = authenticate_voter(reg, pass);
            if (auth_status != STATUS_SUCCESS) {
                if (auth_status == STATUS_VOTER_NOT_FOUND) {
                    printf("\n[ERROR] Voter not found.\n");
                } else {
                    printf("\n[ERROR] Invalid credentials.\n");
                }
                return;
            }

            //Check if they have already voted or not using the can_vote function from voting_logic.c
            int vote_status = can_vote(reg);
            if (vote_status != STATUS_SUCCESS) {
                if (vote_status == STATUS_ALREADY_VOTED) {
                    printf("\n[ERROR] Access Denied: You have already cast your ballot.\n");
                } else {
                    printf("\n[ERROR] Cannot proceed. Voter record not found.\n");
                }
                return;
            }

            // Proceed to cast the vote
            casting_screen(reg); 
        }


    // This function displays the candidates for a specific position, used in the casting screen to show the ballot options for each position
        void show_candidates_by_position(const char* pos) {
            //a local list to hold candidates fetched from the database
            Candidate candidates[MAX_POSITIONS] = {0};

            int count = get_candidates_by_position(pos, candidates, MAX_POSITIONS);

            if (count > 0) {
                int printed = 0;
                for (int i = 0; i < count; i++) {
                    if (candidates[i].id != 0) {
                        printf("  [ID: %d] %-20s\n", candidates[i].id, candidates[i].name);
                        printed++;
                    }
                }
            } else if (count == STATUS_FILE_ERROR) {
                printf("  [ERROR] Could not open candidates data file.\n");
            } else {
                printf("  (No candidates registered for this position)\n");
            }
        }

/** 
 * ADMIN PORTAL
 * ---------------------------------------------------------
 * Requires a hardcoded passkey to prevent students from ending the election.
 * The passkey is "SONU_ADMIN_2026" and is checked against user input before granting access to the admin menu
 */
    void show_admin_menu() {
        char admin_key[20];
        int choice = 0;

        // Clear leftover newlines
        while (getchar() != '\n' && getchar() != EOF); 

        printf("\n--- ADMIN AUTHENTICATION ---");

        // Prompt for the admin security key
        printf("\nEnter Admin Security Key: ");
        scanf("%19s", admin_key);

        // Check the entered key against the hardcoded admin key
        if (strcmp(admin_key, "SONU_ADMIN_2026") == 0) {
            choice = 0;
            while (choice != 6) { 
                printf("\n--- RETURN OFFICER CONTROL PANEL ---");
                printf("\n1. Register New Candidate");
                printf("\n2. Authorize New Student (Voter Registration)");
                printf("\n3. END ELECTION & SECURE FILES");
                printf("\n4. Generate Final Report");
                printf("\n5. View Election Results");
                printf("\n6. Logout");
                printf("\nChoice: ");
                
                if (scanf("%d", &choice) != 1) {
                    while (getchar() != '\n'); // clear bad input
                    continue;
                }

                switch(choice) {
                    case 1:
                    /*This is the UI function that connects to the voting logic module to register a new candidate
                    Which in turn calls the database manager to add the candidate to the database*/ 
                        process_registration_ui();
                        break;

                    case 2:

                    // This is the UI function that connects to the database manager to create new voter accounts
                        register_voter_ui();
                        break;

                    case 3: { //SECURE FILES (and ends election  by creating "election.lock")
                        int result = secure_files();
                        if (result == STATUS_SUCCESS) {
                            printf("\n[SUCCESS] Election ended. Files secured locally.\n");
                        } else {
                            printf("\n[ERROR] Could not secure files.\n");
                        }
                        break;
                    }

                    
                    case 4: { // GENERATE FINAL REPORT

                        if (is_election_open()) {
                            printf("\n[ERROR] Cannot generate final report. Election is still ongoing.\n");
                            break;
                        }

                        // This function generates the final report and saves it to "final_report.txt"
                        int result = print_and_save_final_report();
                        if (result >= 0) {
                            printf("\n[SUCCESS] Final report generated and saved.\n");
                        } else {
                            printf("\n[ERROR] Could not generate report.\n");
    }
                        break;
                    }

                    case 5:
                    // This is the UI function that connects to the tally report module to read the final results from the "final_results.txt" file and print it to the console in a nice format
                        view_results_ui();
                        break;

                    case 6:
                    // Logout option to exit the admin menu and return to the main menu
                        printf("\nLogging out...\n");
                        break;

                    default:
                        printf("\n[ERROR] Invalid choice. Try again.\n");
                        break;
                }
            }
        } else {
            printf("\n[ALERT] Unauthorized Access Attempt Logged.\n");
        }
    }


