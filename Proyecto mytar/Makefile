# https://makefiletutorial.com/

path := ./codigo_fuente
opath := ./archivos_objeto

vpath %.c ./ $(path)
vpath %.h ./ $(path)
vpath %.o $(opath)

all: insertar extraer


extraer: extraer.c extrae_fichero.o mytar_utils.o
	gcc extraer.c $(opath)/extrae_fichero.o $(opath)/mytar_utils.o -o extraer

extrae_fichero.o: extrae_fichero.c s_mytarheader.h mytar_utils.h
	gcc $(path)/extrae_fichero.c -c -o $(opath)/extrae_fichero.o


insertar: insertar.c inserta_fichero.o mytar_utils.o
	gcc insertar.c $(opath)/inserta_fichero.o $(opath)/mytar_utils.o -o insertar

inserta_fichero.o: inserta_fichero.c s_mytarheader.h mytar_utils.h
	gcc $(path)/inserta_fichero.c -c -o $(opath)/inserta_fichero.o


mytar_utils.o: mytar_utils.c mytar_utils.h s_mytarheader.h
	gcc $(path)/mytar_utils.c -c -o $(opath)/mytar_utils.o


clean:
	rm -f $(opath)/*.o

	
.PHONY: all clean
