#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <expat.h>

#include "../objects.h"
#include "../vector.h"

#define BUF_SIZE    4096

typedef struct {
    XML_Parser parser;
    primitive *prim;
    primitive *head;
} parse_data;

static void parse_xyz( float*, const char** );
static void parse_light( primitive*, const char** );
static float parse_attrf( const char*, const char**, float );
static const char* parse_attrs( const char*, const char** );

static void parse_xyz( float *xyz, const char **attr ) {
    int i;

    for( i = 0; attr[i]; i += 2 ) {
        if( strcmp( attr[i], "x" ) == 0 ) {
            xyz[0] = (float)atof( attr[i+1] );
        }
        else if( strcmp( attr[i], "y" ) == 0 ) {
            xyz[1] = (float)atof( attr[i+1] );
        }
        else if( strcmp( attr[i], "z" ) == 0 ) {
            xyz[2] = (float)atof( attr[i+1] );
        }
    }
}

static void parse_light( primitive *p, const char **attr ) {
    const char *light = parse_attrs( "light", attr );
    int i;
    float y,z;

    if( light == NULL ) {
        p->is_light = NOT_LIGHT;
    }
    else if( strcmp( light, "point" ) == 0 ) {
        p->is_light = POINT_LIGHT;
    }
    else {
        p->is_light = AREA_LIGHT;

        if( p->type == BOX ) {
            p->grid = (float*)malloc( 192 * sizeof( float ) );

            p->dx = ((box_data*)p->data)->size.x * 0.25f;
            p->dy = ((box_data*)p->data)->size.y * 0.25f;
            p->dz = ((box_data*)p->data)->size.z * 0.25f;

            for( i = 0; i < 64; i++ ) {
                p->grid[ i*3 ] = ((box_data*)p->data)->point.x + (i%4)*p->dx;

                if( i % 4 == 0 ) {
                    z = ((box_data*)p->data)->point.z + ((float)(i>=16?i%16:i)/4.0f)*p->dz;
                    if( i % 16 == 0 ) {
                        y = ((box_data*)p->data)->point.y + ((float)i/16.0f)*p->dy;
                    }
                }

                p->grid[ i*3+1 ] = y;
                p->grid[ i*3+2 ] = z;
            }

            /*
            for( i = 0; i < 16; i++ ) {
                p->grid[i*2] = ((box_data*)p->data)->point.x + (i%4)*p->dx;
                if( i % 4 == 0 ) {
                    z = ((box_data*)p->data)->point.z + ((float)i/4.0f)*p->dz;
                }

                p->grid[i*2+1] = z;
            }
            */
        }
    }
}


static float parse_attrf( const char *name, const char **attr, float def ) {
    int i;

    for( i = 0; attr[i]; i += 2 ) {
        if( strcmp( attr[i], name ) == 0 ) {
            return( (float)atof( attr[i+1] ) );
        }
    }

    return def;
}

static const char* parse_attrs( const char *name, const char **attr ) {
    int i;

    for( i = 0; attr[i]; i += 2 ) {
        if( strcmp( attr[i], name ) == 0 ) {
            return( attr[i+1] );
        }
    }

    return NULL;
}

static char parse_attrb( const char *name, const char **attr, int def ) {
    const char *truefalse = parse_attrs( name, attr );

    if( truefalse != NULL && strcmp( truefalse, "true" ) == 0 ) {
        return 1;
    }
    else {
        return 0;
    }

    return def;
}

