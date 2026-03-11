#ifndef INTERFACE_H
#define INTERFACE_H

#include "common.h"


/* --- Primary Navigation --- */
void show_main_menu();   // The first screen any user sees
void login_screen();    // Entry point for student authentication

/* --- Display Modules --- */
void show_candidate_list();  // Displays all candidates across all positions
void show_candidates_by_position(const char* pos); // Filters candidates for the ballot
void view_results_ui(); // Displays election results to students

/* --- Restricted Areas --- */
void casting_screen(const char *reg_no);  // The secure voting loop
void show_admin_menu();               

#endif /* INTERFACE_H */