// main.c - Main program

#include "common.h"
#include "node.h"
#include "transaction.h"
#include "dag.h"
#include "zones.h"
#include "phases.h"
#include "consensus.h"
#include "metrics.h"

int main(int argc, char** argv) {
    int rank, size;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Seed random number generator
    srand(time(NULL) + rank);
    
    // Create node
    Node* node = create_node(rank, size);
    
    double experiment_duration = DEFAULT_EXPERIMENT_DURATION;
    if (rank == 0 && argc > 1) {
        char* endptr = NULL;
        double cli_duration = strtod(argv[1], &endptr);
        if (endptr != argv[1] && cli_duration > 0.0) {
            experiment_duration = cli_duration;
        } else {
            fprintf(stderr,
                    "Warning: invalid duration input '%s'. Using default %.2f seconds.\n",
                    argv[1], experiment_duration);
        }
    }
    MPI_Bcast(&experiment_duration, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("=== ASTP Blockchain Simulator ===\n");
        printf("Nodes: %d\n", size);
        printf("Duration: %.0f seconds\n", experiment_duration);
        printf("==================================\n\n");
    }
    
    // Assign geography and exchange latencies
    assign_geography(node);
    exchange_latencies(node);
    
    // Form zones
    form_zones(node, MAX_ZONES);
    create_zone_communicator(node);
    
    // Synchronize to ensure all zone info is printed before continuing
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Create local DAG
    DAG* dag = create_dag(MAX_TRANSACTIONS);
    
    // Create sliding window for phase detection
    SlidingWindow* window = create_window(WINDOW_SIZE * 100);
    
    // Create metrics tracker
    Metrics* metrics = create_metrics();
    metrics->start_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("Initialization complete. Starting simulation...\n\n");
    }
    
    // Main simulation loop
    double end_time = metrics->start_time + experiment_duration;
    double shutdown_time = end_time - 0.3; // Stop generating new txs 300ms before end
    double process_time = end_time - 0.1; // Stop processing received txs 100ms before end
    int tx_counter = 0;
    
    while (MPI_Wtime() < end_time) {
        double current_time = MPI_Wtime();
        int can_generate = (current_time < shutdown_time);
        int can_process = (current_time < process_time);
        
        // Detect phase
        int old_phase = node->phase;
        node->phase = detect_phase(node, window, current_time);
        if (node->phase != old_phase && rank == 0) {
            printf("[%.2fs] Phase transition: %d -> %d\n", 
                   current_time - metrics->start_time, old_phase, node->phase);
        }
        
        // Generate transaction probabilistically (stop before shutdown_time)
        if (can_generate && rand_double() < TX_GENERATION_PROB) {
            int* parents = get_latest_transactions(dag, 2);
            
            Transaction tx = create_transaction(
                rank,
                rand() % size,
                rand_double() * 100.0,
                parents,
                node->zone_id,
                node->phase
            );
            
            tx.tx_id = tx_counter++;
            tx.timestamp = current_time;
            
            free(parents);
            
            // Broadcast to zone
            broadcast_transaction(&tx, node->zone_comm);
            
            // Add to local DAG
            add_transaction(dag, &tx);
            add_timestamp(window, current_time);
            node->total_tx_count++;
            
            // Execute consensus (only if we're still processing)
            if (can_process) {
                int algorithm = get_consensus_algorithm(node, node->phase);
                int result = execute_consensus(&tx, node, dag, algorithm);
                
                // Record metrics
                record_transaction(metrics);
                if (result) {
                    record_finalization(metrics, tx.timestamp);
                }
            } else {
                // Just record the transaction without consensus
                record_transaction(metrics);
            }
        }
        
        // Receive transactions from others (only process if can_process)
        Transaction received_tx;
        while (receive_transaction(&received_tx, node->zone_comm)) {
            if (can_process) {
                add_transaction(dag, &received_tx);
                add_timestamp(window, current_time);
                node->affinity_counts[received_tx.sender]++;
                node->total_tx_count++;
            }
            // If can't process, just drain the message
        }

        
        // Update DAG weights periodically
        if ((int)(current_time * 10) % 10 == 0) {
            update_weights(dag);
        }
        
        // Zone rebalancing (every 300 seconds)
        static double last_rebalance = 0;
        if (current_time - last_rebalance > ZONE_REBALANCE_INTERVAL) {
            form_zones(node, MAX_ZONES);
            last_rebalance = current_time;
            if (rank == 0) {
                printf("[%.2fs] Zone rebalancing complete\n", 
                       current_time - metrics->start_time);
            }
        }
        
        // Small sleep to prevent busy-waiting
        usleep(1000);  // 1ms
    }
    
    metrics->end_time = MPI_Wtime();
    
    // Wait a moment to let any in-flight consensus operations (MPI_Allgather) complete
    // We stopped starting new ones 100ms before end, so give them time to finish
    usleep(200000); // 200ms - enough time for any in-flight collectives to complete
    
    // Drain all pending messages to avoid deadlock from blocking sends
    // Continue receiving until no more messages arrive for a period
    Transaction received_tx;
    int consecutive_empty = 0;
    const int max_empty_iterations = 100; // Stop after 100 empty checks
    
    for (int i = 0; i < max_empty_iterations; i++) {
        if (receive_transaction(&received_tx, node->zone_comm)) {
            consecutive_empty = 0; // Reset counter if we got a message
        } else {
            consecutive_empty++;
            if (consecutive_empty > 10) {
                // If we've had 10 consecutive empty checks, likely done
                break;
            }
            usleep(100); // Small sleep between checks
        }
    }
    
    // Final aggressive pass - keep receiving until truly empty
    while (receive_transaction(&received_tx, node->zone_comm)) {
        // Just drain, don't process
    }
    
    // Synchronize within zone first to ensure all zone communication completes
    // This barrier will ensure all sends/receives in the zone are done
    if (node->zone_comm != MPI_COMM_NULL) {
        MPI_Barrier(node->zone_comm);
    }
    
    // Then synchronize all ranks globally before printing results
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("\n=== Simulation Complete ===\n");
    }
    
    print_metrics(metrics, node);
    aggregate_metrics(metrics, node);
    
    // Cleanup
    destroy_metrics(metrics);
    destroy_window(window);
    destroy_dag(dag);
    destroy_node(node);
    
    MPI_Finalize();
    return 0;
}