// dag.h - DAG data structure

#ifndef DAG_H
#define DAG_H

#include "common.h"
#include "transaction.h"

typedef struct {
    Transaction* transactions;
    int* weights;                // Cumulative weights
    int count;
    int capacity;
} DAG;

// Function declarations
DAG* create_dag(int capacity);
void destroy_dag(DAG* dag);
int add_transaction(DAG* dag, Transaction* tx);
int* get_latest_transactions(DAG* dag, int count);
int get_weight(DAG* dag, int tx_id);
void update_weights(DAG* dag);

#endif