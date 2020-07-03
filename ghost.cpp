#ifndef __WATCOMC__
#    define __export
#endif

#define GLOBALS
#include "div.h"

typedef unsigned char byte;

int ghost_auto = 0;

// Devuelve el color de la paleta m s pr¢ximo a los valores RGB dados
// OJO, probablemente puede optimizarse
byte find_color( int r, int g, int b )
{
    // Cada color es un punto en el cubo de color (0..63,0..63,0..63)
    // Tenemos un punto arbitrario (rgb) y queremos encontrar el color m s cercano.
    // La distancia es sqrt(dr*dr+dg*dg+db*db).
    // Podemos prescindir de la ra¡z cuadrada ya que s¢lo queremos comparar distancias.
    byte* pal = (byte*)_palette;
    int dist = 64 * 64 * 3;
    byte col = 0, c = 0;
    for( byte* p = pal; p < pal + ( 256 * 3 ); p += 3, ++c ) {
        int dr = r - int( p[0] );
        int dg = g - int( p[1] );
        int db = b - int( p[2] );
        int d = dr * dr + dg * dg + db * db;
        if( d < dist ) {
            dist = d;
            col = c;
            if( !d ) break;
        }
    }
    return col;
}

// Sustituye la tabla ghost de DIV por una calculada por nosotros,
// que suma los componentes RGB en vez de promediarlos
void make_ghost_additive()
{
    byte* gho = (byte*)ghost;
    byte* pal1 = (byte*)(_palette) + 3;
    int i;
    // Cualquier color + transparente = el mismo color
    for( i = 0; i < 256; ++i ) {
        gho[i] = i;
        gho[256 * i] = i;
    }
    for( i = 1; i < 256; ++i, pal1 += 3 ) {
        gho[i + 256 * i] = find_color( int( pal1[0] ) * 2, int( pal1[1] ) * 2, int( pal1[2] ) * 2 );
        byte* pal2 = (byte*)(_palette) + 3;
        for( int j = 1; j < i; ++j, pal2 += 3 ) {
            byte c = find_color( int( pal1[0] ) + pal2[0], int( pal1[1] ) + pal2[1], int( pal1[2] ) + pal2[2] );
            gho[i + 256 * j] = c;
            gho[j + 256 * i] = c;
        }
    }
}

// Regenera una tabla de transparencia est ndar
void make_ghost_standard()
{
    byte* gho = (byte*)ghost;
    byte* pal1 = (byte*)(_palette) + 3;
    int i;
    // Cualquier color + transparente = el mismo color
    for( i = 0; i < 256; ++i ) {
        gho[i] = i;
        gho[256 * i] = i;
    }
    for( i = 1; i < 256; ++i, pal1 += 3 ) {
        // Cualquier color + el mismo = el mismo color
        gho[i + 256 * i] = i;
        byte* pal2 = (byte*)(_palette) + 3;
        for( int j = 1; j < i; ++j, pal2 += 3 ) {
            byte c = find_color( ( int( pal1[0] ) + pal2[0] ) / 2, ( int( pal1[1] ) + pal2[1] ) / 2,
                                 ( int( pal1[2] ) + pal2[2] ) / 2 );
            gho[i + 256 * j] = c;
            gho[j + 256 * i] = c;
        }
    }
}

void ghost_set_auto()
{
    ghost_auto = !!getparm();
    retval( 0 );
}

void ghost_select()
{
    int select = getparm();
    if( select == 0 ) {
        make_ghost_standard();
    } else if( select == 1 ) {
        make_ghost_additive();
    }
    retval( 0 );
}

// Refrescamos nuestra tabla ghost personalizada cada vez que
// cargamos una nueva paleta
void process_palette( void )
{
    if( ghost_auto ) {
        make_ghost_additive();
    }
}

void __export divlibrary( LIBRARY_PARAMS )
{
    COM_export( "ghost_set_auto", ghost_set_auto, 1 );
    COM_export( "ghost_select", ghost_select, 1 );
}

void __export divmain( COMMON_PARAMS )
{
    GLOBAL_IMPORT();
    DIV_export( "process_palette", process_palette );
}
