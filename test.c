#include <stdio.h>
#include <string.h>

int main()
{
    char zrodlo[] = "Dokumentacja silnika krokowego.";
    char dupa[] = "dupa";
    char x[] = ";";
    char przeznaczenie[ 40 ];
   
    strcat( przeznaczenie, zrodlo );
    strcat(przeznaczenie, ";");
    strcat(przeznaczenie, dupa);
    if(0){
    printf( "przeznaczenie: %s", przeznaczenie );
    }
    return 0;
}