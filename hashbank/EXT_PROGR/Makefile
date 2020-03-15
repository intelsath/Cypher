GCCFLAGS=-Wall

all: GPIO.o main.o
	g++ -o EXTPROG ../libs/GPIO.o main.o 

GPIO.o: ../libs/GPIO.cpp
	g++ ${GCCFLAGS} -o ../libs/GPIO.o -c ../libs/GPIO.cpp

main.o: main.cpp
	g++ ${GCCFLAGS} -o main.o -c main.cpp

clean:
	rm *.o 
	rm ../libs/*.o