// zones.h - Zone formation

#ifndef ZONES_H
#define ZONES_H

#include "common.h"
#include "node.h"

// Function declarations
int form_zones(Node* node, int k_zones);
void create_zone_communicator(Node* node);
int* identify_witnesses(Node* node, int* zone_assignments, int size, int* count);
double compute_similarity(Node* node, int other_rank);

#endif