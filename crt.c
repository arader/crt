#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <math.h>
#include <string.h>

#include "vector.h"
#include "objects.h"

#define EPSILON     0.001f
#define TRACE_DEPTH 6

void render( int, int, color***, primitive*, int, int );
color* trace( ray*, primitive*, int, float, float*, int );
intersection* intersect( ray*, primitive* );
float calc_shade( primitive*, vector*, vector*, vector*, primitive*, int );
void usage(void);

void render( int width, int height, color ***image, primitive *scene,
        int aa_level, int shadows ) {
    float world_left, world_right, world_top, world_bot;
    float delta_x, delta_y;
    float screen_x, screen_y;
    float tmpf;
    int x, y;
    int aa_x, aa_y;
    int aa_root;
    vector origin, dir;
    ray r;
    color *tmpc;

    world_left = -(4.0f * ((float)width / (float)height) );
    world_right = -world_left;
    world_top = 4.0f;
    world_bot = -4.0f;

    /* The amount to shift for each pixel */
    delta_x = (world_right - world_left) / width;
    delta_y = (world_bot - world_top) / height;

    origin.x = 0.0f;
    origin.y = 0.0f;
    origin.z = -5.0f;

    aa_root = (int)sqrt( aa_level );

    screen_y = world_top;
    for( y = 0; y < height; y++ ) {
        screen_x = world_left;
        for( x = 0; x < width; x++ ) {
            for( aa_x = 0; aa_x < aa_root; aa_x++ ) {
                for( aa_y = 0; aa_y < aa_root; aa_y++ ) {
                    dir.x = screen_x + delta_x * aa_x / (float)aa_root;
                    dir.y = screen_y + delta_y * aa_y / (float)aa_root;
                    dir.z = 0.0f;

                    vect_sub( &dir, &origin );
                    vect_normalize( &dir );

                    r.origin = &origin;
                    r.dir = &dir;

                    tmpc = trace( &r, scene, 0, 1.0f, &tmpf, shadows );
                    if( image[y][x] == NULL ) {
                        image[y][x] = tmpc;
                    }
                    else {
                        vect_add( image[y][x], tmpc );
                        free( tmpc );
                    }
                }
            }

            vect_multf( image[y][x], 1.0f / aa_level );

            screen_x += delta_x;
        }
        screen_y += delta_y;
    }
}

