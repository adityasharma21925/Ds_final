// dag.c - DAG implementation

#include "dag.h"

DAG* create_dag(int capacity) {
    DAG* dag = (DAG*)malloc(sizeof(DAG));
    dag->transactions = (Transaction*)malloc(capacity * sizeof(Transaction));
    dag->weights = (int*)calloc(capacity, sizeof(int));
    dag->count = 0;
    dag->capacity = capacity;
    return dag;
}

void destroy_dag(DAG* dag) {
    free(dag->transactions);
    free(dag->weights);
    free(dag);
}

int add_transaction(DAG* dag, Transaction* tx) {
    if (dag->count >= dag->capacity) return 0;
    
    dag->transactions[dag->count] = *tx;
    dag->weights[dag->count] = 1;  // Initial weight
    dag->count++;
    return 1;
}

int* get_latest_transactions(DAG* dag, int count) {
    int* parents = (int*)malloc(count * sizeof(int));
    
    if (dag->count == 0) {
        parents[0] = -1;
        parents[1] = -1;
    } else if (dag->count == 1) {
        parents[0] = 0;
        parents[1] = -1;
    } else {
        // Return last two transactions
        parents[0] = dag->count - 2;
        parents[1] = dag->count - 1;
    }
    
    return parents;
}

int get_weight(DAG* dag, int tx_id) {
    if (tx_id < 0 || tx_id >= dag->count) return 0;
    return dag->weights[tx_id];
}

void update_weights(DAG* dag) {
    // Reset weights
    for (int i = 0; i < dag->count; i++) {
        dag->weights[i] = 1;
    }
    
    // Compute cumulative weights (bottom-up)
    for (int i = dag->count - 1; i >= 0; i--) {
        Transaction* tx = &dag->transactions[i];
        
        // Add weight from children (transactions referencing this one)
        for (int j = i + 1; j < dag->count; j++) {
            Transaction* child = &dag->transactions[j];
            if (child->parents[0] == i || child->parents[1] == i) {
                dag->weights[i] += (int)(dag->weights[j] * WD_DECAY);
            }
        }
    }
}