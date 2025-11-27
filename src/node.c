#include "node.h"
#include "node.h"

Node* create_node(int rank, int size) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->rank = rank;
    node->total_nodes = size;
    node->zone_id = 0;
    node->phase = PHASE_NORMAL;
    node->x = 0.0;
    node->y = 0.0;
    node->latencies = (double*)calloc(size, sizeof(double));
    node->affinity_counts = (int*)calloc(size, sizeof(int));
    node->total_tx_count = 0;
    node->zone_comm = MPI_COMM_NULL;
    return node;
}

void destroy_node(Node* node) {
    free(node->latencies);
    free(node->affinity_counts);
    if (node->zone_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&node->zone_comm);
    }
    free(node);
}

void assign_geography(Node* node) {
    // Divide nodes into 3 geographic clusters (Asia, Europe, Americas)
    int cluster_size = node->total_nodes / 3;
    int my_cluster = node->rank / cluster_size;
    
    double centers[3][2] = {
        {100.0, 100.0},  // Asia
        {300.0, 100.0},  // Europe
        {500.0, 100.0}   // Americas
    };
    
    if (my_cluster >= 3) my_cluster = 2;
    
    // Add some randomness
    node->x = centers[my_cluster][0] + (rand() % 50) - 25;
    node->y = centers[my_cluster][1] + (rand() % 50) - 25;
}

double calculate_latency(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    double distance = sqrt(dx*dx + dy*dy);
    
    // Convert distance to latency (ms)
    double base_latency = distance * 0.5;
    double noise = (rand() % 20) - 10;  // ±10ms
    
    return MAX(base_latency + noise, 1.0);  // Minimum 1ms
}

void exchange_latencies(Node* node) {
    // Broadcast own coordinates
    double my_coords[2] = {node->x, node->y};
    double* all_coords = (double*)malloc(node->total_nodes * 2 * sizeof(double));
    
    MPI_Allgather(my_coords, 2, MPI_DOUBLE,
                  all_coords, 2, MPI_DOUBLE,
                  MPI_COMM_WORLD);
    
    // Calculate latency to each node
    for (int i = 0; i < node->total_nodes; i++) {
        double other_x = all_coords[i * 2];
        double other_y = all_coords[i * 2 + 1];
        node->latencies[i] = calculate_latency(node->x, node->y, other_x, other_y);
    }
    
    free(all_coords);
}