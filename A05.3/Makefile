# https://makefiletutorial.com/


all: insertarEj extraerEj


extraerEj: extrae_fichero_test.o extrae_fichero.o mytar_utils.o
	gcc extrae_fichero_test.o extrae_fichero.o mytar_utils.o -o extraerEj

extrae_fichero_test.o: extrae_fichero_test.c
	gcc extrae_fichero_test.c -c

extrae_fichero.o: extrae_fichero.c s_mytarheader.h mytar_utils.h
	gcc extrae_fichero.c -c


insertarEj: insertar_fichero_main.o inserta_fichero.o mytar_utils.o
	gcc insertar_fichero_main.o inserta_fichero.o mytar_utils.o -o insertarEj

insertar_fichero_main.o: insertar_fichero_main.c
	gcc insertar_fichero_main.c -c 

inserta_fichero.o: inserta_fichero.c s_mytarheader.h mytar_utils.h
	gcc inserta_fichero.c -c 


mytar_utils.o: mytar_utils.c mytar_utils.h s_mytarheader.h
	gcc mytar_utils.c -c

.PHONY: all, clean

clean:
	@rm -f *.o