#ifndef __VECTOR__H__
#define __VECTOR__H__

typedef struct {
    float x;
    float y;
    float z;
} vector;

typedef struct {
    vector *origin;
    vector *dir;
} ray;

typedef vector color;

/*
 * vect_add - computes a = a + b
 */
void vect_add( vector *a, vector *b );

/*
 * vect_addf - computes a = a + f
 */
void vect_addf( vector *a, float f );

/*
 * vect_sub - computes a = a - b
 */
void vect_sub( vector *a, vector *b );

/*
 * vect_subf - computes a = a - f
 */
void vect_subf( vector *a, float f );

/*
 * vect_mult - computes a = a * b
 */
void vect_mult( vector *a, vector *b );

/*
 * vect_multf -computes a = a * f
 */
void vect_multf( vector *a, float f );

/*
 * vect_length - returns the length of the vector
 */
float vect_length( vector *v );

/*
 * vect_normalize - normalizes a vector
 */
void vect_normalize( vector *v );

/*
 * vect_dot - computes the dot product of a and b
 */
float vect_dot( vector *a, vector *b );

/*
 * vect_copy - copies the values from one vector
 * to another
 */
void vect_copy( vector *dest, vector *src );

#endif
