#include "graph.h"

#ifndef _GRAPH_MAT

typedef struct sGraph Graph;
typedef struct sGraphNode GraphNode;
typedef struct sArcList ArcList;

struct sGraph {
    /// @brief Tableau contenant les noeuds du graphe.
    GraphNode *nodes;
    /// @brief Nombre de noeuds du graphe.
    int size;
};

/// @brief Structure représentant un noeud d'un graphe.
struct sGraphNode {
    /// @brief Identifiant du noeud.
    /// Compris entre 0 et la taille du graphe moins 1.
    int id;
    /// @brief Degré entrant du noeud.
    int negativeValency;
    /// @brief Degré sortant du noeud.
    int positiveValency;
    /// @brief Liste des arcs sortants du noeud.
    ArcList *arcList;
};

/// @brief Structure représentant une liste simplement chaînée des arcs sortants d'un noeud.
struct sArcList {
    /// @brief Pointeur vers l'élément suivant de la liste.
    /// Vaut NULL s'il s'agit du dernier élément.
    ArcList *next;
    /// @brief Arc associé au noeud de liste.
    Arc arc;
};

Graph *Graph_create(int size) {
    if (size < 1) return NULL;
    Graph *graph = calloc(1, sizeof(Graph));
    graph->nodes = calloc(size, sizeof(GraphNode));
    for (int i = 0; i < size; ++i) {
        graph->nodes[i].id = i;
    }
    graph->size = size;
    return graph;
}

void Graph_destroyRec(ArcList *arc) {
    if (!arc->next) {
        free(arc);
        return;
    }
    return Graph_destroyRec(arc->next);
}

void Graph_destroy(Graph *graph) {
    assert(graph);
    for (int i = 0; i < graph->size; ++i) {
        if (graph->nodes[i].arcList)
            Graph_destroyRec(graph->nodes[i].arcList);
    }
    free(graph->nodes);
    free(graph);
}

int Graph_size(Graph *graph) {
    assert(graph);
    return graph->size;
}

