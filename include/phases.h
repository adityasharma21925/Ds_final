// phases.h - Phase detection

#ifndef PHASES_H
#define PHASES_H

#include "common.h"
#include "node.h"

typedef struct {
    double* timestamps;
    int head;
    int count;
    int capacity;
} SlidingWindow;

// Function declarations
SlidingWindow* create_window(int capacity);
void destroy_window(SlidingWindow* window);
void add_timestamp(SlidingWindow* window, double timestamp);
int detect_phase(Node* node, SlidingWindow* window, double current_time);
double calculate_tps(SlidingWindow* window, double current_time);

#endif