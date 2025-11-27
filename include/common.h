
// common.h - Common definitions

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>
#include "../config.h"

// Forward declarations only (no typedef yet)
struct Node;
struct Transaction;
struct DAG;
struct Metrics;

// Utility Macros
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// Random double in [0, 1)
static inline double rand_double() {
    return (double)rand() / RAND_MAX;
}

#endif