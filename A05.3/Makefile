# https://makefiletutorial.com/

vpath insertarEj ../

all: insertarEj extrearEj


extrearEj: extrear_fichero_main.o extrae_fichero.o mytar_utils.o
	gcc extrear_fichero_main.o extrae_fichero.o mytar_utils.o -o ../extrearEj

extrear_fichero_main.o: extrear_fichero_main.c
	gcc extrear_fichero_main.c -c

extrae_fichero.o: extrae_fichero.c s_mytarheader.h mytar_utils.h
	gcc extrae_fichero.c -c


insertarEj: insertar_fichero_main.o inserta_fichero.o mytar_utils.o
	gcc insertar_fichero_main.o inserta_fichero.o mytar_utils.o -o ../insertarEj

insertar_fichero_main.o: insertar_fichero_main.c
	gcc insertar_fichero_main.c -c 

inserta_fichero.o: inserta_fichero.c s_mytarheader.h mytar_utils.h
	gcc inserta_fichero.c -c 


mytar_utils.o: mytar_utils.c mytar_utils.h s_mytarheader.h
	gcc mytar_utils.c -c

.PHONY: all, clean

clean:
	@rm -f *.o