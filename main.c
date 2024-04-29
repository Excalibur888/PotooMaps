#include "settings.h"
#include "graph.h"
#include "uniStr.h"
#include "cJSON.h"
#include "dict.h"
#include "poi.h"

/// @brief Print le message correspondant à l'ouverture du fichier.
/// @param err 1 si l'ouverture à échoué,
/// 0 sinon.
/// @param path Le chemin vers le fichier ouvert.
/// @author Arthur
INLINE void err_open(bool err, char *path) {
    if (err) {
        printf("\033[0;31m");
        printf("ERROR: Can't open %s file.\n", path);
        printf("\033[0m");
    } else {
        printf("\033[0;32m");
        printf("INFO: %s file opened successfully.\n", path);
        printf("\033[0m");
    }
}

/// @brief Print le message correspondant au parsing du fichier.
/// @param err 1 si le parsing à échoué,
/// 0 sinon.
/// @param path Le chemin vers le fichier ouvert.
/// @author Arthur
INLINE void err_parse(bool err, char *path) {
    if (err) {
        printf("\033[0;31m");
        printf("ERROR: Can't parse %s file.\n", path);
        printf("\033[0m");
    } else {
        printf("\033[0;32m");
        printf("INFO: %s file successfully parsed.\n", path);
        printf("\033[0m");
    }
}


/// @brief Parse le fichier des communes.
/// @param input Le fichier csv des communes.
/// @param sep Le séparateur.
/// @param count Le nombre total de communes.
/// @return Retourne un dictionnaire INSEE - Structure commune, si le parsing à fonctionné,
/// NULL sinon.
/// @author Arthur
Dict *municipalitiesParse(FILE *input, UniStr *sep, int *count) {
    *count = -1;
    int subCount = 0, end = 0;
    char *string = calloc(MAX_MUNICIPALITIES, sizeof(char));

    // On crée un dictionnaire.
    Dict *dict = Dict_create();

    // Tant qu'il y a des lignes dans le fichier :
    while (fgets(string, sizeof(char) * MAX_MUNICIPALITIES, input)) {
        char cat[6];
        cat[0] = '\0';
        // On décode la ligne.
        UniStr *decoded = UniStr_decodeU8(string, sizeof(string) * MAX_MUNICIPALITIES);
        // On divise la ligne en chaines de caractères selon le séparateur.
        UniStr **subs = UniStr_split(decoded, sep, &subCount);
        // On crée une structure commune et on la remplie.
        Municipalities *municipality = calloc(1, sizeof(Municipalities));
        municipality->id = *count;
        char *len = UniStr_encodeU8(subs[0]);
        municipality->code_commune_INSEE = calloc(6, sizeof(char));
        municipality->nom_commune_postal = calloc(1024, sizeof(char));
        if (strlen(len) == 4) {
            strcat(cat, "0");
            strcat(cat, len);
            strcpy(municipality->code_commune_INSEE, cat);
        } else {
            char *cci = UniStr_encodeU8(subs[0]);
            strcpy(municipality->code_commune_INSEE, cci);
            free(cci);
        }
        free(len);
        char * ncp = UniStr_encodeU8(subs[1]);
        strcpy(municipality->nom_commune_postal, ncp);
        free(ncp);
        municipality->latitude = UniStr_getDouble(subs[5], 0, &end);
        municipality->longitude = UniStr_getDouble(subs[6], 0, &end);
        if (*count > -1)
            // Si la ligne nous intéresse, on ajoute la commune au dictionnaire et on l'associe à son numéro INSEE :
            Dict_insert(dict, municipality->code_commune_INSEE, municipality);
        else
            // Sinon, on détruit la commune :
            free(municipality);
        (*count)++;

        UniStr_destroyArray(subs, subCount);
        UniStr_destroy(decoded);
    }
    free(string);
    // Si le dictionnaire existe, on le retourne :
    if (dict)
        return dict;
    // Sinon on retourne null :
    return NULL;
}