color* trace( ray *aray, primitive *scene, int depth, float refr, float *dist,
        int shadows ){
    intersection *isect;
    primitive *prim, *iter;
    vector isect_pt, pn, lv, ln, tmpv1, tmpv2;
    ray tmpr;
    float tmpf1, tmpf2, tmpf3, shade;
    color *tmpc;
    color *c = (color*)malloc( sizeof( color ) );
    if( c == NULL ) {
        fprintf( stderr, "*** error: could not allocate color memory\n" );
        exit( 1 );
    }

    c->x = 0.0f;
    c->y = 0.0f;
    c->z = 0.0f;

    isect = intersect( aray, scene );

    if( isect == NULL ) {
        return c;
    }

    *dist = isect->dist;

    prim = isect->prim;

    if( prim->is_light ) {
        vect_copy( c, &(isect->prim->mat.col) );
        free( isect );
        return c;
    }

    vect_copy( &isect_pt, aray->dir );
    vect_multf( &isect_pt, isect->dist );
    vect_add( &isect_pt, aray->origin );

    prim->normal( prim, &isect_pt, &pn );

    iter = scene;
    while( iter != NULL ) {
        if( iter->is_light ) {
            vect_copy( &lv, &iter->center );
            vect_sub( &lv, &isect_pt );

            vect_copy( &ln, &lv );
            vect_normalize( &ln );

            shade = calc_shade( iter, &isect_pt, &lv, &ln, scene, shadows );

            if( shade > 0.0f ) {
                /* determine the diffuse component */
                tmpf1 = prim->mat.diffuse;
                if( tmpf1 > 0.0f ) {
                    tmpf2 = vect_dot( &pn, &ln );
                    if( tmpf2 > 0.0f ) {
                        tmpf1 *= tmpf2 * shade;
                        vect_copy( &tmpv1, &prim->mat.col );
                        vect_mult( &tmpv1, &iter->mat.col );
                        vect_multf( &tmpv1, tmpf1 );
                        vect_add( c, &tmpv1 );
                    }
                }

                /* determine the specular component */
                tmpf1 = prim->mat.specular;
                if( tmpf1 > 0.0f ) {
                    vect_copy( &tmpv1, &pn );
                    vect_copy( &tmpv2, &ln );
                    tmpf2 = 2.0f * vect_dot( &ln, &pn );
                    vect_multf( &tmpv1, tmpf2 );
                    vect_sub( &tmpv2, &tmpv1 );
                    tmpf2 = vect_dot( aray->dir, &tmpv2 );
                    if( tmpf2 > 0.0f ) {
                        tmpf1 = powf( tmpf2, 20.0f ) * tmpf1 * shade;
                        vect_copy( &tmpv1, &iter->mat.col );
                        vect_multf( &tmpv1, tmpf1 );
                        vect_add( c, &tmpv1 );
                    }
                }
            }
        }

        iter = iter->next;
    }

    /* calculate reflection */
    if( prim->mat.refl > 0.0f && depth < TRACE_DEPTH ) {
        vect_copy( &tmpv1, &pn );
        vect_multf( &tmpv1, 2.0f * vect_dot( &pn, aray->dir ) );
        vect_copy( &tmpv2, aray->dir );
        vect_sub( &tmpv2, &tmpv1 );

        vect_copy( &tmpv1, &tmpv2 );
        vect_multf( &tmpv1, EPSILON );
        vect_add( &tmpv1, &isect_pt );

        tmpr.origin = &tmpv1;
        tmpr.dir = &tmpv2;

        tmpc = trace( &tmpr, scene, depth + 1, refr, &tmpf1, shadows );
        vect_multf( tmpc, prim->mat.refl );

        vect_copy( &tmpv1, &prim->mat.col );
        vect_mult( &tmpv1, tmpc );
        vect_add( c, &tmpv1 );

        free( tmpc );
    }

    /* calculate refraction */
    if( prim->mat.is_refr && depth < TRACE_DEPTH ) {
        vect_copy( &tmpv1, &pn );
        if( isect->inside ) {
            vect_multf( &tmpv1, -1.0f );
        }

        tmpf1 = refr / prim->mat.refr;
        tmpf2 = -( vect_dot( &tmpv1, aray->dir ) );
        tmpf3 = 1.0f - tmpf1 * tmpf1 * (1.0f - tmpf2 * tmpf2);
        if( tmpf3 > 0.0f ) {
            vect_copy( &tmpv2, aray->dir );
            vect_multf( &tmpv2, tmpf1 );
            vect_multf( &tmpv1, tmpf1 * tmpf2 - sqrtf( tmpf3 ) );
            vect_add( &tmpv1, &tmpv2 );

            vect_copy( &tmpv2, &tmpv1 );
            vect_multf( &tmpv2, EPSILON );
            vect_add( &tmpv2, &isect_pt );

            tmpr.origin = &tmpv2;
            tmpr.dir = &tmpv1;

            tmpc = trace( &tmpr, scene, depth + 1, refr, &tmpf1, shadows );

            vect_copy( &tmpv1, &prim->mat.col );
            vect_multf( &tmpv1, prim->mat.absorb * tmpf1 );

            tmpv2.x = expf( -tmpv1.x );
            tmpv2.y = expf( -tmpv1.y );
            tmpv2.z = expf( -tmpv1.z );

            vect_mult( tmpc, &tmpv2 );
            vect_add( c, tmpc );

            free( tmpc );
        }
    }

    free( isect );

    if( c->x > 1.0f ) {
        c->x = 1.0f;
    }
    else if( c->x < 0.0f ) {
        c->x = 0.0f;
    }

    if( c->y > 1.0f ) {
        c->y = 1.0f;
    }
    else if( c->y < 0.0f ) {
        c->y = 0.0f;
    }

    if( c->z > 1.0f ) {
        c->z = 1.0f;
    }
    else if( c->z < 0.0f ) {
        c->z = 0.0f;
    }

    return c;
}

