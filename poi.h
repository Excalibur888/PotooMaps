#include "municipalities.h"

#include "settings.h"
#include "intList.h"

#define GRID_WIDTH 1000
#define GRID_HEIGHT 1000
#define CELL_SIZE 0.015
#define GRID_MIN_LAT 41.337
#define GRID_MIN_LON -5.141
#define RAYON 10

typedef struct {
    IntList *poiList;
} GridCell;

GridCell **createGrid();

void addPOI(GridCell **grid, Poi *poi);
