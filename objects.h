#ifndef __OBJECTS__H__
#define __OBJECTS__H__

#define SPHERE  0
#define BOX     1
#define PLANE   2

#define NOT_LIGHT       0
#define POINT_LIGHT     1
#define AREA_LIGHT      2

#include "vector.h"

struct _isect;
typedef struct _isect intersection;
typedef struct _prim primitive;

typedef struct {
    color col;
    float diffuse, refl, refr, specular, absorb;
    int is_refr;
} material;

struct _prim {
    primitive *next;

    char type;
    char is_light;

    vector center;

    material mat;

    void *data;

    float *grid;
    float dx;
    float dy;
    float dz;

    intersection* (*intersect)( primitive*, ray* );
    void (*normal)( primitive*, vector*, vector* );
};

struct _isect {
    primitive *prim;
    float dist;
    char inside;
};

typedef struct {
    vector point;
    vector size;
} box_data;

typedef struct {
    vector center;
    float radius;
} sphere_data;

typedef struct {
    vector normal;
    float dist;
} plane_data;

intersection* box_isect( primitive *prim, ray *r );
void box_normal( primitive *box, vector *isect, vector *ret );
intersection* sphere_isect( primitive *prim, ray *r );
void sphere_normal( primitive *sphere, vector *isect, vector *ret );
intersection* plane_isect( primitive *prim, ray *r );
void plane_normal( primitive *plane, vector *isect, vector *ret );

#endif
