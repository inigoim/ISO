
#include <stdio.h>
#include <stdlib.h>

int inserta_fichero(const char * f_mytar, const char * f_dat);

int main(int argc, char *argv[]){
    int res;

    if(argc!=3){
        printf("Cantidad de argumentos erronea.\nDebe introducir lo siguiente: nombreFichero.tar , nombreFichero.dat\n");
        exit(1);
    }

   res = inserta_fichero(argv[1], argv[2]);
   printf("inserta_fichero = %d\n", res);
   exit(0);
}
