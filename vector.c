#include <math.h>
#include "vector.h"

void vect_add( vector *a, vector *b ) {
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

void vect_addf( vector *a, float f ) {
    a->x += f;
    a->y += f;
    a->z += f;
}

void vect_sub( vector *a, vector *b ) {
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

void vect_subf( vector *a, float f ) {
    a->x -= f;
    a->y -= f;
    a->z -= f;
}

void vect_mult( vector *a, vector *b ) {
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

void vect_multf( vector *a, float f ) {
    a->x *= f;
    a->y *= f;
    a->z *= f;
}

float vect_length( vector *v ) {
    return sqrtf( v->x*v->x + v->y*v->y + v->z*v->z );
}

void vect_normalize( vector *v ) {
    float length = 0.0f;
    float tmp = 0.0f;

    length = sqrtf( v->x * v->x + v->y * v->y + v->z * v->z );
    tmp = 1.0f / length;

    v->x *= tmp;
    v->y *= tmp;
    v->z *= tmp;
}

float vect_dot( vector *a, vector *b ) {
    return( a->x * b->x + a->y * b->y + a->z * b->z );
}

inline void vect_copy( vector *dest, vector *src ) {
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}