void Graph_print(Graph *graph) {
    if (!graph) {
        printf("ERROR: Invalid graph provided\n");
        return;
    }
    printf("Node count : %d\n\n", graph->size);
    ArcList *current = calloc(1, sizeof(ArcList));
    for (int i = 0; i < graph->size; ++i) {
        printf("Node %d", i);
        printf(" (d+%d) ", Graph_getPositiveValency(graph, i));
        printf(" (d-%d) ", Graph_getNegativeValency(graph, i));
        current = graph->nodes[i].arcList;
        while (current) {
            printf("[%f, %d, %d]", current->arc.weight, current->arc.source, current->arc.target);
            current = current->next;
        }
        /*for (int j = 0; j < graph->size; ++j) {
            printf("[%f] ", graph->nodes->arcList[j].arc.weight);
        }*/
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

    //On place current sur la tête de liste.
    ArcList *current = graph->nodes[u].arcList;
    ArcList *tmp = NULL;

    //Si on souhaite supprimer le nœud.
    if (weight == -1.f) {
        //Si la tête de liste existe et que c'est le nœud que l'on recherche,
        // on le supprime, on actualise les valency et on retourne.
        if (current && current->arc.target == v) {
            graph->nodes[u].arcList = current->next;
            free(current);
            graph->nodes[u].positiveValency--;
            graph->nodes[v].negativeValency--;
            return;
        }

        // Tant que current existe, on avance dans la liste.
        while (current) {
            //Si le suivant est le nœud que l'on recherche, on le supprime,
            // on actualise les valency et on retourne.
            if (current->next && current->next->arc.target == v) {
                tmp = current->next;
                current->next = current->next->next;
                free(tmp);
                graph->nodes[u].positiveValency--;
                graph->nodes[v].negativeValency--;
                return;
            }
            current = current->next;
        }
        return;
    }
    // Tant que current et son suivant existent on avance dans la liste.
    while (current && current->next) {
        //Si la cible de current est plus grande que celle du nœud à insérer, on quitte.
        if (current->arc.target >= v) {
            break;
        }
        //Si la cible du suivant de current est plus grande que celle du nœud à insérer, on quitte.
        if (current->next && current->next->arc.target > v)
            break;
        current = current->next;
    }

    //Si la cible de current est égale à celle du nœud à insérer, on remplace juste le poids
    // et on retourne.
    if (current && current->arc.target == v) {
        current->arc.weight = weight;
        return;
    }

    //On crée le nœud à insérer
    ArcList *arc = calloc(1, sizeof(ArcList));
    arc->arc.source = u;
    arc->arc.target = v;
    arc->arc.weight = weight;

    //Si la liste est vite, on insère le nœud en tête, on actualise les valency et on retourne.
    if (!current) {
        graph->nodes[u].arcList = arc;
        graph->nodes[u].positiveValency++;
        graph->nodes[v].negativeValency++;
        return;
    }

    //Si le premier nœud est plus grand que celui à insérer, on insère le nœud en tête,
    // on actualise les valency et on retourne.
    if (current->arc.target > v){
        arc->next = current;
        graph->nodes[u].arcList = arc;
        graph->nodes[u].positiveValency++;
        graph->nodes[v].negativeValency++;
        return;
    }

    //Sinon on insère le nœud entre current et le suivant de current et on actualise les valency.
    tmp = current->next;
    current->next = arc;
    arc->next = tmp;
    graph->nodes[u].positiveValency++;
    graph->nodes[v].negativeValency++;
}

float Graph_get(Graph *graph, int u, int v) {
    if (!graph) {
        printf("ERROR : Invalid graph provided\n");
        return -1.f;
    }
    if (u < 0 || v < 0 || u >= graph->size || v >= graph->size) {
        printf("ERROR : Out of bounds value\n");
        return -1.f;
    }
    ArcList *current = graph->nodes[u].arcList;
    while (current) {
        if (current->arc.target == v)
            return current->arc.weight;
        current = current->next;
    }
    return -1.f;
}

int Graph_getPositiveValency(Graph *graph, int u) {
    if (!graph) {
        printf("ERROR : Invalid graph provided\n");
        return -1;
    }
    if (u < 0 || u >= graph->size) {
        printf("ERROR : Out of bounds value\n");
        return -1;
    }
    return graph->nodes[u].positiveValency;
}

int Graph_getNegativeValency(Graph *graph, int u) {
    if (!graph) {
        printf("ERROR : Invalid graph provided\n");
        return -1;
    }
    if (u < 0 || u >= graph->size) {
        printf("ERROR : Out of bounds value\n");
        return -1;
    }
    return graph->nodes[u].negativeValency;
}

Arc *Graph_getPredecessors(Graph *graph, int u, int *size) {
    if (!graph) {
        printf("ERROR : Invalid graph provided\n");
        return NULL;
    }
    if (u < 0 || u >= graph->size) {
        printf("ERROR : Out of bounds value\n");
        return NULL;
    }
    *size = 0;
    if (!Graph_getNegativeValency(graph, u))
        return NULL;
    Arc *arc = calloc(Graph_getNegativeValency(graph, u), sizeof(Arc));
    ArcList *current = NULL;
    for (int i = 0; i < graph->size; ++i) {
        current = graph->nodes[i].arcList;
        while (current) {
            if (current->arc.target == u)
                arc[(*size)++] = current->arc;
            current = current->next;
        }
    }
    return arc;
}

Arc *Graph_getSuccessors(Graph *graph, int u, int *size) {
    if (!graph) {
        printf("ERROR : Invalid graph provided\n");
        return NULL;
    }
    if (u < 0 || u >= graph->size) {
        printf("ERROR : Out of bounds value\n");
        return NULL;
    }
    *size = 0;
    if (!Graph_getPositiveValency(graph, u))
        return NULL;
    Arc *arc = calloc(Graph_getPositiveValency(graph, u), sizeof(Arc));
    ArcList *current = NULL;
    current = graph->nodes[u].arcList;
    while (current) {
        arc[(*size)++] = current->arc;
        current = current->next;
    }
    return arc;
}

#endif