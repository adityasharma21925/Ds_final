# ASTP: Adaptive Spatial-Temporal Partitioning for BlockDAGs

## Overview
MPI implementation of ASTP consensus algorithm for distributed ledgers.

## Requirements
- MPI library (MPICH or OpenMPI)
- GCC compiler
- Linux/Unix environment

## Installation
```bash
# Install MPI (Ubuntu/Debian)
sudo apt-get install mpich

# Compile
make all
```

## Usage
```bash
# Run with 16 nodes
make run16

# Run with 50 nodes
make run50

# Run with 100 nodes
make run100

# Custom run
mpirun -np <N> ./bin/astp
```

## Project Structure
- `include/` - Header files
- `src/` - Source implementations
- `bin/` - Compiled binaries

## Authors
[Your Names]

## Date
[Current Date]
```

---

## **FILE 3: hosts.txt**
```
# MPI Host Configuration
# Format: hostname slots=<cores>

localhost slots=16
# node1 slots=8
# node2 slots=8
# Add more hosts if running on cluster