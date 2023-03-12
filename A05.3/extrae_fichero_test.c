int extrae_fichero(char * f_mytar, char * f_dat);

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]){
    int res;

    if(argc!=3){
        printf("Cantidad de argumentos erronea.\nDebe introducir lo siguiente: nombreFichero.tar , nombreFichero.dat\n");
    }

    res = extrae_fichero(argv[1], argv[2]);
    printf("extrae_fichero = %d\n", res);
    exit(0);
}