/// @brief Parse le fichier des communes adjacentes.
/// @param input Le fichier csv des communes adjacentes.
/// @param sep Le séparateur.
/// @param count Le nombre total de communes.
/// @param dict Le dictionnaire des communes.
/// @return Retourne un graph des Communes - Communes adjacentes, si le parsing à fonctionné,
/// NULL Sinon.
/// @author Arthur
Graph *adjMunicipalitiesParse(FILE *input, UniStr *sep, int count, Dict *dict) {
    int subCount = 0, childCount = 0, cnt = -1, source, target;
    char *string = calloc(MAX_MUNICIPALITIES, sizeof(char));

    // On crée un graphe.
    Graph *graph = Graph_create(count);

    // Tant qu'il y a des lignes dans le fichier :
    while (fgets(string, sizeof(char) * MAX_MUNICIPALITIES, input)) {

        // On décode la ligne.
        UniStr *decoded = UniStr_decodeU8(string, sizeof(string) * MAX_MUNICIPALITIES);
        // On divise la ligne en chaines des caractères selon le séparateur.
        UniStr **subs = UniStr_split(decoded, sep, &subCount);

        UniStr *pipeSeparator = UniStr_decodeU8("|", -1);

        // Si la ligne nous intéresse :
        if (cnt > -1) {
            // On récupère la commune source.
            char *key = UniStr_encodeU8(subs[0]);
            Municipalities *sourceMunicipality = Dict_get(dict, key);
            if (sourceMunicipality) {
                source = sourceMunicipality->id;
                // On divise la chaine de caractères en numéros INSEE selon le séparateur.
                UniStr **children = UniStr_split(subs[3], pipeSeparator, &childCount);
                // Pour chaque numéro INSEE :
                for (int i = 0; i < childCount; ++i) {
                    // On récupère la commune associée au numéro INSEE.
                    char *keyChild = UniStr_encodeU8(children[i]);
                    Municipalities *targetMunicipality = Dict_get(dict, keyChild);
                    if (targetMunicipality) {
                        target = targetMunicipality->id;
                        // On crée l'arc entre la commune source et la commune destination
                        Graph_set(graph, source, target, 0);
                    }
                    free(keyChild);
                }
                UniStr_destroyArray(children, childCount);
            }
            free(key);
        }
        cnt++;
        UniStr_destroyArray(subs, subCount);
        UniStr_destroy(decoded);
        UniStr_destroy(pipeSeparator);
    }
    free(string);

    // Si le graph existe, on le retourne :
    if (graph)
        return graph;
    // Sinon on retourne null :
    return NULL;
}


/// @brief Parse le fichier des POI.
/// @param input Le fichier csv des poi.
/// @param sep Le séparateur.
/// @param count Le nombre total de poi.
/// @return Retourne un dictionnaire des poi, si le parsing à fonctionné,
/// NULL Sinon.
/// @author Arthur
Dict *poiParse(FILE *input, UniStr *sep, int *count) {
    *count = -1;
    int subCount = 0, end = 0;
    char *string = calloc(MAX_MUNICIPALITIES, sizeof(char));

    // On crée un dictionnaire.
    Dict *dict = Dict_create();

    // Tant qu'il y a des lignes dans le fichier :
    while (fgets(string, sizeof(char) * MAX_MUNICIPALITIES, input)) {
        // On décode la ligne.
        UniStr *decoded = UniStr_decodeU8(string, sizeof(string) * MAX_MUNICIPALITIES);
        // On divise la ligne en chaines de caractères selon le séparateur.
        UniStr **subs = UniStr_split(decoded, sep, &subCount);

        // Si le POI nous intéresse :
        char *amenity = UniStr_encodeU8(subs[17]);
        if (!strcmp(amenity, "bar") || !strcmp(amenity, "pub") ||
            !strcmp(amenity, "cafe")) {
            // On crée une structure POI et on la remplie.
            Poi *poi = calloc(1, sizeof(Poi));
            poi->name = UniStr_encodeU8(subs[4]);
            poi->longitude = UniStr_getDouble(subs[1], 0, &end);
            poi->latitude = UniStr_getDouble(subs[2], 0, &end);
            Dict_insert(dict, poi->name, poi);
            (*count)++;
        }
        free(amenity);
        UniStr_destroyArray(subs, subCount);
        UniStr_destroy(decoded);
    }
    free(string);

    // Si le dictionnaire existe, on le retourne :
    if (dict)
        return dict;
    // Sinon on retourne null :
    return NULL;
}