float calc_shade( primitive *light, vector *isect_pt, vector *lv, vector *ln,
        primitive *scene, int shadows ) {
    float shade = 1.0f;
    float l_dist;
    int i;
    ray r;
    vector o, dir;
    intersection *isect;
    primitive *iter;

    if( light->is_light == AREA_LIGHT && shadows > 1 && light->grid != NULL ) {
        for( i = 0; i < shadows; i++ ) {
            dir.x = light->grid[(i&63)*3] +
                ((float)rand()/RAND_MAX)*light->dx;
            dir.y = light->grid[(i&63)*3+1] +
                ((float)rand()/RAND_MAX)*light->dy;
            dir.z = light->grid[(i&63)*3+2] +
                ((float)rand()/RAND_MAX)*light->dz;

            vect_sub( &dir, isect_pt );
            l_dist = vect_length( &dir );
            vect_multf( &dir, 1.0f / l_dist );

            vect_copy( &o, &dir );
            vect_multf( &o, EPSILON );
            vect_add( &o, isect_pt );

            r.origin = &o;
            r.dir = &dir;

            for( iter = scene; iter != NULL; iter = iter->next ) {
                isect = iter->intersect( iter, &r );

                if( isect != NULL &&
                        !iter->is_light && isect->dist < l_dist ) {
                    shade -= 1.0f / shadows;
                    free( isect );
                    break;
                }

                free( isect );
            }
        }
    }
    else {
        /* setup the ray */
        vect_copy( &o, ln );
        vect_multf( &o, EPSILON );
        vect_add( &o, isect_pt );

        r.origin = &o;
        r.dir = ln;

        l_dist = vect_length( lv );

        for( iter = scene; iter != NULL; iter = iter->next ) {
            isect = iter->intersect( iter, &r );

            if( isect != NULL && !iter->is_light && isect->dist < l_dist ) {
                shade = 0.0f;
                free( isect );
                break;
            }

            free( isect );
        }
    }

    return shade;
}

intersection* intersect( ray *r, primitive *scene ) {
    intersection *isect = NULL;
    intersection *closest = NULL;
    primitive *p = scene;

    while( p != NULL ) {
        if( (isect = p->intersect( p, r )) ) {
            if( closest == NULL || isect->dist < closest->dist ) {
                free( closest );
                closest = isect;
            }
            else {
                free( isect );
            }
        }

        p = p->next;
    }

    return closest;
}

void usage( void ) {
    printf("crt usage: crt [options] <scene> <output>\n" );
    printf("options:\n" );
    printf("\t--input <in plugin>   - specifies the method of scene input\n" );
    printf("\t                        defaults to 'xml.so'\n" );
    printf("\t--output <out plugin> - specifies the fild output method\n" );
    printf("\t                        defaults to 'bmp.so'\n" );
    printf("\t--aa <aa level>       - specifies the level of anti-aliasing\n");
    printf("\t                        use, defaults to '1', values should\n");
    printf("\t                        be squares (such as 4, 16, 36, etc)\n" );
}

