#include "metrics.h"

Metrics* create_metrics() {
    Metrics* metrics = (Metrics*)malloc(sizeof(Metrics));
    metrics->total_transactions = 0;
    metrics->finalized_transactions = 0;
    metrics->start_time = 0.0;
    metrics->end_time = 0.0;
    metrics->latency_capacity = 10000;
    metrics->latencies = (double*)malloc(metrics->latency_capacity * sizeof(double));
    metrics->latency_count = 0;
    return metrics;
}

void destroy_metrics(Metrics* metrics) {
    free(metrics->latencies);
    free(metrics);
}

void record_transaction(Metrics* metrics) {
    metrics->total_transactions++;
}

void record_finalization(Metrics* metrics, double creation_time) {
    metrics->finalized_transactions++;
    
    double latency = (MPI_Wtime() - creation_time) * 1000.0;  // Convert to ms
    
    if (metrics->latency_count < metrics->latency_capacity) {
        metrics->latencies[metrics->latency_count++] = latency;
    }
}

void print_metrics(Metrics* metrics, Node* node) {
    double duration = metrics->end_time - metrics->start_time;
    double tps = (double)metrics->finalized_transactions / duration;
    
    // Calculate average latency
    double avg_latency = 0.0;
    if (metrics->latency_count > 0) {
        for (int i = 0; i < metrics->latency_count; i++) {
            avg_latency += metrics->latencies[i];
        }
        avg_latency /= metrics->latency_count;
    }
    
    printf("Node %d: Created %d txs, Finalized %d txs, TPS=%.2f, Avg Latency=%.2f ms\n",
           node->rank, metrics->total_transactions, metrics->finalized_transactions,
           tps, avg_latency);
}

void aggregate_metrics(Metrics* metrics, Node* node) {
    // Aggregate results to rank 0
    double my_tps = (double)metrics->finalized_transactions / 
                    (metrics->end_time - metrics->start_time);
    
    double total_tps;
    int total_finalized;
    
    MPI_Reduce(&my_tps, &total_tps, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&metrics->finalized_transactions, &total_finalized, 1, MPI_INT, 
               MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (node->rank == 0) {
        printf("\n=== AGGREGATE RESULTS ===\n");
        printf("Total Network TPS: %.2f\n", total_tps);
        printf("Total Finalized Transactions: %d\n", total_finalized);
        printf("========================\n");
    }
}