/// @brief Renvoi un tableau associant à chaque indice de case l'ID de la commune et ayant pour contenu la structure de la commune.
/// @param count Le nombre total de communes.
/// @param dict Le dictionnaire des communes.
/// @return Retourne un tableau ID - Commune.
/// @author Arthur
Municipalities **linkIdToStruct(int count, Dict *dict) {
    Municipalities **municipalities = calloc(count, sizeof(Municipalities));
    DictIter *iter = calloc(1, sizeof(DictIter));
    Dict_getIterator(dict, iter);
    // Tant qu'il y a des communes dans le dictionnaire :
    while (DictIter_hasNext(iter)) {
        KVPair *pair = DictIter_next(iter);
        if (!pair) continue;
        Municipalities *value = pair->value;
        // On les ajoute dans le tableau.
        if (value) {
            municipalities[value->id] = value;
        }
    }
    free(iter);
    // On retourne le tableau.
    return municipalities;
}


/// @brief Cherche la commune à partir de son numéro INSEE ou de son nom.
/// La fonction n'est pas sensible à la casse.
/// @param dict Le dictionnaire des communes.
/// @param input La chaine de caractères à trouver.
/// @return Renvoie la structure si elle est trouvé,
/// NULL sinon.
/// @author Arthur
Municipalities *getMunicipality(Dict *dict, char *input) {
    // Si l'entrée correspond à un numéro INSEE :
    if (isdigit(input[0])) {
        // Si le numéro INSEE n'est pas au bon format, on le reformate correctement :
        if (strlen(input) == 4) {
            char cat[6];
            cat[0] = '\0';
            strcat(cat, "0");
            strcat(cat, input);
            input = cat;
        }
        // On cherche le numéro INSEE dans le dictionnaire et on retourne la commune correspondante.
        return Dict_get(dict, input);
        // Sinon, on parcourt le dictionnaire à la recherche du nom de la commune :
    } else {
        DictIter *iter = calloc(1, sizeof(DictIter));
        Dict_getIterator(dict, iter);
        while (DictIter_hasNext(iter)) {
            KVPair *pair = DictIter_next(iter);
            if (!pair)
                continue;
            Municipalities *value = pair->value;
            if (value) {
                // On Reformate la chaine de caractère entrée pour la faire correspondre à celles dans le dictionnaire.
                UniStr *decoded = UniStr_decodeU8(input, -1);
                char *caseSensitive = UniStr_encodeAscii(decoded);
                for (int i = 0; i < strlen(caseSensitive); i++) {
                    caseSensitive[i] = toupper((unsigned char) caseSensitive[i]);
                    if (caseSensitive[i] == '-')
                        caseSensitive[i] = ' ';
                }
                if (!strcmp(value->nom_commune_postal, caseSensitive)) {
                    UniStr_destroy(decoded);
                    free(caseSensitive);
                    free(iter);
                    // On retourne la commune correspondante.
                    return value;
                }
                UniStr_destroy(decoded);
                free(caseSensitive);
            }
        }
        free(iter);
    }
    // Sinon on retourne null :
    return NULL;
}


/// @brief Renvoi les communes adjacentes à celle donnée.
/// @param municipality La structure commune.
/// @param graph Le graph des communes adjacentes.
/// @param array Le tableau ID - Commune.
/// @param size Pointeur vers la taille du tableau de retour.
/// @return Retourne un tableau de structures communes adjacentes à celle donnée.
/// @author Arthur
Municipalities **getAdj(Municipalities *municipality, Graph *graph, Municipalities **array, int *size) {
    // Récupère les successeurs de la commune dans le graph.
    Arc *adjacentsNodes = Graph_getSuccessors(graph, municipality->id, size);
    Municipalities **adjacents = calloc(*size, sizeof(Municipalities));
    // Pour chaque successeur, on associe la commune correspondante :
    for (int i = 0; i < *size; ++i) {
        adjacents[i] = array[adjacentsNodes[i].target];
    }
    // Retourne le tableau de communes adjacentes.
    return adjacents;
}

