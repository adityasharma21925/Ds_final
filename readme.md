# ASTP: Adaptive Spatial-Temporal Partitioning for BlockDAGs

An MPI-based distributed blockchain simulator implementing an adaptive consensus system with AI-driven algorithm selection, k-means clustering for zone formation, and dynamic phase detection.

## 📋 Overview

ASTP (Adaptive Spatial-Temporal Partitioning) is a high-performance blockchain simulator that dynamically adapts its consensus mechanism based on network conditions. The system uses:

- **AI-powered consensus selection** - Machine learning models select optimal consensus algorithms
- **K-means clustering** - Groups nodes into zones based on similarity/affinity for efficient communication
- **Adaptive phase detection** - Monitors network load and switches between LOW, NORMAL, and HIGH phases
- **Multiple consensus algorithms** - Supports BFT, Fast Voting, and Weighted DAG protocols
- **DAG-based transaction structure** - Uses Directed Acyclic Graphs for transaction ordering

## ✨ Key Features

### 1. AI-Driven Consensus Algorithm Selection
- Softmax classifier model selects optimal consensus algorithm based on:
  - Current network phase (LOW/NORMAL/HIGH)
  - Zone size and network topology
  - Average latency metrics
  - Transaction volume hints
  - Permissioned/permissionless mode
- Falls back to heuristic rules if AI model unavailable

### 2. K-Means Clustering for Zone Formation
- Groups nodes into zones using k-means clustering
- Clustering based on:
  - **Latency** (60% weight) - Nodes with lower latency grouped together
  - **Affinity** (40% weight) - Nodes that frequently communicate grouped together
- Uses k-means++ initialization for optimal starting centroids
- Automatic zone rebalancing every 300 seconds

### 3. Adaptive Phase Detection
- Monitors transaction throughput using sliding window
- Three phases:
  - **LOW** (0): TPS < 10 (energy-efficient mode)
  - **NORMAL** (1): 10 ≤ TPS ≤ 50 (balanced mode)
  - **HIGH** (2): TPS > 50 (high-throughput mode)
- Hysteresis prevents rapid phase switching

### 4. Multiple Consensus Algorithms
- **BFT (Byzantine Fault Tolerance)**: 2/3 majority consensus, suitable for permissioned networks
- **Fast Voting**: Sampling-based consensus for high-throughput scenarios
- **Weighted DAG**: DAG-based consensus using transaction weights for permissionless networks

### 5. Distributed DAG Structure
- Each node maintains a local DAG of transactions
- Parent-child relationships for transaction ordering
- Dynamic weight computation based on transaction importance
- Periodic weight updates and decay

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    MPI Network Layer                     │
│              (Node Communication & Zones)                │
└────────────────────┬────────────────────────────────────┘
                     │
        ┌────────────┴────────────┐
        │                         │
┌───────▼────────┐      ┌────────▼────────┐
│ Zone Formation │      │ Phase Detection │
│ (K-Means)      │      │ (Sliding Window)│
└───────┬────────┘      └────────┬────────┘
        │                        │
        └────────────┬───────────┘
                     │
        ┌────────────▼────────────┐
        │  AI Consensus Selector  │
        │  (Python/C Integration) │
        └────────────┬────────────┘
                     │
        ┌────────────▼────────────────────┐
        │   Consensus Algorithm Pool       │
        │  • BFT                          │
        │  • Fast Voting                  │
        │  • Weighted DAG                 │
        └─────────────────────────────────┘
```

## 📦 Dependencies

### Required
- **MPI** (MPICH or OpenMPI) - For distributed computing
- **GCC** - C11 compiler with support for GNU extensions
- **Python 3** - For AI consensus selection
- **Make** - Build system

### Python Packages (for AI features)
- Standard library only (no external dependencies required)
- Optional: `numpy`, `sklearn` for enhanced AI features (falls back to heuristics if unavailable)

## 🚀 Installation

### macOS
```bash
# Install MPI via Homebrew
brew install open-mpi

# Verify installation
mpicc --version
```

### Ubuntu/Debian
```bash
# Install MPI
sudo apt-get update
sudo apt-get install mpich libmpich-dev

