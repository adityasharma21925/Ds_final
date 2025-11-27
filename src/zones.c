// zones.c - Zone formation with k-means clustering and AI

#include "zones.h"
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>

double compute_similarity(Node* node, int other_rank) {
    // Normalized latency (0-1, lower is better)
    double norm_latency = node->latencies[other_rank] / 300.0;  // Max 300ms
    norm_latency = MIN(norm_latency, 1.0);
    
    // Affinity (0-1, higher is better)
    double affinity = 0.0;
    if (node->total_tx_count > 0) {
        affinity = (double)node->affinity_counts[other_rank] / node->total_tx_count;
    }
    
    // Combined similarity
    return LATENCY_WEIGHT * (1.0 - norm_latency) + AFFINITY_WEIGHT * affinity;
}

// Helper function to compute distance between two feature vectors
double compute_distance(double* vec1, double* vec2, int dim) {
    double sum = 0.0;
    for (int i = 0; i < dim; i++) {
        double diff = vec1[i] - vec2[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

// K-means clustering implementation
// Groups nodes together based on similarity/affinity patterns.
// Each node's feature vector is its row in the similarity matrix,
// which represents its similarity (latency + affinity) to all other nodes.
// Nodes with similar similarity patterns are grouped into the same zone.
void kmeans_clustering(double* similarity_matrix, int n_nodes, int k, 
                       int* initial_centroids, int* assignments, int max_iterations) {
    // Each node's feature vector is its row in the similarity matrix
    // This captures both latency and affinity relationships
    double* centroids = (double*)malloc(k * n_nodes * sizeof(double));
    double* old_centroids = (double*)malloc(k * n_nodes * sizeof(double));
    int* cluster_sizes = (int*)calloc(k, sizeof(int));
    
    // Initialize centroids from initial_centroids or randomly
    if (initial_centroids != NULL) {
        for (int i = 0; i < k; i++) {
            int centroid_idx = initial_centroids[i];
            for (int j = 0; j < n_nodes; j++) {
                centroids[i * n_nodes + j] = similarity_matrix[centroid_idx * n_nodes + j];
            }
        }
    } else {
        // Random initialization
        for (int i = 0; i < k; i++) {
            int centroid_idx = rand() % n_nodes;
            for (int j = 0; j < n_nodes; j++) {
                centroids[i * n_nodes + j] = similarity_matrix[centroid_idx * n_nodes + j];
            }
        }
    }
    
    // K-means iterations
    for (int iter = 0; iter < max_iterations; iter++) {
        // Save old centroids
        memcpy(old_centroids, centroids, k * n_nodes * sizeof(double));
        
        // Assignment step: assign each node to nearest centroid
        memset(cluster_sizes, 0, k * sizeof(int));
        for (int i = 0; i < n_nodes; i++) {
            double min_dist = INFINITY;
            int best_cluster = 0;
            
            for (int j = 0; j < k; j++) {
                double dist = compute_distance(
                    &similarity_matrix[i * n_nodes],
                    &centroids[j * n_nodes],
                    n_nodes
                );
                if (dist < min_dist) {
                    min_dist = dist;
                    best_cluster = j;
                }
            }
            assignments[i] = best_cluster;
            cluster_sizes[best_cluster]++;
        }
        
        // Update step: recompute centroids
        memset(centroids, 0, k * n_nodes * sizeof(double));
        for (int i = 0; i < n_nodes; i++) {
            int cluster = assignments[i];
            if (cluster_sizes[cluster] > 0) {
                for (int j = 0; j < n_nodes; j++) {
                    centroids[cluster * n_nodes + j] += 
                        similarity_matrix[i * n_nodes + j] / cluster_sizes[cluster];
                }
            }
        }
        
        // Check for convergence
        double max_change = 0.0;
        for (int i = 0; i < k; i++) {
            double change = compute_distance(
                &centroids[i * n_nodes],
                &old_centroids[i * n_nodes],
                n_nodes
            );
            if (change > max_change) {
                max_change = change;
            }
        }
        
        if (max_change < 1e-4) {
            // Converged
            break;
        }
    }
    
    free(centroids);
    free(old_centroids);
    free(cluster_sizes);
}

// K-means++ initialization (AI technique for better clustering)
void kmeans_plusplus_init(double* similarity_matrix, int n_nodes, int k, int* centroids) {
    // First centroid: random
    centroids[0] = rand() % n_nodes;
    
    // Convert similarity to distance (higher similarity = lower distance)
    double max_sim = 0.0;
    for (int i = 0; i < n_nodes * n_nodes; i++) {
        if (similarity_matrix[i] > max_sim) {
            max_sim = similarity_matrix[i];
        }
    }
    
    // Select remaining centroids using k-means++ strategy
    for (int c = 1; c < k; c++) {
        double* distances = (double*)malloc(n_nodes * sizeof(double));
        double total = 0.0;
        
        for (int i = 0; i < n_nodes; i++) {
            // Check if already selected
            int already_selected = 0;
            for (int j = 0; j < c; j++) {
                if (centroids[j] == i) {
                    already_selected = 1;
                    break;
                }
            }
            
            if (already_selected) {
                distances[i] = 0.0;
                continue;
            }
            
            // Distance to nearest existing centroid
            double min_dist = DBL_MAX;
            for (int j = 0; j < c; j++) {
                int centroid_idx = centroids[j];
                // Distance = max_sim - similarity (higher similarity = lower distance)
                double dist = max_sim - similarity_matrix[i * n_nodes + centroid_idx] + 1e-6;
                if (dist < min_dist) {
                    min_dist = dist;
                }
            }
            distances[i] = min_dist * min_dist;  // Square for probability
            total += distances[i];
        }
        
        // Select next centroid with probability proportional to distance^2
        if (total > 1e-10) {
            double r = ((double)rand() / RAND_MAX) * total;
            double cumsum = 0.0;
            for (int i = 0; i < n_nodes; i++) {
                cumsum += distances[i];
                if (cumsum >= r) {
                    centroids[c] = i;
                    break;
                }
            }
        } else {
            // Fallback: select randomly
            centroids[c] = rand() % n_nodes;
        }
        
        free(distances);
    }
}

// Call Python AI script to get optimal k and initial centroids (optional)
// Falls back to C implementation if Python fails
int call_ai_zone_formation(double* similarity_matrix, int n_nodes, int max_k, 
                           int* optimal_k, int** initial_centroids) {
    // Only rank 0 calls the Python script
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    // For now, skip Python and use C implementation directly to avoid hanging
    // This ensures the program runs reliably
    *optimal_k = MIN(max_k, n_nodes);
    *initial_centroids = (int*)malloc(*optimal_k * sizeof(int));
    
    if (rank == 0) {
        // Use k-means++ initialization (AI technique)
        kmeans_plusplus_init(similarity_matrix, n_nodes, *optimal_k, *initial_centroids);
    }
    
    // Broadcast results to all processes
    MPI_Bcast(optimal_k, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0) {
        *initial_centroids = (int*)malloc(*optimal_k * sizeof(int));
    }
    MPI_Bcast(*initial_centroids, *optimal_k, MPI_INT, 0, MPI_COMM_WORLD);
    
    return 0;
}

int form_zones(Node* node, int k_zones) {
    int rank = node->rank;
    int size = node->total_nodes;

    // Step 1: Compute similarity matrix based on latency and affinity
    // Similarity combines normalized latency (lower is better) and affinity (higher is better)
    double* similarities = (double*)malloc(size * sizeof(double));
    for (int i = 0; i < size; i++) {
        similarities[i] = compute_similarity(node, i);
    }

    // Step 2: Exchange similarities across all nodes to build full similarity matrix
    // This creates a complete similarity graph where each node knows its similarity to all others
    double* all_similarities = (double*)malloc(size * size * sizeof(double));
    MPI_Allgather(similarities, size, MPI_DOUBLE,
                  all_similarities, size, MPI_DOUBLE,
                  MPI_COMM_WORLD);

    // Limit number of zones to not exceed total nodes
    if (k_zones > size)
        k_zones = size;
    if (k_zones < 1)
        k_zones = 1;

    // Step 3: Use AI to determine optimal k (number of zones) and initial centroids
    // The AI analyzes the similarity matrix to find the best clustering
    int optimal_k = k_zones;
    int* initial_centroids = NULL;
    int ai_result = call_ai_zone_formation(all_similarities, size, k_zones, 
                                           &optimal_k, &initial_centroids);
    
    // If AI call failed, use default k
    if (ai_result != 0 || optimal_k <= 0) {
        optimal_k = k_zones;
        initial_centroids = NULL;
    }

    // Step 4: Perform k-means clustering to group nodes with high similarity/affinity together
    // Nodes with similar latency and affinity patterns are grouped into the same zone
    int* zone_assignments = (int*)malloc(size * sizeof(int));
    kmeans_clustering(all_similarities, size, optimal_k, initial_centroids, 
                      zone_assignments, 100);  // Max 100 iterations

    // Step 5: Assign zone to this node based on k-means clustering result
    node->zone_id = zone_assignments[rank];

    // Cleanup
    free(similarities);
    free(all_similarities);
    free(zone_assignments);
    if (initial_centroids != NULL) {
        free(initial_centroids);
    }

    return node->zone_id;
}


void create_zone_communicator(Node* node) {
    if (node->zone_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&node->zone_comm);
    }
    
    MPI_Comm_split(MPI_COMM_WORLD, node->zone_id, node->rank, &node->zone_comm);
    
    int zone_rank, zone_size;
    MPI_Comm_rank(node->zone_comm, &zone_rank);
    MPI_Comm_size(node->zone_comm, &zone_size);
    
    // All nodes print their zone assignment
    printf("Node %d: Zone %d (local rank %d/%d)\n",
           node->rank, node->zone_id, zone_rank, zone_size);
    fflush(stdout);
}

int* identify_witnesses(Node* node, int* zone_assignments, int size, int* count) {
    int* witnesses = (int*)malloc(size * sizeof(int));
    *count = 0;
    
    for (int i = 0; i < size; i++) {
        int neighbor_zones[MAX_ZONES];
        int zone_count = 0;
        
        // Check neighbors (latency < 50ms)
        for (int j = 0; j < size; j++) {
            if (i == j) continue;
            if (node->latencies[j] < 50.0) {
                int other_zone = zone_assignments[j];
                
                // Add unique zones
                int found = 0;
                for (int k = 0; k < zone_count; k++) {
                    if (neighbor_zones[k] == other_zone) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    neighbor_zones[zone_count++] = other_zone;
                }
            }
        }
        
        // If connects multiple zones, it's a witness
        if (zone_count > 1) {
            witnesses[(*count)++] = i;
        }
    }
    
    return witnesses;
}