// config.h - Configuration constants

#ifndef CONFIG_H
#define CONFIG_H

// Zone Formation
#define MAX_ZONES 4
#define ZONE_REBALANCE_INTERVAL 300.0  // seconds
#define LATENCY_WEIGHT 0.6
#define AFFINITY_WEIGHT 0.4

// Phase Detection
#define PHASE_LOW 0
#define PHASE_NORMAL 1
#define PHASE_HIGH 2
#define TAU_HIGH 50.0           // TPS threshold
#define TAU_LOW 10.0            // TPS threshold
#define HYSTERESIS 0.1          // 10%
#define WINDOW_SIZE 60          // seconds
#define CONSECUTIVE_CHECKS 2

// Consensus Algorithms
#define CONSENSUS_FAST_VOTING 0
#define CONSENSUS_WEIGHTED_DAG 1
#define CONSENSUS_BFT 2

// Fast Voting Parameters
#define FV_SAMPLE_SIZE 10
#define FV_QUORUM 7
#define FV_CONSECUTIVE_ROUNDS 5

// Weighted DAG Parameters
#define WD_MIN_WEIGHT 5
#define WD_DECAY 0.1

// BFT Parameters
#define BFT_QUORUM 0.67         // 2/3 majority
#define BFT_TIMEOUT 5.0         // seconds

// Network
#define MAX_NODES 1000
#define MAX_TRANSACTIONS 100000
#define TX_GENERATION_PROB 0.1  // 10% per iteration

// Experiment
#define DEFAULT_EXPERIMENT_DURATION 10.0  // seconds

#endif