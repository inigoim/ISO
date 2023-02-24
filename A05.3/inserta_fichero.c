/**
 * @file insertar_fichero.c
 * @author I.I., XG.C., E.F.
 * @date 24/02/2023
 * @brief First version of inserta_fichero
 * @details  Insert a tar file in dat file
*/

#include "s_mytarheader.h"



int inserta_fichero(char * f_mytar, char * f_dat)
{
    int ret;
    struct c_header_gnu_tar tar_header;

    ret = BuilTarHeader(f_dat, &tar_header);
    if (ret == HEADER_ERR) return -1;

}