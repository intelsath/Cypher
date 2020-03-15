GCCFLAGS=-Wall

all: PWM.o main.o
  g++ -o BL ../libs/PWM.o main.o 

PWM.o: ../libs/PWM.cpp
	g++ ${GCCFLAGS} -o ../libs/PWM.o -c ../libs/PWM.cpp

Backlight.o: main.cpp
	g++ ${GCCFLAGS} -o main.o -c main.cpp

clean:
	rm *.o
	rm ../libs/*.o
