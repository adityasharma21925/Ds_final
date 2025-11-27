// bft.c - Byzantine Fault Tolerance consensus (simplified PBFT)

#include "consensus.h"

int bft_consensus(Transaction* tx, Node* node) {
    int zone_rank, zone_size;
    MPI_Comm_rank(node->zone_comm, &zone_rank);
    MPI_Comm_size(node->zone_comm, &zone_size);
    
    // Simplified BFT: just check if 2/3 would accept
    // In full implementation, would have pre-vote and pre-commit rounds
    
    // Each node votes
    int my_vote = validate_transaction(tx) ? 1 : 0;
    
    // For very small zones, just use local vote to avoid collectives
    if (zone_size <= 2) {
        return my_vote; // Accept if valid
    }
    
    // Gather all votes using Allgather
    // NOTE: This requires all zone members to participate
    // The caller must ensure this is only called when all ranks are active
    int* all_votes = (int*)malloc(zone_size * sizeof(int));
    MPI_Allgather(&my_vote, 1, MPI_INT,
                  all_votes, 1, MPI_INT,
                  node->zone_comm);
    
    // Count accepts
    int accept_count = 0;
    for (int i = 0; i < zone_size; i++) {
        if (all_votes[i] == 1) {
            accept_count++;
        }
    }
    
    free(all_votes);
    
    // Check if quorum reached
    double acceptance_ratio = (double)accept_count / zone_size;
    
    if (acceptance_ratio >= BFT_QUORUM) {
        return 1;  // Accept
    }
    
    return 0;  // Reject
}