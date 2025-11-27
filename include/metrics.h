// metrics.h - Performance measurement

#ifndef METRICS_H
#define METRICS_H

#include "common.h"
#include "node.h"

typedef struct {
    int total_transactions;
    int finalized_transactions;
    double start_time;
    double end_time;
    double* latencies;
    int latency_count;
    int latency_capacity;
} Metrics;

// Function declarations
Metrics* create_metrics();
void destroy_metrics(Metrics* metrics);
void record_transaction(Metrics* metrics);
void record_finalization(Metrics* metrics, double creation_time);
void print_metrics(Metrics* metrics, Node* node);
void aggregate_metrics(Metrics* metrics, Node* node);

#endif