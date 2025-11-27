// consensus.h - Consensus algorithms

#ifndef CONSENSUS_H
#define CONSENSUS_H

#include "common.h"
#include "node.h"
#include "transaction.h"
#include "dag.h"

// Function declarations
int get_consensus_algorithm(Node* node, int phase);
int execute_consensus(Transaction* tx, Node* node, DAG* dag, int algorithm);

// Specific consensus algorithms
int fast_voting_consensus(Transaction* tx, Node* node);
int weighted_dag_consensus(Transaction* tx, Node* node, DAG* dag);
int bft_consensus(Transaction* tx, Node* node);

#endif