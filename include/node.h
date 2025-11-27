// node.h - Node structure and operations

#ifndef NODE_H
#define NODE_H

#include "common.h"

typedef struct {
    int rank;                    // MPI rank
    int total_nodes;             // Total nodes in network
    int zone_id;                 // Current zone
    int phase;                   // Current phase (LOW/NORMAL/HIGH)
    
    // Geographic location (simulated)
    double x, y;
    
    // Latency to other nodes
    double* latencies;
    
    // Transaction tracking
    int* affinity_counts;        // Transactions with each node
    int total_tx_count;
    
    // Zone communicator
    MPI_Comm zone_comm;
} Node;

// Function declarations
Node* create_node(int rank, int size);
void destroy_node(Node* node);
void assign_geography(Node* node);
void exchange_latencies(Node* node);
double calculate_latency(double x1, double y1, double x2, double y2);

#endif