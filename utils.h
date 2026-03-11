#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

// path to the lock file used for database locking
#define DB_LOCK "database.lock"

// Function prototypes
void lock_database();   //used to acquire the lock before accessing the database
void unlock_database(); //used to release the lock after accessing the database

#endif