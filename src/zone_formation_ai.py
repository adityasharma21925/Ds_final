#!/usr/bin/env python3
"""
AI-assisted zone formation using k-means clustering.
This script uses AI/ML techniques to determine optimal k and initial centroids
based on similarity/affinity between nodes.
"""

import sys
import json
import math
import random
from pathlib import Path

# Try to use existing AI model if available
MODEL_PATH = Path(__file__).with_name("ai_model.pkl")


def euclidean_distance(vec1, vec2):
    """Compute Euclidean distance between two vectors."""
    return math.sqrt(sum((a - b) ** 2 for a, b in zip(vec1, vec2)))


def compute_silhouette_score(similarity_matrix, assignments, k):
    """
    Compute silhouette score for clustering quality.
    Higher score = better clustering.
    """
    n_nodes = len(similarity_matrix)
    if n_nodes == 0:
        return -1.0
    
    # Convert similarity to distance (higher similarity = lower distance)
    max_sim = max(max(row) for row in similarity_matrix)
    
    def similarity_to_distance(sim):
        return max_sim - sim + 1e-6
    
    scores = []
    for i in range(n_nodes):
        cluster_i = assignments[i]
        
        # Compute average distance to nodes in same cluster
        same_cluster_dists = []
        for j in range(n_nodes):
            if i != j and assignments[j] == cluster_i:
                dist = similarity_to_distance(similarity_matrix[i][j])
                same_cluster_dists.append(dist)
        
        a_i = sum(same_cluster_dists) / len(same_cluster_dists) if same_cluster_dists else 0.0
        
        # Compute minimum average distance to other clusters
        min_other_cluster_dist = float('inf')
        for other_cluster in range(k):
            if other_cluster == cluster_i:
                continue
            other_cluster_dists = []
            for j in range(n_nodes):
                if assignments[j] == other_cluster:
                    dist = similarity_to_distance(similarity_matrix[i][j])
                    other_cluster_dists.append(dist)
            if other_cluster_dists:
                avg_dist = sum(other_cluster_dists) / len(other_cluster_dists)
                min_other_cluster_dist = min(min_other_cluster_dist, avg_dist)
        
        b_i = min_other_cluster_dist if min_other_cluster_dist != float('inf') else a_i
        
        # Silhouette score for this point
        if max(a_i, b_i) == 0:
            s_i = 0.0
        else:
            s_i = (b_i - a_i) / max(a_i, b_i)
        scores.append(s_i)
    
    return sum(scores) / len(scores) if scores else -1.0


def simple_kmeans_assign(similarity_matrix, centroids, k):
    """
    Simple k-means assignment step.
    Returns cluster assignments for each node.
    """
    n_nodes = len(similarity_matrix)
    assignments = []
    
    for i in range(n_nodes):
        min_dist = float('inf')
        best_cluster = 0
        node_features = similarity_matrix[i]
        
        for j in range(k):
            centroid_features = centroids[j]
            dist = euclidean_distance(node_features, centroid_features)
            if dist < min_dist:
                min_dist = dist
                best_cluster = j
        
        assignments.append(best_cluster)
    
    return assignments


def kmeans_plusplus_init(similarity_matrix, k):
    """
    K-means++ initialization: select initial centroids that are well-spread.
    This is an AI/ML technique for better clustering.
    """
    n_nodes = len(similarity_matrix)
    if k >= n_nodes:
        return list(range(n_nodes))
    
    # Convert similarity to distance
    max_sim = max(max(row) for row in similarity_matrix)
    
    def similarity_to_distance(sim):
        return max_sim - sim + 1e-6
    
    # First centroid: random
    centroids = [random.randint(0, n_nodes - 1)]
    centroid_indices = [centroids[0]]
    
    # Select remaining centroids using k-means++ strategy
    for _ in range(k - 1):
        distances = []
        for i in range(n_nodes):
            if i in centroid_indices:
                distances.append(0.0)
                continue
            
            # Distance to nearest existing centroid
            min_dist = float('inf')
            for c_idx in centroid_indices:
                dist = similarity_to_distance(similarity_matrix[i][c_idx])
                min_dist = min(min_dist, dist)
            distances.append(min_dist)
        
        # Select next centroid with probability proportional to distance^2
        total = sum(d * d for d in distances)
        if total == 0:
            # Fallback: select randomly
            next_idx = random.randint(0, n_nodes - 1)
            while next_idx in centroid_indices:
                next_idx = random.randint(0, n_nodes - 1)
            centroid_indices.append(next_idx)
        else:
            r = random.uniform(0, total)
            cumsum = 0.0
            for i, d in enumerate(distances):
                cumsum += d * d
                if cumsum >= r:
                    centroid_indices.append(i)
                    break
    
    return centroid_indices


