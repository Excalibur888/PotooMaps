#include "graph.h"

#ifdef _GRAPH_MAT

typedef struct sGraph {
    /// @brief Matrice des poids.
    float **arcs;
    /// @brief Tableau contenant les degrés entrants.
    int *negativeValencies;
    /// @brief Tableau contenant les degrés sortants.
    int *positiveValencies;
    /// @brief Nombre de noeuds du graphe.
    int size;
} Graph;

Graph *Graph_create(int size) {
    if (size < 1) return NULL;
    Graph *graph = calloc(1, sizeof(Graph));
    graph->arcs = calloc(size, sizeof(float *));
    for (int i = 0; i < size; ++i) {
        graph->arcs[i] = calloc(size, sizeof(float));
        for (int j = 0; j < size; ++j) {
            graph->arcs[i][j] = -1;
        }
    }
    graph->negativeValencies = calloc(size, sizeof(int));
    graph->positiveValencies = calloc(size, sizeof(int));
    graph->size = size;
    return graph;
}

void Graph_destroy(Graph *graph) {
    assert(graph);
    for (int i = 0; i < graph->size; ++i) {
        free(graph->arcs[i]);
    }
    free(graph->arcs);

    free(graph->negativeValencies);
    free(graph->positiveValencies);

    free(graph);
}

int Graph_size(Graph *graph) {
    assert(graph);
    return graph->size;
}

void Graph_print(Graph *graph) {
    if (!graph) {
        printf("ERROR : Invalid graph provided\n");
        return;
    }
    printf("Node count : %d\n\n", graph->size);
    for (int i = 0; i < graph->size; ++i) {
        printf("Node %d :", i);
        printf(" (d+%d) ", Graph_getPositiveValency(graph, i));
        printf(" (d-%d) ", Graph_getNegativeValency(graph, i));
        for (int j = 0; j < graph->size; ++j) {
            printf("[%f] ", graph->arcs[i][j]);
        }
        printf("\n");
    }
}

void Graph_set(Graph *graph, int u, int v, float weight) {
    if (!graph) {
        printf("ERROR : Invalid graph provided\n");
        return;
    }
    if (u < 0 || v < 0 || u >= graph->size || v >= graph->size) {
        printf("ERROR : Out of bounds value\n");
        return;
    }
    if (weight < 0.0f) {
        if (graph->arcs[u][v] < 0.0f)
            return;
        graph->arcs[u][v] = -1.f;
        graph->positiveValencies[u]--;
        graph->negativeValencies[v]--;
        return;
    }
    if (graph->arcs[u][v] >= 0.0f) {
        graph->arcs[u][v] = weight;
        return;
    }
    graph->arcs[u][v] = weight;
    graph->positiveValencies[u]++;
    graph->negativeValencies[v]++;
}

float Graph_get(Graph *graph, int u, int v) {
    if (!graph)
        printf("ERROR : Invalid graph provided\n");
    if (u < 0 || v < 0 || u >= graph->size || v >= graph->size) {
        printf("ERROR : Out of bounds value\n");
        return -1.f;
    }
    return graph->arcs[u][v];
}

int Graph_getPositiveValency(Graph *graph, int u) {
    if (!graph)
        printf("ERROR : Invalid graph provided\n");
    if (u < 0 || u >= graph->size) {
        printf("ERROR : Out of bounds value\n");
        return 0;
    }
    return graph->positiveValencies[u];
}

int Graph_getNegativeValency(Graph *graph, int u) {
    if (!graph)
        printf("ERROR : Invalid graph provided\n");
    if (u < 0 || u >= graph->size) {
        printf("ERROR : Out of bounds value\n");
        return 0;
    }
    return graph->negativeValencies[u];
}

Arc *Graph_getPredecessors(Graph *graph, int u, int *size) {
    if (!graph)
        printf("ERROR : Invalid graph provided\n");
    if (u < 0 || u >= graph->size) {
        printf("ERROR : Out of bounds value\n");
        return NULL;
    }
    *size = Graph_getNegativeValency(graph, u);
    if (!(*size))
        return NULL;
    Arc *predecessors = calloc(*size, sizeof(Arc));
    int idx = 0;
    for (int i = 0; i < graph->size; ++i) {
        if (graph->arcs[i][u] >= 0) {
            predecessors[idx].weight = graph->arcs[i][u];
            predecessors[idx].source = i;
            predecessors[idx].target = u;
            idx++;
        }
    }
    return predecessors;
}

Arc *Graph_getSuccessors(Graph *graph, int u, int *size) {
    if (!graph)
        printf("ERROR : Invalid graph provided\n");
    if (u < 0 || u >= graph->size) {
        printf("ERROR : Out of bounds value\n");
        return NULL;
    }
    *size = Graph_getPositiveValency(graph, u);
    if (!(*size))
        return NULL;
    Arc *successors = calloc(*size, sizeof(Arc));
    int idx = 0;
    for (int i = 0; i < graph->size; ++i) {
        if (graph->arcs[u][i] >= 0) {
            successors[idx].weight = graph->arcs[u][i];
            successors[idx].source = u;
            successors[idx].target = i;
            idx++;
        }
    }
    return successors;
}

#endif