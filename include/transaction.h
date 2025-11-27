// transaction.h - Transaction structure

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "common.h"

typedef struct Transaction {
    int tx_id;
    int sender;
    int receiver;
    double amount;
    int parents[2];              // DAG parent references
    int zone_id;
    int phase;
    double timestamp;
} Transaction;

// Function declarations
Transaction create_transaction(int sender, int receiver, double amount, 
                               int* parents, int zone_id, int phase);
int validate_transaction(Transaction* tx);
void broadcast_transaction(Transaction* tx, MPI_Comm comm);
int receive_transaction(Transaction* tx, MPI_Comm comm);

#endif