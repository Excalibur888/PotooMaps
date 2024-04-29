#include "graph.h"

Graph *Graph_load(char *filename) {
    FILE *input = fopen(filename, "r");
    if (!input) {
        printf("ERROR: Invalid filepath provided");
        return NULL;
    }
    int nodeCount, arcCount, source, target;
    float weight;

    fscanf(input, "%d ", &nodeCount);
    fscanf(input, "%d ", &arcCount);
    Graph *graph = Graph_create(nodeCount);
    for (int i = 0; i < arcCount; ++i) {
        fscanf(input, "%d ", &source);
        fscanf(input, "%d ", &target);
        fscanf(input, "%f ", &weight);
        Graph_set(graph, source, target, weight);
    }
    return graph;
}

/// @brief Sous-fonction de Graph_dfsPrint() pour les appels récursifs.
/// @param graph le graphe.
/// @param currID l'identifiant du noeud courant.
/// @param explored tableau de booléens indiquant pour chaque identifiant de
/// noeud s'il a été marqué comme atteint.
void Graph_dfsPrintRec(Graph *graph, int currID, bool *explored) {
    int size;
    explored[currID] = true;
    printf("%d-", currID);
    Arc * successors = Graph_getSuccessors(graph, currID, &size);
    for (int i = 0; i < size; ++i) {
        if (explored[successors[i].target])
            continue;
        Graph_dfsPrintRec(graph, successors[i].target, explored);
    }
}

void Graph_dfsPrint(Graph *graph, int start) {
    if (!graph) {
        printf("ERROR : Invalid graph provided\n");
        return;
    }
    int size = Graph_size(graph);
    bool *explored = calloc(size, sizeof(bool));
    Graph_dfsPrintRec(graph, start ,explored);
    printf("\n");
}

void Graph_bfsPrint(Graph *graph, int start) {
    if (!graph) {
        printf("ERROR : Invalid graph provided\n");
        return;
    }
    IntList *list = IntList_create();
    int size = Graph_size(graph);
    bool *explored = calloc(size, sizeof(bool));
    IntList_enqueue(list, start);
    explored[start] = true;
    while (!IntList_isEmpty(list)){
        int node = IntList_dequeue(list);
        printf("%d-", node);
        Arc *successors = Graph_getSuccessors(graph, node, &size);
        for (int i = 0; i < size; ++i) {
            if (explored[successors[i].target])
                continue;
            IntList_enqueue(list, successors[i].target);
            explored[successors[i].target] = true;
        }
    }
    printf("\n");
}

IntTree *Graph_spanningTreeRec(Graph *graph, int current, bool *explored) {
    int size;
    explored[current] = true;
    IntTree *tree = IntTree_create(current);
    Arc *successors = Graph_getSuccessors(graph, current, &size);
    for (int i = 0; i < size; ++i) {
        if (explored[successors[i].target])
            continue;
        IntTree_addChild(tree, Graph_spanningTreeRec(graph, successors[i].target, explored));
    }
    return tree;
}

IntTree *Graph_spanningTree(Graph *graph, int start) {
    if (!graph) {
        printf("ERROR : Invalid graph provided\n");
        return NULL;
    }
    int size = Graph_size(graph);
    bool *explored = calloc(size, sizeof(bool));
    return Graph_spanningTreeRec(graph, start ,explored);
}



Path* Graph_shortestPath(Graph* graph, int start, int end)
{
    int size = Graph_size(graph);

    int* predecessors = calloc(size, sizeof(int));
    float* distances = calloc(size, sizeof(float));

    Graph_dijkstra(graph, start, end, predecessors, distances);

    Path* path = Graph_dijkstraGetPath(predecessors, distances, end);

    free(predecessors);
    free(distances);

    return path;
}

void Graph_dijkstra(Graph* graph, int start, int end, int* predecessors, float* distances)
{
    int size = Graph_size(graph);
    bool* expl = calloc(size, sizeof(bool));
    for (int i = 0; i < size; i++)
    {
        predecessors[i] = -1;
        expl[i] = false;
        distances[i] = INFINITY;
    }
    distances[start] = 0;

    while (true)
    {
        int u = -1;
        float min = INFINITY;
        for (int i = 0; i < size; i++)
        {
            if (distances[i] < min && expl[i] == false)
            {
                u = i;
                min = distances[i];
            }
        }
        if (min == INFINITY)
        {
            break;
        }
        assert(u >= 0);
        expl[u] = true;


        if (u == end)
        {
            break;
        }

        int sizen;
        Arc* succ = Graph_getSuccessors(graph, u, &sizen);
        for (int i = 0; i < sizen; i++) {
            float poids = distances[u] + Graph_get(graph, u, succ[i].target);
            int v = succ[i].target;
            if (expl[v] != true) {
                if (poids < distances[v]) {
                    distances[v] = poids;
                    predecessors[v] = u;
                }
            }
        }
        free(succ);
    }
    free(expl);
}

Path* Graph_dijkstraGetPath(int* predecessors, float* distances, int end)
{
    if (predecessors == NULL || distances == NULL)
        return NULL;

    Path* path = Path_create(end);
    int current = end;

    while (predecessors[current] != -1)
    {
        current = predecessors[current];
        Path_insertFirst(path, current, 0.0f);
    }
    path->distance = distances[end];
    return path;
}
