test: inserta_fichero.o mytar_utils.o
	gcc test.c inserta_fichero.o mytar_utils.o -o test

inserta_fichero.o: mytar_utils.o mytar_utils.h
	gcc -c inserta_fichero.c

mytar_utils.o: mytar_utils.c s_mytarheader.h
	gcc -c mytar_utils.c


clean:
	rm -f *.o