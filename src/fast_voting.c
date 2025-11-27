// fast_voting.c - Fast Voting consensus (Avalanche-style)

#include "consensus.h"

int fast_voting_consensus(Transaction* tx, Node* node) {
    int zone_rank, zone_size;
    MPI_Comm_rank(node->zone_comm, &zone_rank);
    MPI_Comm_size(node->zone_comm, &zone_size);
    
    // If zone too small, accept immediately
    if (zone_size < FV_SAMPLE_SIZE) {
        return 1;
    }
    
    int preference = -1;  // -1=undecided, 0=reject, 1=accept
    int consecutive = 0;
    
    for (int round = 0; round < FV_CONSECUTIVE_ROUNDS + 10; round++) {
        // Sample random nodes from zone
        int accept_count = 0;
        int reject_count = 0;
        
        for (int i = 0; i < FV_SAMPLE_SIZE; i++) {
            int sample_rank = rand() % zone_size;
            
            // Simulate vote (in real implementation, would query node)
            // Simple heuristic: accept if tx is valid
            int vote = validate_transaction(tx) ? 1 : 0;
            
            if (vote == 1) {
                accept_count++;
            } else {
                reject_count++;
            }
        }
        
        // Determine new preference
        int new_preference = preference;
        if (accept_count >= FV_QUORUM) {
            new_preference = 1;  // Accept
        } else if (reject_count >= FV_QUORUM) {
            new_preference = 0;  // Reject
        } else {
            consecutive = 0;
            continue;
        }
        
        // Update consecutive counter
        if (new_preference == preference) {
            consecutive++;
        } else {
            preference = new_preference;
            consecutive = 1;
        }
        
        // Check if decision reached
        if (consecutive >= FV_CONSECUTIVE_ROUNDS) {
            return preference;
        }
    }
    
    return 0;  // Reject if no consensus
}