//
// Created by DUMOND on 14/05/2023.
//

#include "poi.h"

/// @brief Initialisation de la grille de POIs.
/// @author Adrien
/// @return Retourne le grille de IntList crée.
GridCell **createGrid() {
    // Allocation de la matrice.
    GridCell **grid = (GridCell **) calloc(GRID_WIDTH, sizeof(GridCell *));
    for (int i = 0; i < GRID_WIDTH; i++) {
        grid[i] = (GridCell *) calloc(GRID_HEIGHT, sizeof(GridCell));
        for (int j = 0; j < GRID_HEIGHT; j++) {
            // Allocation de la structure IntList dans chaque case.
            grid[i][j].poiList = IntList_create();
        }
    }
    return grid;
}


/// @brief Ajout des POIs dans la grille.
/// @author Adrien
/// @param graph le graphe.
/// @param poi  le tableau de POIs.
void addPOI(GridCell **grid, Poi *poi) {
    // Calcules des positions des POI.
    int i = floor((poi->latitude - GRID_MIN_LAT) / CELL_SIZE);
    int j = floor((poi->longitude - GRID_MIN_LON) / CELL_SIZE);
    // Si les indices rentrent dans la grille :
    if (i >= 0 && i < GRID_WIDTH && j >= 0 && j < GRID_HEIGHT) {
        // On récupère la liste de la cellule.
        IntList *poiList = grid[i][j].poiList;

        // Si la liste n'est pas initialisée, on l'initialise :
        if (!poiList) {
            poiList = IntList_create();
            grid[i][j].poiList = poiList;
        }
        IntList_insertLast(poiList, 1);
    }
}
