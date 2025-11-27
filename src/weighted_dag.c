// weighted_dag.c - Weighted DAG consensus (Phantom-style)

#include "consensus.h"

int weighted_dag_consensus(Transaction* tx, Node* node, DAG* dag) {
    // Find transaction in DAG
    int tx_index = -1;
    for (int i = 0; i < dag->count; i++) {
        if (dag->transactions[i].tx_id == tx->tx_id &&
            dag->transactions[i].sender == tx->sender) {
            tx_index = i;
            break;
        }
    }
    
    if (tx_index == -1) {
        return 0;  // Transaction not in DAG
    }
    
    // Check weight
    int weight = get_weight(dag, tx_index);
    
    if (weight >= WD_MIN_WEIGHT) {
        return 1;  // Accept
    }
    
    return 0;  // Not enough weight yet
}