/// @brief Renvoi distance entre deux communes avec latitudes et longitudes.
/// @param latitude1 La latitude de la premiere commune.
/// @param longitude1 La longitude de la premiere commune.
/// @param latitude2 La latitude de la deuxième commune.
/// @param longitude2 La longitude de la deuxième commune.
/// @return Retourne la distance en float entre deux communes.
/// @author Adrien
double computeDistance(double latitude1, double longitude1, double latitude2, double longitude2) {
    double phi1 = latitude1 * M_PI / 180.0;
    double phi2 = latitude2 * M_PI / 180.0;
    double delta_phi = (latitude2 - latitude1) * M_PI / 180.0;
    double delta_lambda = (longitude2 - longitude1) * M_PI / 180.0;

    double a = sin(delta_phi / 2) * sin(delta_phi / 2) +
               cos(phi1) * cos(phi2) * sin(delta_lambda / 2) * sin(delta_lambda / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    double d = 6371 * c;
    return d;
}

/// @brief Applique la distance entre deux communes sur les arcs selon la latitude et longitude
/// @param municipalitiesGraph Le graphe.
/// @param municipalitiesList Le tableau des municipalités.
/// @author Adrien
void municipalityWeight(Graph *municipalitiesGraph, Municipalities **municipalitiesList, int count) {
    for (int i = 0; i < count; i++) {
        int size;
        Arc *arc = Graph_getSuccessors(municipalitiesGraph, i, &size);
        for (int j = 0; j < Graph_getPositiveValency(municipalitiesGraph, i); j++) {
            Graph_set(municipalitiesGraph, i, arc[j].target,
                      computeDistance(municipalitiesList[i]->latitude, municipalitiesList[i]->longitude,
                                      municipalitiesList[arc[j].target]->latitude,
                                      municipalitiesList[arc[j].target]->longitude));
        }
        free(arc);
    }
}

/// @brief Crée un objet cJSON à partir d'un template.
/// @return Retourne l'objet cJSON créé.
/// @author Arthur
cJSON *jsonTemplateParse() {
    char *template = "{\n"
                     "  \"type\": \"FeatureCollection\",\n"
                     "  \"features\": [\n"
                     "    {\n"
                     "      \"type\": \"Feature\",\n"
                     "      \"geometry\":\n"
                     "      {\n"
                     "        \"type\": \"LineString\",\n"
                     "        \"coordinates\": [\n"
                     "        ]\n"
                     "      }\n"
                     "    }\n"
                     "  ]\n"
                     "}";
    return cJSON_Parse(template);
};


/// @brief Crée un tableau de coordonnées cJSON à partir de deux doubles.
/// @return Retourne le tableau de doubles cJSON créé.
/// @author Arthur
cJSON *jsonCoosGenerate(double d1, double d2) {
    double *coos = calloc(2, sizeof(double));
    coos[0] = d1;
    coos[1] = d2;
    cJSON *coordinates = cJSON_CreateDoubleArray(coos, 2);
    free(coos);
    return coordinates;
}

/// @brief Écrit le fichier GéoJSON.
/// @param path La structure path contenant le chemin.
/// @param file Le pointeur sur le fichier à écrire.
/// @param arr  Le tableau ID - Commune.
/// @author Arthur
void jsonGenerate(Path *path, FILE *file, Municipalities **arr) {
    cJSON *json = jsonTemplateParse();
    cJSON *feature = NULL;
    cJSON *features = cJSON_GetObjectItemCaseSensitive(json, "features");
    cJSON_ArrayForEach(feature, features) {
        cJSON *geometry = cJSON_GetObjectItemCaseSensitive(feature, "geometry");
        cJSON *coordinates = cJSON_GetObjectItem(geometry, "coordinates");

        IntListNode *sentinel = &(path->list->sentinel);
        IntListNode *curr = sentinel->next;
        while (curr != sentinel) {
            double co1 = arr[curr->value]->longitude;
            double co2 = arr[curr->value]->latitude;
            cJSON *coordinate = jsonCoosGenerate(co1, co2);
            cJSON_AddItemToArray(coordinates, coordinate);

            curr = curr->next;
        }
    }
    char *out = cJSON_Print(json);
    fprintf(file, "%s", out);
    free(out);
    cJSON_Delete(json);
}


void municipalitiesDict_destroy(Dict *municipalitiesDict) {
    DictIter *iter = calloc(1, sizeof(DictIter));
    Dict_getIterator(municipalitiesDict, iter);
    while (DictIter_hasNext(iter)) {
        KVPair *pair = DictIter_next(iter);
        if (!pair) continue;
        Municipalities *value = pair->value;
        free(value->code_commune_INSEE);
        free(value->nom_commune_postal);
        free(value);
    }
    free(iter);
    Dict_destroy(municipalitiesDict);
}


void poi_destroy(Dict *poiDict) {
    DictIter *iter = calloc(1, sizeof(DictIter));
    Dict_getIterator(poiDict, iter);
    while (DictIter_hasNext(iter)) {
        KVPair *pair = DictIter_next(iter);
        if (!pair) continue;
        Poi *value = pair->value;
        free(value->name);
        free(value);
    }
    free(iter);
    Dict_destroy(poiDict);
}


void grid_destroy(GridCell **grid) {
    for (int i = 0; i < GRID_WIDTH; ++i) {
        for (int j = 0; j < GRID_HEIGHT; ++j) {
            IntList_destroy(grid[i][j].poiList);
        }
        free(grid[i]);
    }
    free(grid);
}


/// @authors Arthur et Adrien
int main(int argc, char *argv[]) {
    // Déclaration des variables.
    int municipalitiesCount, poiCount;
    char *input_start = calloc(1024, sizeof(char));
    char *input_end = calloc(1024, sizeof(char));
    char *path_municipalities = "./Data/communes.csv";
    char *path_adjacentMunicipalities = "./Data/communes_adjacentes.csv";
    char *path_poi = "./Data/poi.csv";
    char *path_map = "./Data/map.geojson";
    UniStr *commaSeparator = UniStr_decodeU8(u8",", -1);
    UniStr *tabulationSeparator = UniStr_decodeU8(u8"\t", -1);


    // Ouverture des fichiers.
    FILE *input_municipalities = fopen(path_municipalities, "r");
    if (!input_municipalities) {
        err_open(1, path_municipalities);
        return EXIT_FAILURE;
    }
    err_open(0, path_municipalities);

    FILE *input_adjacentMunicipalities = fopen(path_adjacentMunicipalities, "r");
    if (!input_adjacentMunicipalities) {
        err_open(1, path_adjacentMunicipalities);
        return EXIT_FAILURE;
    }
    err_open(0, path_adjacentMunicipalities);

    FILE *input_poi = fopen(path_poi, "r");
    if (!input_poi) {
        err_open(1, path_poi);
        return EXIT_FAILURE;
    }
    err_open(0, path_poi);

    FILE *output_map = fopen(path_map, "w+");
    if (!output_map) {
        err_open(1, path_map);
        return EXIT_FAILURE;
    }
    err_open(0, path_map);


    // Lecture fichiers des communes et création tableau de struct communes.
    Dict *municipalitiesDict = municipalitiesParse(input_municipalities, commaSeparator,
                                                   &municipalitiesCount);
    if (!municipalitiesDict) {
        err_parse(1, path_municipalities);
        return EXIT_FAILURE;
    }
    err_parse(0, path_municipalities);


    // Génération du graph à partir de la lecture du fichier des communes adjacentes.
    Graph *municipalitiesGraph = adjMunicipalitiesParse(input_adjacentMunicipalities, commaSeparator,
                                                        municipalitiesCount,
                                                        municipalitiesDict);
    if (!municipalitiesGraph) {
        err_parse(1, path_adjacentMunicipalities);
        return EXIT_FAILURE;
    }
    err_parse(0, path_adjacentMunicipalities);


    // Génération du graph à partir de la lecture du fichier des communes adjacentes.
    Dict *poiDict = poiParse(input_poi, tabulationSeparator, &poiCount);
    if (!poiDict) {
        err_parse(1, path_poi);
        return EXIT_FAILURE;
    }
    err_parse(0, path_poi);


    // Génération du tableau liant l'ID de la commune à sa structure.
    Municipalities **municipalitiesList = linkIdToStruct(municipalitiesCount, municipalitiesDict);
    if (!municipalitiesList) {
        printf("\033[0;31m");
        printf("ERROR: Can't link structs to IDs.\n");
        printf("\033[0m");
        return EXIT_FAILURE;
    }
    printf("\033[0;32m");
    printf("INFO: Structs and IDs successfully linked.\n");
    printf("\033[0m");


    // Vérification de la ville de départ.
    if (argc < 2) {
        printf("\033[0;31m");
        printf("\nERROR: You have not specified a departure city.\n"
               "Enter your departure city: ");
        printf("\033[0m");
        scanf("%s", input_start);
    } else {
        stpcpy(input_start, argv[1]);
    }
    Municipalities *start = getMunicipality(municipalitiesDict, input_start);
    while (!start) {
        printf("\033[0;31m");
        printf("\nERROR: Departure city not found.\n"
               "Try again: ");
        printf("\033[0m");
        scanf("%s", input_start);
        start = getMunicipality(municipalitiesDict, input_start);
    }
    printf("\033[0;32m");
    printf("INFO: Departure city found.\n");
    printf("\033[0m");


    // Vérification de la ville d'arrivée.
    if (argc < 3) {
        printf("\033[0;31m");
        printf("\nERROR: You have not specified an arrival city.\n"
               "Enter your city of arrival: ");
        printf("\033[0m");
        scanf("%s", input_end);
    } else {
        stpcpy(input_end, argv[2]);
    }
    Municipalities *end = getMunicipality(municipalitiesDict, input_end);
    while (!end) {
        printf("\033[0;31m");
        printf("\nERROR: Arrival city not found.\n"
               "Try again: ");
        printf("\033[0m");
        scanf("%s", input_end);
        end = getMunicipality(municipalitiesDict, input_end);
    }
    printf("\033[0;32m");
    printf("INFO: Arrival city found.\n");
    printf("\033[0m");


    municipalityWeight(municipalitiesGraph, municipalitiesList, municipalitiesCount);

    // Creation de la grille de la France.
    GridCell **grid = createGrid();

    DictIter *iter = calloc(1, sizeof(DictIter));
    Dict_getIterator(poiDict, iter);
    // Tant qu'il y a des poi dans le dictionnaire :
    while (DictIter_hasNext(iter)) {
        KVPair *pair = DictIter_next(iter);
        if (!pair) continue;
        Poi *value = pair->value;
        // Appelle de la fonction pour ajouter les poi dans la grille.
        addPOI(grid, value);
    }

    // Passage par toutes les communes.
    for (int i = 0; i < municipalitiesCount; i++) {
        if (!municipalitiesList[i] || municipalitiesList[i]->latitude == 0 || municipalitiesList[i]->longitude == 0 ||
            municipalitiesList[i]->longitude < -6 || municipalitiesList[i]->longitude > 10 ||
            municipalitiesList[i]->latitude < 40 || municipalitiesList[i]->latitude > 53)
            continue;
        int compteur = 0;
        // Calcule de la position de la commune.
        int x = floor((municipalitiesList[i]->latitude - GRID_MIN_LAT) / CELL_SIZE);
        int y = floor((municipalitiesList[i]->longitude - GRID_MIN_LON) / CELL_SIZE);

        // Récupération des bars.
        for (int a = x - RAYON; a < x + RAYON; a++) {
            for (int b = y - RAYON; b < y + RAYON; b++) {
                if (a <= 0 || a >= 1000 || b <= 0 || b >= 1000)
                    continue;
                compteur += grid[a][b].poiList->nodeCount;
            }
        }


        int predecessorsSize;
        Arc *arc = Graph_getPredecessors(municipalitiesGraph, i, &predecessorsSize);

        for (int m = 0; m < predecessorsSize; m++) {
            if (compteur > 0) {
                float value = Graph_get(municipalitiesGraph, arc[m].source, i) / (compteur + 1);
                Graph_set(municipalitiesGraph, arc[m].source, i, value);
            }
        }
        free(arc);
    }



    // Algorithme plus court chemin.
    Path *path = Graph_shortestPath(municipalitiesGraph, start->id, end->id);



    // Calcule du nombre de bars selon les communes où passe le trajet.
    int compteurTotal = 0;
    int compteurPoi = 0;
    int checkPoi = 0;
    int *communesCheck = calloc(path->list->nodeCount, sizeof(int));
    int index = 0;

    IntList *list = path->list;
    IntListNode *sentinel = &(list->sentinel);
    IntListNode *curr = sentinel->next;
    while (curr != sentinel->prev) {
        communesCheck[index] = curr->value;
        index++;
        curr = curr->next;
    }
    if (curr != sentinel) {
        communesCheck[index] = curr->value;
        index++;
    }

    // Passage par toutes les communes.
    for (int i = 0; i < path->list->nodeCount; i++) {
        if (!municipalitiesList[i] || municipalitiesList[i]->latitude == 0 || municipalitiesList[i]->longitude == 0 ||
            municipalitiesList[i]->longitude < -6 || municipalitiesList[i]->longitude > 10 ||
            municipalitiesList[i]->latitude < 40 || municipalitiesList[i]->latitude > 53) {
            continue;
        }
        // Calcule de la position de la commune.
        int x = floor((municipalitiesList[i]->latitude - GRID_MIN_LAT) / CELL_SIZE);
        int y = floor((municipalitiesList[i]->longitude - GRID_MIN_LON) / CELL_SIZE);

        checkPoi = 0;

        // Calcule dans le périmètre de la commune du nombre de bar.
        for (int a = x - RAYON; a < x + RAYON; a++) {
            for (int b = y - RAYON; b < y + RAYON; b++) {
                if (a <= 0 || a >= 1000 || b <= 0 || b >= 1000)
                    continue;
                compteurTotal += grid[a][b].poiList->nodeCount;
                if (grid[a][b].poiList->nodeCount == 0) {
                    checkPoi++;
                }
            }
        }
        if (checkPoi > 0) {
            compteurPoi++;
        }
    }


    printf("Nombre total de bars: %d \n", compteurTotal);
    printf("Nombre total de communes de passage: %d \n", path->list->nodeCount);
    printf("Communes de passage avec bars: %d \n", compteurPoi);
    printf("Communes de passage sans bars: %d ", path->list->nodeCount - compteurPoi);



    // Génération du fichier geojson.
    jsonGenerate(path, output_map, municipalitiesList);
    printf("\033[0;32m");
    printf("\nINFO: Output file successfully generated at %s.\n", path_map);
    printf("\033[0m");


    // Libération de la mémoire.
    free(iter);
    free(input_end);
    free(input_start);
    free(communesCheck);
    fclose(input_poi);
    fclose(output_map);
    fclose(input_municipalities);
    fclose(input_adjacentMunicipalities);
    Path_destroy(path);
    poi_destroy(poiDict);
    municipalitiesDict_destroy(municipalitiesDict);
    free(municipalitiesList);
    grid_destroy(grid);
    Graph_destroy(municipalitiesGraph);
    UniStr_destroy(commaSeparator);
    UniStr_destroy(tabulationSeparator);


    return EXIT_SUCCESS;
}