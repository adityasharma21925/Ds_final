#include "consensus.h"
#include <Python.h>
#include <string.h>

// ------------------------------------------------------------------
// Python co-process selector
// ------------------------------------------------------------------
static PyObject* selector_module = NULL;
static PyObject* selector_func = NULL;
static int python_ready = 0;

static void ensure_python_ready(void) {
    if (python_ready) {
        return;
    }

    Py_Initialize();

    // Ensure the src directory (where ai_selector.py lives) is on sys.path
    PyObject* sys_path = PySys_GetObject("path");
    PyObject* src_path = PyUnicode_FromString("./src");
    if (sys_path && src_path) {
        PyList_Append(sys_path, src_path);
    }
    Py_XDECREF(src_path);

    selector_module = PyImport_ImportModule("ai_selector");
    if (!selector_module) {
        PyErr_Print();
        return;
    }

    selector_func = PyObject_GetAttrString(selector_module, "select");
    if (!selector_func || !PyCallable_Check(selector_func)) {
        PyErr_Print();
        Py_XDECREF(selector_func);
        selector_func = NULL;
        Py_DECREF(selector_module);
        selector_module = NULL;
        return;
    }

    python_ready = 1;
}

static PyObject* build_metrics_dict(Node* node, int phase) {
    PyObject* metrics = PyDict_New();
    if (!metrics) return NULL;

    int zone_size = node->total_nodes;
    if (node->zone_comm != MPI_COMM_NULL) {
        MPI_Comm_size(node->zone_comm, &zone_size);
    }

    double avg_latency = 0.0;
    if (node->total_nodes > 0 && node->latencies) {
        double sum = 0.0;
        for (int i = 0; i < node->total_nodes; i++) {
            sum += node->latencies[i];
        }
        avg_latency = sum / node->total_nodes;
    }

    PyObject* value = PyLong_FromLong(node->zone_id);
    PyDict_SetItemString(metrics, "zone_id", value);
    Py_DECREF(value);

    value = PyLong_FromLong(zone_size);
    PyDict_SetItemString(metrics, "zone_size", value);
    Py_DECREF(value);

    value = PyLong_FromLong(node->total_nodes);
    PyDict_SetItemString(metrics, "network_size", value);
    Py_DECREF(value);

    value = PyLong_FromLong(phase);
    PyDict_SetItemString(metrics, "phase", value);
    Py_DECREF(value);

    value = PyFloat_FromDouble(avg_latency);
    PyDict_SetItemString(metrics, "avg_latency_ms", value);
    Py_DECREF(value);

    value = PyFloat_FromDouble((double)node->total_tx_count);
    PyDict_SetItemString(metrics, "tx_count_hint", value);
    Py_DECREF(value);

    int permissioned = (node->zone_id % 2 == 0);
    value = PyBool_FromLong(permissioned);
    PyDict_SetItemString(metrics, "permissioned", value);
    Py_DECREF(value);

    return metrics;
}

static int map_label_to_algorithm(const char* label) {
    if (!label) return -1;
    if (strcmp(label, "bft") == 0 || strcmp(label, "pbft") == 0) {
        return CONSENSUS_BFT;
    }
    if (strcmp(label, "dag") == 0) {
        return CONSENSUS_WEIGHTED_DAG;
    }
    if (strcmp(label, "fast_voting") == 0 || strcmp(label, "nakamoto") == 0) {
        return CONSENSUS_FAST_VOTING;
    }
    return -1;
}

static int select_via_python(Node* node, int phase) {
    ensure_python_ready();
    if (!python_ready || !selector_func) {
        return -1;
    }

    PyObject* metrics = build_metrics_dict(node, phase);
    if (!metrics) {
        return -1;
    }

    PyObject* result = PyObject_CallFunctionObjArgs(selector_func, metrics, NULL);
    Py_DECREF(metrics);

    if (!result) {
        PyErr_Print();
        return -1;
    }

    int algorithm = -1;
    if (PyUnicode_Check(result)) {
        const char* label = PyUnicode_AsUTF8(result);
        algorithm = map_label_to_algorithm(label);
    }
    Py_DECREF(result);
    return algorithm;
}

// ------------------------------------------------------------------
// Flowchart fallback selector
// ------------------------------------------------------------------
static int select_by_flowchart(Node* node, int phase) {
    int permissioned = (node->zone_id % 2 == 0);

    if (permissioned) {
        return CONSENSUS_BFT;
    }

    int high_scalability_needed = (phase == PHASE_HIGH);
    int tolerate_energy_usage = (phase != PHASE_LOW);
    int decentralization_important = (phase != PHASE_LOW);

    if (high_scalability_needed) {
        if (tolerate_energy_usage) {
            return CONSENSUS_FAST_VOTING;
        }
        return CONSENSUS_WEIGHTED_DAG;
    }

    if (decentralization_important) {
        return CONSENSUS_WEIGHTED_DAG;
    }

    return CONSENSUS_BFT;
}

int get_consensus_algorithm(Node* node, int phase) {
    int ai_choice = select_via_python(node, phase);
    if (ai_choice != -1) {
        return ai_choice;
    }
    return select_by_flowchart(node, phase);
}

int execute_consensus(Transaction* tx, Node* node, DAG* dag, int algorithm) {
    if (tx == NULL || node == NULL) return 0;
    
    switch (algorithm) {
        case CONSENSUS_FAST_VOTING:
            return fast_voting_consensus(tx, node);
        case CONSENSUS_WEIGHTED_DAG:
            if (dag == NULL) return 0;
            return weighted_dag_consensus(tx, node, dag);
        case CONSENSUS_BFT:
        default:
            return bft_consensus(tx, node);
    }
}