def determine_optimal_k(similarity_matrix, max_k=None, min_k=2):
    """
    Use AI/ML techniques to determine optimal number of clusters.
    Uses silhouette score to evaluate different k values.
    Optimized for speed - limits computation for large networks.
    """
    n_nodes = len(similarity_matrix)
    if n_nodes < 2:
        return 1
    
    if max_k is None:
        max_k = min(n_nodes // 2, 10)  # Reasonable upper bound
    max_k = min(max_k, n_nodes)
    min_k = min(min_k, max_k)
    
    if max_k < min_k:
        return min_k
    
    # For large networks, use faster heuristic instead of full silhouette computation
    if n_nodes > 50:
        # Use simple heuristic for large networks to avoid timeout
        if n_nodes <= 100:
            return min(4, max_k)
        elif n_nodes <= 200:
            return min(5, max_k)
        else:
            return min(max_k, 6)
    
    # Each node's feature vector is its row in the similarity matrix
    features = similarity_matrix
    
    best_k = min_k
    best_score = -1.0
    
    # Try different k values and use silhouette score (limited iterations for speed)
    # Only try a few k values to speed up
    k_values_to_try = list(range(min_k, min(max_k + 1, min_k + 4)))  # Try at most 4 values
    if max_k not in k_values_to_try:
        k_values_to_try.append(max_k)
    
    for k in k_values_to_try:
        # Get initial centroids using k-means++
        initial_centroids_idx = kmeans_plusplus_init(similarity_matrix, k)
        initial_centroids = [features[idx] for idx in initial_centroids_idx]
        
        # Run a few iterations of k-means to get assignments
        assignments = simple_kmeans_assign(features, initial_centroids, k)
        
        # Update centroids (fewer iterations for speed)
        for _ in range(3):  # Reduced from 5 to 3 for speed
            # Recompute centroids
            new_centroids = []
            for cluster in range(k):
                cluster_points = [features[i] for i in range(n_nodes) if assignments[i] == cluster]
                if cluster_points:
                    # Average of cluster points
                    centroid = [sum(col) / len(col) for col in zip(*cluster_points)]
                    new_centroids.append(centroid)
                else:
                    # Empty cluster, use a random point
                    new_centroids.append(features[random.randint(0, n_nodes - 1)])
            
            # Reassign
            assignments = simple_kmeans_assign(features, new_centroids, k)
        
        # Calculate silhouette score (simplified for speed)
        if len(set(assignments)) > 1:  # Need at least 2 clusters
            # Use simplified silhouette for speed
            score = compute_silhouette_score(similarity_matrix, assignments, k)
            
            if score > best_score:
                best_score = score
                best_k = k
    
    # Fallback: if no good score, use heuristic based on network size
    if best_score < 0.1:
        if n_nodes <= 10:
            best_k = 2
        elif n_nodes <= 30:
            best_k = 3
        elif n_nodes <= 100:
            best_k = 4
        else:
            best_k = min(5, max_k)
    
    return best_k


def get_initial_centroids(similarity_matrix, k):
    """
    Use AI (k-means++ initialization) to select good initial centroids.
    This groups nodes with high similarity/affinity together.
    """
    return kmeans_plusplus_init(similarity_matrix, k)


def main():
    """
    Main function: reads similarity matrix from stdin, performs AI-assisted analysis,
    outputs optimal k and initial centroids for k-means clustering.
    """
    if len(sys.argv) < 2:
        print("Usage: zone_formation_ai.py <max_zones> [similarity_matrix_json]", file=sys.stderr)
        sys.exit(1)
    
    max_zones = int(sys.argv[1])
    
    # Read similarity matrix
    try:
        data = json.load(sys.stdin)
        similarity_matrix = data['similarity_matrix']
        n_nodes = data.get('n_nodes', len(similarity_matrix))
    except Exception as e:
        print(f"Error reading input: {e}", file=sys.stderr)
        sys.exit(1)
    
    # Ensure matrix is square
    if len(similarity_matrix) != len(similarity_matrix[0]):
        print("Error: similarity matrix must be square", file=sys.stderr)
        sys.exit(1)
    
    n_nodes = len(similarity_matrix)
    
    # Determine optimal k using AI techniques
    optimal_k = determine_optimal_k(similarity_matrix, max_k=max_zones)
    optimal_k = min(optimal_k, max_zones)  # Respect max_zones limit
    optimal_k = max(1, optimal_k)  # At least 1 zone
    
    # Get initial centroids using AI (k-means++ initialization)
    # This ensures nodes with high similarity/affinity are grouped together
    initial_centroids = get_initial_centroids(similarity_matrix, optimal_k)
    
    # Output results as JSON
    result = {
        'k': optimal_k,
        'initial_centroids': initial_centroids
    }
    
    print(json.dumps(result))
    return 0


if __name__ == '__main__':
    random.seed(42)  # For reproducibility
    sys.exit(main())
