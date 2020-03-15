GCCFLAGS=-Wall

all: I2C.o FT6236.o GPIO.o main.o
	g++ -o TEST ../libs/I2C.o ../libs/FT6236.o ../libs/GPIO.o main.o 

I2C.o: ../libs/I2C.cpp
	g++ ${GCCFLAGS} -o ../libs/I2C.o -c ../libs/I2C.cpp

FT6236.o: ../libs/FT6236.cpp
	g++ ${GCCFLAGS} -o ../libs/FT6236.o -c ../libs/FT6236.cpp

GPIO.o: ../libs/GPIO.cpp
	g++ ${GCCFLAGS} -o ../libs/GPIO.o -c ../libs/GPIO.cpp

main.o: main.cpp
	g++ ${GCCFLAGS} -o main.o -c main.cpp

clean:
	rm *.o 
	rm ../libs/*.o