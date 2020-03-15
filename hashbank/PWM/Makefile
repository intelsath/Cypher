GCCFLAGS=-Wall

all: PWM.o GPIO.o main.o
	g++ -o PWM ../libs/PWM.o ../libs/GPIO.o main.o

GPIO.o: ../libs/GPIO.cpp
	g++ ${GCCFLAGS} -o ../libs/GPIO.o -c ../libs/GPIO.cpp

PWM.o: ../libs/PWM.cpp
	g++ ${GCCFLAGS} -o ../libs/PWM.o -c ../libs/PWM.cpp

main.o: main.cpp
	g++ ${GCCFLAGS} -o main.o -c main.cpp

clean:
	rm *.o 