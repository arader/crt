#include <math.h>
#include <stdlib.h>
#include "objects.h"
#include "vector.h"

#define EPSILON 0.001f

intersection* box_isect( primitive *prim, ray *r ) {
    intersection *isect = NULL;
    box_data *data = (box_data*)prim->data;
    float dist[] = {-1.0f,-1.0f,-1.0f,-1.0f,-1.0f,-1.0f};
    vector ip[6];
    vector v2;
    float rc;

    char hit = 0;
    float distance = 0.0f;
    int index = 0;
    char inside = 0;
    int i;

    vect_copy( &v2, &data->point );
    vect_add( &v2, &data->size );

    if( r->dir->x != 0.0f ) {
        rc = 1.0f / r->dir->x;
        dist[0] = (data->point.x - r->origin->x) * rc;
        dist[3] = (v2.x - r->origin->x) * rc;
    }
    if( r->dir->y != 0.0f ) {
        rc = 1.0f / r->dir->y;
        dist[1] = (data->point.y - r->origin->y) * rc;
        dist[4] = (v2.y - r->origin->y) * rc; 
    }
    if( r->dir->z != 0.0f ) {
        rc = 1.0f / r->dir->z;
        dist[2] = (data->point.z - r->origin->z) * rc;
        dist[5] = (v2.z - r->origin->z) * rc;
    }

    for( i = 0; i < 6; i++ ) {
        if( dist[i] > 0.0f ) {
            vect_copy( &ip[i], r->dir );
            vect_multf( &ip[i], dist[i] );
            vect_add( &ip[i], r->origin );

            if( ip[i].x > (data->point.x - EPSILON) &&
                ip[i].x < (v2.x + EPSILON) &&
                ip[i].y > (data->point.y - EPSILON) &&
                ip[i].y < (v2.y + EPSILON) &&
                ip[i].z > (data->point.z - EPSILON) &&
                ip[i].z < (v2.z + EPSILON) ) {

                if( !hit || dist[i] < distance ) {
                    hit = 1;
                    distance = dist[i];
                    index = i;
                }
            }
        }
    }

    if( hit ) {
        if( prim->mat.is_refr ) {
            switch( index ) {
                case 0:
                    inside = r->dir->x < 0.0f ? 1 : 0;
                    break;
                case 1:
                    inside = r->dir->y < 0.0f ? 1 : 0;
                    break;
                case 2:
                    inside = r->dir->z < 0.0f ? 1 : 0;
                    break;
                case 3:
                    inside = r->dir->x > 0.0f ? 1 : 0;
                    break;
                case 4:
                    inside = r->dir->y > 0.0f ? 1 : 0;
                    break;
                case 5:
                    inside = r->dir->z > 0.0f ? 1 : 0;
                    break;
                default:
                    inside = 0;
            }
        }

        isect = (intersection*)malloc( sizeof( intersection ) );
        isect->prim = prim;
        isect->dist = distance;
        isect->inside = inside;

        return isect;
    }

    return NULL;
}

void box_normal( primitive *box, vector *isect, vector *ret ) {
    box_data *data = (box_data*)box->data;
    float dist[6];
    int best = 0, i;
    float best_dist;

    dist[0] = fabsf( isect->x - data->point.x );
    dist[1] = fabsf( isect->x - (data->point.x + data->size.x) );
    dist[2] = fabsf( isect->y - data->point.y );
    dist[3] = fabsf( isect->y - (data->point.y + data->size.y) );
    dist[4] = fabsf( isect->z - data->point.z );
    dist[5] = fabsf( isect->z - (data->point.z + data->size.z) );

    best_dist = dist[0];
    for( i = 1; i < 6; i++ ) {
        if( dist[i] < best_dist ) {
            best_dist = dist[i];
            best = i;
        }
    }

    if( best == 0 ) {
        ret->x = -1.0f;
        ret->y = 0.0f;
        ret->z = 0.0f;
    }
    else if( best == 1 ) {
        ret->x = 1.0f;
        ret->y = 0.0f;
        ret->z = 0.0f;
    }
    else if( best == 2 ) {
        ret->x = 0.0f;
        ret->y = -1.0f;
        ret->z = 0.0f;
    }
    else if( best == 3 ) {
        ret->x = 0.0f;
        ret->y = 1.0f;
        ret->z = 0.0f;
    }
    else if( best == 4 ) {
        ret->x = 0.0f;
        ret->y = 0.0f;
        ret->z = -1.0f;
    }
    else {
        ret->x = 0.0f;
        ret->y = 0.0f;
        ret->z = 1.0f;
    }
}

intersection* sphere_isect( primitive *prim, ray *r ) {
    intersection *isect;
    vector v;
    float b;
    float det;
    float dist = 0.0f;
    int hit = 0;
    int inside = 0;
    float i1, i2;

    sphere_data *sphere = (sphere_data*)prim->data;

    vect_copy( &v, r->origin );
    vect_sub( &v, &sphere->center );
    
    b = -(vect_dot( &v, r->dir ) );
    det = (b*b) - vect_dot( &v, &v ) + (sphere->radius * sphere->radius);

    if( det > 0 ) {
        det = sqrtf( det );
        i1 = b - det;
        i2 = b + det;
        if( i2 > 0 ) {
            if( i1 < 0 ) {
                dist = i2;
                hit = 1;
                inside = 1;
            }
            else {
                dist = i1;
                hit = 1;
                inside = 0;
            }
        }
    }

    if( hit ) {
        isect = (intersection*)malloc( sizeof(intersection) );
        isect->prim = prim;
        isect->dist = dist;
        isect->inside = inside;

        return isect;
    }
    else {
        return NULL;
    }
}

void sphere_normal( primitive *sphere, vector *v, vector *ret ) {
    vect_copy( ret, v );
    vect_sub( ret, &sphere->center );
    vect_multf( ret, 1.0f / ((sphere_data*)sphere->data)->radius );
}

intersection* plane_isect( primitive *prim, ray *r ) {
    plane_data *data = (plane_data*)prim->data;
    float d = vect_dot( &data->normal, r->dir );
    float t;
    intersection *isect = NULL;

    if( d < 0.0f ) {
        t = -( vect_dot( &data->normal, r->origin ) + data->dist ) / d;

        if( t > 0.0f ) {
            isect = (intersection*)malloc( sizeof( intersection ) );
            isect->prim = prim;
            isect->dist = t;
            isect->inside = 0.0f;
            return isect;
        }
    }

    return NULL;
}

void plane_normal( primitive *plane, vector *v, vector *ret ) {
    vect_copy( ret, &((plane_data*)plane->data)->normal );
}