void start( void *data, const char *el, const char **attr ) {
    parse_data *pdata = (parse_data*)data;
    float xyz[3];

    if( pdata == NULL ) {
        fprintf( stderr, "*** xml: parse data is NULL\n" );
        exit( 1 );
    }

    if( strcmp( el, "scene" ) == 0 ) {
    }
    else if( pdata->prim == NULL ) {
        pdata->prim = (primitive*)calloc( 1, sizeof( primitive ) );

        if( strcmp( el, "sphere" ) == 0 ) {
            pdata->prim->type = SPHERE;

            pdata->prim->intersect = &sphere_isect;
            pdata->prim->normal = &sphere_normal;

            pdata->prim->data = (sphere_data*)malloc( sizeof( sphere_data ) );

            parse_light( pdata->prim, attr );
            parse_xyz( xyz, attr );
            ((sphere_data*)pdata->prim->data)->radius = parse_attrf(
                "radius", attr, 0.0f );

            ((sphere_data*)pdata->prim->data)->center.x = xyz[0];
            ((sphere_data*)pdata->prim->data)->center.y = xyz[1];
            ((sphere_data*)pdata->prim->data)->center.z = xyz[2];

            vect_copy( &pdata->prim->center,
                    &((sphere_data*)pdata->prim->data)->center );
            
            /* set some defaults */
            pdata->prim->mat.col.x = 1.0f;
            pdata->prim->mat.col.y = 1.0f;
            pdata->prim->mat.col.z = 1.0f;

            pdata->prim->mat.diffuse = 1.0f;
            pdata->prim->mat.specular = 0.0f;
            pdata->prim->mat.refl = 0.0f;
            pdata->prim->mat.is_refr = 0;
        }
        else if( strcmp( el, "plane" ) == 0 ) {
            pdata->prim->type = PLANE;

            pdata->prim->intersect = &plane_isect;
            pdata->prim->normal = &plane_normal;

            pdata->prim->data = (plane_data*)malloc( sizeof( plane_data ) );

            parse_light( pdata->prim, attr );
            parse_xyz( xyz, attr );

            ((plane_data*)pdata->prim->data)->normal.x = xyz[0];
            ((plane_data*)pdata->prim->data)->normal.y = xyz[1];
            ((plane_data*)pdata->prim->data)->normal.z = xyz[2];
            ((plane_data*)pdata->prim->data)->dist = parse_attrf(
                "distance", attr, 0.0f );

            vect_copy( &pdata->prim->center,
                    &((plane_data*)pdata->prim->data)->normal );

            /* set some defaults */
            pdata->prim->mat.col.x = 1.0f;
            pdata->prim->mat.col.y = 1.0f;
            pdata->prim->mat.col.z = 1.0f;

            pdata->prim->mat.diffuse = 1.0f;
            pdata->prim->mat.specular = 0.0f;
            pdata->prim->mat.refl = 0.0f;
            pdata->prim->mat.is_refr = 0;
        }
        else if( strcmp( el, "box" ) == 0 ) {
            pdata->prim->type = BOX;

            pdata->prim->intersect = &box_isect;
            pdata->prim->normal = &box_normal;

            pdata->prim->data = (box_data*)malloc( sizeof( box_data ) );

            parse_xyz( xyz, attr );

            ((box_data*)pdata->prim->data)->point.x = xyz[0];
            ((box_data*)pdata->prim->data)->point.y = xyz[1];
            ((box_data*)pdata->prim->data)->point.z = xyz[2];
            ((box_data*)pdata->prim->data)->size.x = parse_attrf(
                "width", attr, 0.0f );
            ((box_data*)pdata->prim->data)->size.y = parse_attrf(
                "height", attr, 0.0f );
            ((box_data*)pdata->prim->data)->size.z = parse_attrf(
                "depth", attr, 0.0f );

            pdata->prim->center.x = xyz[0] +
                (0.5f * ((box_data*)pdata->prim->data)->size.x);
            pdata->prim->center.y = xyz[1] +
                (0.5f * ((box_data*)pdata->prim->data)->size.y);
            pdata->prim->center.z = xyz[2] +
                (0.5f * ((box_data*)pdata->prim->data)->size.z);

            parse_light( pdata->prim, attr );

            /* set some defaults */
            pdata->prim->mat.col.x = 1.0f;
            pdata->prim->mat.col.y = 1.0f;
            pdata->prim->mat.col.z = 1.0f;

            pdata->prim->mat.diffuse = 1.0f;
            pdata->prim->mat.specular = 0.0f;
            pdata->prim->mat.refl = 0.0f;
            pdata->prim->mat.is_refr = 0;
        }
        else {
            fprintf( stderr,
                    "*** xml: unknown primitive type \"%s\" at line %lu\n",
                    el, XML_GetCurrentLineNumber( pdata->parser ) );
            exit( 1 );
        }

        pdata->prim->next = pdata->head;
        pdata->head = pdata->prim;
    }
    else if( strcmp( el, "material" ) == 0 ) {
        pdata->prim->mat.col.x = parse_attrf( "r", attr, 1.0f );
        pdata->prim->mat.col.y = parse_attrf( "g", attr, 1.0f );
        pdata->prim->mat.col.z = parse_attrf( "b", attr, 1.0f );

        pdata->prim->mat.diffuse = parse_attrf( "diffuse", attr, 1.0f );
        pdata->prim->mat.specular = parse_attrf( "specular", attr, 0.0f );
        pdata->prim->mat.refl = parse_attrf( "reflectivity", attr, 0.0f );
        pdata->prim->mat.is_refr = parse_attrb( "refractive", attr, 0 );
        if( pdata->prim->mat.is_refr ) {
            pdata->prim->mat.refr = parse_attrf( "refractivity", attr, 1.0f );
        }
    }
    else if( strcmp( el, "light" ) == 0 ) {
    }
}

