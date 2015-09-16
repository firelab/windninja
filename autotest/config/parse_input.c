
#include <stdio.h>
#include <string.h>

static void chomp( char *s )
{
    while( *s != '\0' && isspace( *s ) ) s++;
}

void base( char *dst, char *s )
{
    char *p;
    p = strrchr( s, '/' ) ? strrchr( s, '/' ) : strrchr( s, '\\' );
    if( !p )
        p = s;
    else if( *(p+1) != '\0' )
        p++;
    strcpy( dst, p );
}

int main( int argc, char *argv[] )
{
    const char *pszConfigFile;
    const char *pszKey;
    const char *pszValue;
    char szBuf[512];
    char szBase[512];
    *szBase = '\0';
    char *p;
    FILE *fin;

    if( argc < 3 )
        return 1;
    pszConfigFile = argv[1];
    pszKey = argv[2];
    pszValue == NULL;
    fin = fopen( pszConfigFile, "r" );
    if( !fin )
        return 1;
    while( fgets( szBuf, 512, fin ) != NULL )
    {
        p = szBuf;
        chomp( p );
        /* empty or comment */
        if( *p == '\0' || *p == '#' )
            continue;
        if( strncmp( p, pszKey, strlen( pszKey ) ) != 0 )
            continue;
        p = strchr( szBuf, '=' );
        if( !p ) /* invalid cfg */
        {
            fclose( fin );
            return 1;
        }
        chomp( ++p );
        if( *p == '\0' ) /* again, invalid */
        {
            fclose( fin );
            return 1;
        }
        base( szBase, p );
        break;
    }
    chomp( szBase );
    printf( "%s", szBase );
    fclose( fin );
    return 0;
}

