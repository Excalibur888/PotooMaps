//
// Created by DUMOND on 03/05/2023.
//

typedef struct sMunicipalities Municipalities;
typedef struct sPoi Poi;

struct sMunicipalities
{
    int id;
    char *code_commune_INSEE;
    char *nom_commune_postal;
    double latitude;
    double longitude;
};

struct sPoi
{
    char *name;
    double latitude;
    double longitude;
};