# Verify installation
mpicc --version
```

### Build the Project
```bash
# Clone or navigate to project directory
cd astp-mpi-copy

# Clean previous builds (optional)
make clean

# Build the project
make

# The binary will be created in bin/astp
```

## 💻 Usage

### Basic Usage
```bash
# Run with 8 nodes for 5 seconds
mpirun -np 8 bin/astp 5

# Run with 16 nodes (uses default 10 seconds)
mpirun -np 16 bin/astp

# Run with 50 nodes for 30 seconds
mpirun -np 50 bin/astp 30
```

### Using Makefile Shortcuts
```bash
# Quick runs with predefined node counts
make run16   # 16 nodes
make run50   # 50 nodes
make run100  # 100 nodes
```

### Multi-Host Execution
Edit `hosts.txt` to configure multiple hosts:
```
localhost slots=16
node1 slots=8
node2 slots=8
```

Then run:
```bash
mpirun -hostfile hosts.txt bin/astp 10
```

### Command Line Arguments
- **Duration** (optional): Simulation duration in seconds (default: 10.0)
  ```bash
  bin/astp <duration>
  ```

## ⚙️ Configuration

Edit `config.h` to customize behavior:

### Zone Formation
```c
#define MAX_ZONES 4                    // Maximum number of zones
#define ZONE_REBALANCE_INTERVAL 300.0  // Rebalance interval (seconds)
#define LATENCY_WEIGHT 0.6             // Weight for latency in similarity
#define AFFINITY_WEIGHT 0.4            // Weight for affinity in similarity
```

### Phase Detection
```c
#define TAU_HIGH 50.0        // TPS threshold for HIGH phase
#define TAU_LOW 10.0         // TPS threshold for LOW phase
#define WINDOW_SIZE 60       // Sliding window size (seconds)
#define HYSTERESIS 0.1       // Hysteresis factor (10%)
```

### Consensus Algorithms
```c
// Fast Voting
#define FV_SAMPLE_SIZE 10
#define FV_QUORUM 7

// BFT
#define BFT_QUORUM 0.67      // 2/3 majority
#define BFT_TIMEOUT 5.0      // Timeout (seconds)

// Weighted DAG
#define WD_MIN_WEIGHT 5
#define WD_DECAY 0.1
```

### Network
```c
#define MAX_NODES 1000
#define MAX_TRANSACTIONS 100000
#define TX_GENERATION_PROB 0.1  // Transaction generation probability
```

## 📁 Project Structure

```
astp-mpi-copy/
├── include/              # Header files
│   ├── common.h         # Common definitions and utilities
│   ├── consensus.h      # Consensus algorithm interfaces
│   ├── dag.h           # DAG structure definitions
│   ├── metrics.h       # Metrics tracking
│   ├── node.h          # Node structure
│   ├── phases.h        # Phase detection
│   ├── transaction.h   # Transaction structure
│   └── zones.h         # Zone formation
│
├── src/                # Source files
│   ├── main.c         # Main program entry point
│   ├── consensus.c    # AI consensus selector & executor
│   ├── zones.c        # K-means zone formation
│   ├── phases.c       # Phase detection
│   ├── dag.c          # DAG management
│   ├── bft.c          # BFT consensus
│   ├── fast_voting.c  # Fast Voting consensus
│   ├── weighted_dag.c # Weighted DAG consensus
│   ├── transaction.c  # Transaction handling
│   ├── node.c         # Node management
│   ├── metrics.c      # Performance metrics
│   ├── witnesses.c    # Witness node handling
│   ├── ai_selector.py # AI consensus algorithm selector
│   └── zone_formation_ai.py # Zone formation AI (optional)
│
├── bin/               # Compiled binaries
├── obj/               # Object files
├── config.h           # Configuration constants
├── makefile           # Build configuration
├── hosts.txt          # MPI host configuration
└── README.md          # This file
```

## 🔬 Algorithms & Concepts

### Zone Formation (K-Means Clustering)
1. **Similarity Computation**: For each node pair, compute:
   - Normalized latency (lower is better)
   - Transaction affinity (higher is better)
   - Combined similarity score: `0.6 × latency_score + 0.4 × affinity_score`

2. **K-Means++ Initialization**: 
   - Select initial centroids using probability distribution
   - Ensures good spread of starting points

3. **Clustering Iteration**:
   - Assign nodes to nearest centroid based on similarity vectors
   - Update centroids as mean of cluster members
   - Repeat until convergence

### AI Consensus Selection
The AI model uses a softmax classifier with features:
- Network phase (LOW/NORMAL/HIGH)
- Zone size and network topology
- Average latency
- Transaction count hints
- Permissioned mode flag

Selection priority:
1. AI model prediction (if confidence ≥ 0.45)
2. Heuristic fallback based on network conditions

### Phase Detection
Uses a sliding window to track transaction rate:
- **Window Size**: 60 seconds
- **Hysteresis**: 10% threshold to prevent oscillation
- **Consecutive Checks**: Requires 2 consecutive phase changes

### Consensus Algorithms

#### BFT (Byzantine Fault Tolerance)
- Requires 2/3 majority agreement
- Suitable for permissioned networks
- High security, lower throughput

#### Fast Voting
- Samples subset of nodes (default: 10)
- Requires quorum (default: 7/10)
- Multiple consecutive rounds for agreement
- Optimized for high-throughput scenarios

#### Weighted DAG
- Uses transaction weights in DAG
- Decay function for weight updates
- Suitable for permissionless networks
- Good for low-latency scenarios

## 📊 Output & Metrics

The simulator provides detailed metrics:

### Per-Node Metrics
- Created transactions count
- Finalized transactions count
- Transactions per second (TPS)
- Average latency

### Aggregate Metrics
- Total network TPS
- Total finalized transactions

### Example Output
```
=== ASTP Blockchain Simulator ===
Nodes: 8
Duration: 5 seconds
==================================

