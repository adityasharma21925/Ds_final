// transaction.c - Transaction implementation

#include "transaction.h"

Transaction create_transaction(int sender, int receiver, double amount,
                               int* parents, int zone_id, int phase) {
    Transaction tx;
    tx.tx_id = 0;  // Set by caller
    tx.sender = sender;
    tx.receiver = receiver;
    tx.amount = amount;
    tx.parents[0] = parents ? parents[0] : -1;
    tx.parents[1] = parents ? parents[1] : -1;
    tx.zone_id = zone_id;
    tx.phase = phase;
    tx.timestamp = 0.0;  // Set by caller
    return tx;
}

int validate_transaction(Transaction* tx) {
    // Simple validation: amount must be positive
    if (tx->amount <= 0) return 0;
    if (tx->sender < 0 || tx->receiver < 0) return 0;
    return 1;
}

void broadcast_transaction(Transaction* tx, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    // Use non-blocking sends to avoid deadlocks
    MPI_Request* requests = (MPI_Request*)malloc((size - 1) * sizeof(MPI_Request));
    int req_count = 0;
    
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Isend(tx, sizeof(Transaction), MPI_BYTE, i, 0, comm, &requests[req_count++]);
        }
    }
    
    // Wait for all sends to complete
    if (req_count > 0) {
        MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);
    }
    
    free(requests);
}

int receive_transaction(Transaction* tx, MPI_Comm comm) 
{
    MPI_Status status;
int flag;
MPI_Iprobe(MPI_ANY_SOURCE, 0, comm, &flag, &status);
if (flag) {
    MPI_Recv(tx, sizeof(Transaction), MPI_BYTE, status.MPI_SOURCE, 0, comm, &status);
    return 1;
}

    return 0;
}