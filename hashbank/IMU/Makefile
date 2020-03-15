GCCFLAGS=-Wall

all: I2C.o IMU.o main.o
	g++ -o IMU ../libs/I2C.o ../libs/IMU.o main.o

I2C.o: ../libs/I2C.cpp
	g++ ${GCCFLAGS} -o ../libs/I2C.o -c ../libs/I2C.cpp

IMU.o: ../libs/IMU.cpp
	g++ ${GCCFLAGS} -o ../libs/IMU.o -c ../libs/IMU.cpp

main.o: main.cpp
	g++ ${GCCFLAGS} -o main.o -c main.cpp

clean:
	rm *.o 