void end( void *data, const char *el ) {
    parse_data *pdata = (parse_data*)data;
    if( strcmp( el, "scene" ) == 0 ) {
    }
    else {
        pdata->prim = NULL;
    }
}

primitive* load_scene( char *file ) {
    primitive *head = NULL;
    char buff[BUF_SIZE];
    FILE *ffile;
    parse_data *pdata = (parse_data*)calloc( 1, sizeof( parse_data ) );

    if( pdata == NULL ) {
        fprintf( stderr, "*** xml: could not allocate memory for parsing\n" );
        exit( 1 );
    }

    pdata->parser = XML_ParserCreate( NULL );
    
    if( pdata->parser == NULL ) {
        fprintf( stderr, "*** xml: could not allocate memory for parser\n" );
        exit( 1 );
    }

    ffile = fopen( file, "r" );
    if( ffile == NULL ) {
        fprintf( stderr, "*** xml: could not open %s for reading\n", file );
        exit( 1 );
    }

    XML_SetElementHandler( pdata->parser, start, end );
    XML_SetUserData( pdata->parser, pdata );

    for( ;; ) {
        int done;
        int len;

        len = fread( buff, 1, BUF_SIZE, ffile );
        if( ferror( ffile ) ) {
            fprintf( stderr, "*** xml: read error\n" );
            exit( 1 );
        }

        done = feof( ffile );

        if( ! XML_Parse( pdata->parser, buff, len, done ) ) {
            fprintf( stderr, "*** xml: parse error at line %lu:\n%s\n",
                    XML_GetCurrentLineNumber( pdata->parser ),
                    XML_ErrorString( XML_GetErrorCode( pdata->parser ) ) );
            exit( 1 );
        }

        if( done ) break;
    }

#if 0
    p = (primitive*)malloc( sizeof( primitive ) );

    p->type = PLANE;
    p->is_light = NOT_LIGHT;

    p->intersect = &plane_isect;
    p->normal = &plane_normal;

    p->data = (plane_data*)malloc( sizeof( plane_data ) );

    ((plane_data*)p->data)->normal.x = 0.0f;
    ((plane_data*)p->data)->normal.y = 1.0f;
    ((plane_data*)p->data)->normal.z = 0.0f;
    ((plane_data*)p->data)->dist = 2.0f;

    p->center.x = 0.0f;
    p->center.y = 0.0f;
    p->center.z = 0.0f;

    p->mat.diffuse = 1.0f;
    p->mat.specular = 0.0f;
    p->mat.refl = 0.0f;
    p->mat.is_refr = 0;

    p->mat.col.x = 0.46667f;
    p->mat.col.y = 0.52941f;
    p->mat.col.z = 0.60784f;

    p->next = head;
    head = p;
#endif

    fclose( ffile );
    XML_ParserFree( pdata->parser );

    head = pdata->head;
    free( pdata );

    return head;
}
