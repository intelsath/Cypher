GCCFLAGS=-Wall

all: SPI.o RC522.o main.o
	g++ -o RFID ../libs/SPI.o ../libs/RC522.o main.o

SPI.o: ../libs/SPI.c
	g++ ${GCCFLAGS} -o ../libs/SPI.o -c ../libs/SPI.c

RC522.o: ../libs/RC522.cpp
	g++ ${GCCFLAGS} -o ../libs/RC522.o -c ../libs/RC522.cpp

main.o: main.cpp
	g++ ${GCCFLAGS} -o main.o -c main.cpp

clean:
	rm *.o 
	rm ../libs/*.o