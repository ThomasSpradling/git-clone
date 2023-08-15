pit: main.c pit.c pit.h
	gcc -std=c99 main.c pit.c -o pit

clean:
	rm -rf pit