int main( int argc, char **argv ) {
    void *dl_out = NULL;
    void *dl_in = NULL;
    char *dl_error = NULL;
    primitive *scene = NULL;
    primitive *tmp = NULL;

    int aa_level = 1;
    int shadows = 1;
    char *input = NULL;
    char *output = NULL;

    int (*so_write)(int,int,color***,char*);
    primitive* (*so_load)(char*);

    int width = 512;
    int height = 512;

    color ***img = NULL;
    int i,j;

    printf( "CRT - version 0.2.0\n" );

    if( argc < 3 ) {
        usage();
        exit( 1 );
    }

    for( i = 1; i < argc - 2; i++ ) {
        if( strcmp( argv[i], "--help" ) == 0 ) {
            usage();
            exit( 0 );
        }
        else if( strcmp( argv[i], "--input" ) == 0 ) {
            if( i == argc - 3 || (strncmp( argv[i+1], "--", 2 ) == 0) ) {
                fprintf( stderr, "*** crt: missing argument to \"--input\"\n" );
                exit( 1 );
            }
            else {
                input = argv[i+1];
            }
        }
        else if( strcmp( argv[i], "--output" ) == 0 ) {
            if( i == argc - 3 || (strncmp( argv[i+1], "--", 2 ) == 0) ) {
                fprintf( stderr,"*** crt: missing argument to \"--output\"\n" );
                exit( 1 );
            }
            else {
                output = argv[i+1];
            }
        }
        else if( strcmp( argv[i], "--aa" ) == 0 ) {
            if( i == argc - 3 || (strncmp( argv[i+1], "--", 2 ) == 0) ) {
                fprintf( stderr, "*** crt: missing argument to \"--aa\"\n" );
                exit( 1 );
            }
            else {
                aa_level = atoi( argv[i+1] );
            }
        }
        else if( strcmp( argv[i], "--shadows" ) == 0 ) {
            if( i == argc - 3 || (strncmp( argv[i+1], "--", 2 ) == 0) ) {
                fprintf(stderr, "*** crt: missing argument to \"--shadow\"\n");
                exit( 1 );
            }
            else {
                shadows = atoi( argv[i+1] );
            }
        }
        else if( strcmp( argv[i], "--width" ) == 0 ) {
            if( i == argc - 3 || (strncmp( argv[i+1], "--", 2 ) == 0) ) {
                fprintf( stderr, "*** crt: missing argument to \"--width\"\n" );
            }
            else {
                width = atoi( argv[i+1] );
            }
        }
        else if( strcmp( argv[i], "--height" ) == 0 ) {
            if( i == argc - 3 || (strncmp( argv[i+1], "--", 2 ) == 0) ) {
                fprintf( stderr,"*** crt: missing argument to \"--height\"\n" );
            }
            else {
                height = atoi( argv[i+1] );
            }
        }
    }

    /* setup the output plugin */
    if( output == NULL ) {
        dl_out = dlopen( "./outputs/bmp.so", RTLD_LAZY );
    }
    else {
        dl_out = dlopen( output, RTLD_LAZY );
    }
    if( !dl_out ) {
        fprintf( stderr, "*** error: %s\n", dlerror() );
        exit( 1 );
    }

    dlerror();      /* clear error code */
    so_write = (int (*)(int,int,color***,char*))dlsym( dl_out, "write_img" );
    if( (dl_error = dlerror()) != NULL ) {
        fprintf( stderr, "*** error: %s\n", dl_error );
        exit( 1 );
    }

    /* setup the input plugin */
    if( input == NULL ) {
        dl_in = dlopen( "./inputs/xml.so", RTLD_LAZY );
    }
    else {
        dl_in = dlopen( input, RTLD_LAZY );
    }
    if( !dl_in ) {
        fprintf( stderr, "*** error: %s\n", dlerror() );
        exit( 1 );
    }
    
    dlerror();
    so_load = (primitive* (*)(char*))dlsym( dl_in, "load_scene" );
    if( (dl_error = dlerror()) != NULL ) {
        fprintf( stderr, "*** error: %s\n", dl_error );
        exit( 1 );
    }

    /* setup the image's memory */
    img = malloc( height * sizeof(color**) );
    if( img == NULL ) {
        fprintf( stderr, "*** error: could not allocate image memory\n" );
        exit( 1 );
    }

    for( i = 0; i < height; i++ ) {
        img[i] = calloc( width, sizeof( color* ) );
        if( img[i] == NULL ) {
            fprintf( stderr, "*** error: could not allocate image memory\n" );
            exit( 1 );
        }
    }

    scene = so_load( argv[argc-2] );

    render( width, height, img, scene, aa_level, shadows );

    so_write( width, height, img, argv[argc-1] );

    for( i = 0; i < height; i++ ) {
        for( j = 0; j < width; j++ ) {
            /* free the color's memory */
            free( img[i][j] );
        }

        /* free the line's memory */
        free( img[i] );
    }

    /* free the image memory */
    free( img );

    /* free the scene object's memory */

    while( scene != NULL ) {
        free( scene->data );
        if( scene->grid != NULL ) {
            free( scene->grid );
        }

        tmp = scene->next;
        free( scene );

        scene = tmp;
    }

    dlclose( dl_out );
    dlclose( dl_in );

    return 0;
}