Node 0: Zone 2 (local rank 0/2)
Node 1: Zone 2 (local rank 1/2)
...
Initialization complete. Starting simulation...

[0.01s] Phase transition: 1 -> 0
[4.65s] Phase transition: 0 -> 1

=== Simulation Complete ===
Node 0: Created 330 txs, Finalized 330 txs, TPS=65.99, Avg Latency=0.41 ms
...

=== AGGREGATE RESULTS ===
Total Network TPS: 373.14
Total Finalized Transactions: 1866
========================
```

## 🧪 Testing & Verification

### Quick Test
```bash
# Test with 4 nodes for 3 seconds
mpirun -np 4 bin/astp 3
```

Expected output:
- Zone assignments printed
- Phase transitions logged
- Final metrics displayed

### Performance Testing
```bash
# Test with varying node counts
for nodes in 8 16 32 64; do
    echo "Testing with $nodes nodes..."
    mpirun -np $nodes bin/astp 10
done
```

## 🐛 Troubleshooting

### Common Issues

**MPI not found**
```bash
# Check MPI installation
which mpicc
mpicc --version

# If not found, install MPI (see Installation section)
```

**Python errors in consensus selection**
- The system falls back to heuristic rules automatically
- Check that Python 3 is available: `python3 --version`

**Zone formation issues**
- Ensure sufficient nodes for clustering (minimum 2)
- Check `MAX_ZONES` configuration in `config.h`

**Hanging/Deadlock**
- Ensure all MPI processes can communicate
- Check firewall settings for multi-host runs
- Verify `hosts.txt` configuration

## 🔧 Development

### Adding New Consensus Algorithms
1. Implement algorithm in `src/your_algorithm.c`
2. Add declaration to `include/consensus.h`
3. Add case to `execute_consensus()` in `src/consensus.c`
4. Update AI model training data if needed

### Modifying Zone Formation
- Edit `src/zones.c` for clustering logic
- Adjust weights in `config.h`
- Modify `src/zone_formation_ai.py` for AI-enhanced clustering

### Customizing AI Selection
- Train new model and save as `src/ai_model.pkl`
- Modify features in `src/ai_selector.py`
- Update heuristic fallback in `_heuristic_fallback()`

## 📝 License

[Specify your license here]

## 👥 Authors

[Your Names/Institution]

## 📅 Date

2024

## 🙏 Acknowledgments

- MPI community for distributed computing framework
- Blockchain research community for consensus algorithms

---

For questions or issues, please check the troubleshooting section or open an issue.
