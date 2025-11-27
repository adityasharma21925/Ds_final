#include "phases.h"

SlidingWindow* create_window(int capacity) {
    SlidingWindow* window = (SlidingWindow*)malloc(sizeof(SlidingWindow));
    window->timestamps = (double*)malloc(capacity * sizeof(double));
    window->head = 0;
    window->count = 0;
    window->capacity = capacity;
    return window;
}

void destroy_window(SlidingWindow* window) {
    free(window->timestamps);
    free(window);
}

void add_timestamp(SlidingWindow* window, double timestamp) {
    window->timestamps[window->head] = timestamp;
    window->head = (window->head + 1) % window->capacity;
    if (window->count < window->capacity) {
        window->count++;
    }
}

double calculate_tps(SlidingWindow* window, double current_time) {
    int valid_count = 0;
    
    for (int i = 0; i < window->count; i++) {
        if (current_time - window->timestamps[i] < WINDOW_SIZE) {
            valid_count++;
        }
    }
    
    return (double)valid_count / WINDOW_SIZE;
}

int detect_phase(Node* node, SlidingWindow* window, double current_time) {
    double tps = calculate_tps(window, current_time);
    
    int current_phase = node->phase;
    
    if (current_phase == PHASE_HIGH) {
        if (tps < TAU_HIGH * (1.0 - HYSTERESIS)) {
            return (tps > TAU_LOW) ? PHASE_NORMAL : PHASE_LOW;
        }
    } else if (current_phase == PHASE_NORMAL) {
        if (tps > TAU_HIGH * (1.0 + HYSTERESIS)) {
            return PHASE_HIGH;
        } else if (tps < TAU_LOW * (1.0 - HYSTERESIS)) {
            return PHASE_LOW;
        }
    } else { // PHASE_LOW
        if (tps > TAU_LOW * (1.0 + HYSTERESIS)) {
            return (tps > TAU_HIGH) ? PHASE_HIGH : PHASE_NORMAL;
        }
    }
    
    return current_phase;
}