GCCFLAGS=-Wall -O3 # Optimizions on
# GCCFLAGS=-Wall -g debug mode on
# GCCFLAGS=-Wall # No optimizations


all: IPC.o fhandler.o stopwatch.o sqldb.o touchctrl.o I2C.o SPI.o ILI9488.o GPIO.o GFX.o IMG.o BTN.o font.o animation.o PWM.o main.o
	g++ -o LCD ../libs/IPC.o ../libs/fhandler.o ../libs/sqldb.o ../libs/stopwatch.o ../libs/touchctrl.o ../libs/I2C.o ../libs/SPI.o ../libs/ILI9488.o ../libs/GPIO.o ../libs/GFX.o ../libs/IMG.o ../libs/BTN.o ../libs/font.o ../libs/animation.o ../libs/PWM.o main.o -lsqlite3 -lpthread

fhandler.o: ../libs/fhandler.cpp
	g++ ${GCCFLAGS} -o ../libs/fhandler.o -c ../libs/fhandler.cpp

IPC.o: ../libs/IPC.cpp
	g++ ${GCCFLAGS} -o ../libs/IPC.o -c ../libs/IPC.cpp
	
stopwatch.o: ../libs/stopwatch.cpp
	g++ ${GCCFLAGS} -o ../libs/stopwatch.o -c ../libs/stopwatch.cpp 

sqldb.o: ../libs/sqldb.cpp
	g++ ${GCCFLAGS} -o ../libs/sqldb.o -c ../libs/sqldb.cpp 

GPIO.o: ../libs/GPIO.cpp
	g++ ${GCCFLAGS} -o ../libs/GPIO.o -c ../libs/GPIO.cpp

SPI.o: ../libs/SPI.cpp
	g++ ${GCCFLAGS} -o ../libs/SPI.o -c ../libs/SPI.cpp

ILI9488.o: ../libs/ILI9488.cpp
	g++ ${GCCFLAGS} -o ../libs/ILI9488.o -c ../libs/ILI9488.cpp

touchctrl.o: ../libs/touchctrl.cpp
	g++ ${GCCFLAGS} -o ../libs/touchctrl.o -c ../libs/touchctrl.cpp

I2C.o: ../libs/I2C.cpp
	g++ ${GCCFLAGS} -o ../libs/I2C.o -c ../libs/I2C.cpp

GFX.o: ../libs/GFX.cpp
	g++ ${GCCFLAGS} -o ../libs/GFX.o -c ../libs/GFX.cpp

IMG.o: ../libs/IMG.cpp
	g++ ${GCCFLAGS} -o ../libs/IMG.o -c ../libs/IMG.cpp

BTN.o: ../libs/BTN.cpp
	g++ ${GCCFLAGS} -o ../libs/BTN.o -c ../libs/BTN.cpp

font.o: ../libs/font.cpp
	g++ ${GCCFLAGS} -o ../libs/font.o -c ../libs/font.cpp

animation.o: ../libs/animation.cpp
	g++ ${GCCFLAGS} -o ../libs/animation.o -c ../libs/animation.cpp

PWM.o: ../libs/PWM.cpp
	g++ ${GCCFLAGS} -o ../libs/PWM.o -c ../libs/PWM.cpp

main.o: main.cpp
	g++ ${GCCFLAGS} -o main.o -c main.cpp

clean:
	rm *.o 
	rm ../libs/*.o
