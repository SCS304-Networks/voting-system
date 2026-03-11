#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interface.h"


// Main function: Entry point of the SONU Voting System 

int main() {
    // Variable to store user's menu choice
    int choice;

    // Flag to control the main loop of the program
    int running = 1;

    printf("--- SONU VOTING SYSTEM CLIENT ---\n");
    

    while (running) {
        show_main_menu();
        
        // Read user input for menu choice and validate it
        if (scanf("%d", &choice) != 1) {
            printf("[ERROR] Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); 
            continue;
        }

        switch (choice) {
            case 1:
            // Call the login screen function to handle student authentication
                login_screen(); 
                break;

            case 2:
            // Call the admin menu function to allow administrators to manage candidates and view results
                show_admin_menu(); 
                break;

            case 3:
            // Call the function to display the list of candidates across all positions
                show_candidate_list(); 
                break;

            case 4:
            // Call the function to display election results to students
                view_results_ui();  
                break;

            case 0:
            // Exit the program gracefully
                printf("\nExiting SONU Voting System. Goodbye!\n");
                running = 0;
                break;

            default:
            // Handle invalid menu choices by displaying an error message
                printf("\n[ERROR] Option %d is not valid. Try again.\n", choice);
                break;
        }
    }
    
    return 0;
}