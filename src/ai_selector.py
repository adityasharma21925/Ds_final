from __future__ import annotations

import math
import pickle
from pathlib import Path
from typing import Dict, Iterable, Tuple


MODEL_PATH = Path(__file__).with_name("ai_model.pkl")


class SoftmaxModel:
    def __init__(self, labels: Iterable[str], weights: Dict[str, Iterable[float]]):
        self.labels = list(labels)
        self.weights = {lbl: list(weights[lbl]) for lbl in self.labels}

    def predict(self, features):
        scores = []
        for label in self.labels:
            w = self.weights[label]
            score = sum(f * w_i for f, w_i in zip(features, w))
            scores.append(score)
        max_score = max(scores)
        exp_scores = [math.exp(s - max_score) for s in scores]
        denom = sum(exp_scores)
        probs = [e / denom for e in exp_scores]
        best_idx = max(range(len(self.labels)), key=lambda i: probs[i])
        return self.labels[best_idx], probs[best_idx]


def _load_model() -> SoftmaxModel | None:
    if not MODEL_PATH.exists():
        return None
    with MODEL_PATH.open("rb") as f:
        raw = pickle.load(f)
    return SoftmaxModel(raw["labels"], raw["weights"])


MODEL = _load_model()


def _build_feature_vector(metrics: dict):
    bias = 1.0
    phase = float(metrics.get("phase", 1))
    zone_size = float(metrics.get("zone_size", 1))
    network_size = float(metrics.get("network_size", zone_size))
    avg_latency = float(metrics.get("avg_latency_ms", 0.0))
    tx_hint = float(metrics.get("tx_count_hint", 0.0))
    permissioned = 1.0 if metrics.get("permissioned", False) else 0.0
    return [bias, phase, zone_size, network_size, avg_latency, tx_hint, permissioned]


def _heuristic_fallback(metrics: dict) -> str:
    permissioned = bool(metrics.get("permissioned", False))
    phase = int(metrics.get("phase", 1))
    zone_size = int(metrics.get("zone_size", 1))
    network_size = int(metrics.get("network_size", zone_size))
    avg_latency = float(metrics.get("avg_latency_ms", 0.0))
    tx_hint = float(metrics.get("tx_count_hint", 0.0))

    if permissioned:
        return "bft"
    if phase >= 2 or zone_size > 64 or tx_hint > 1500:
        return "fast_voting"
    if (phase == 1 and avg_latency < 250) or (network_size > 32 and avg_latency < 200):
        return "dag"
    return "bft"


def select(metrics: dict) -> str:
    vector = _build_feature_vector(metrics)
    if MODEL is not None:
        label, confidence = MODEL.predict(vector)
        if confidence >= 0.45:
            return label
    return _heuristic_fallback(metrics)


if __name__ == "__main__":
    demo_metrics = {
        "zone_id": 1,
        "zone_size": 50,
        "network_size": 200,
        "phase": 2,
        "avg_latency_ms": 180.0,
        "tx_count_hint": 2200,
        "permissioned": False,
    }
    print("Demo selection:", select(demo_metrics))
