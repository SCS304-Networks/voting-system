#ifndef VOTING_LOGIC_H
#define VOTING_LOGIC_H

#include "common.h"

/*
 * Logic Controller (Validation Module)
 * - Authenticates voters
 * - Checks voting eligibility
 * - Validates candidate registration input
 */

int authenticate_voter(const char *reg, const char *pass);
int can_vote(const char *reg_no);
int process_registration(const char *name, const char *pos);

#endif /* VOTING_LOGIC_H */
