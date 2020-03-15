GCCFLAGS=-Wall

all: IPC.o fhandler.o uart.o sqldb.o main.o
	g++ -o UART ../libs/IPC.o ../libs/fhandler.o ../libs/uart.o ../libs/sqldb.o main.o -lsqlite3

fhandler.o: ../libs/fhandler.cpp
	g++ ${GCCFLAGS} -o ../libs/fhandler.o -c ../libs/fhandler.cpp

IPC.o: ../libs/IPC.cpp
	g++ ${GCCFLAGS} -o ../libs/IPC.o -c ../libs/IPC.cpp

uart.o: ../libs/uart.cpp
	g++ ${GCCFLAGS} -o ../libs/uart.o -c ../libs/uart.cpp

sqldb.o: ../libs/sqldb.cpp
	g++ ${GCCFLAGS} -o ../libs/sqldb.o -c ../libs/sqldb.cpp 

main.o: main.cpp
	g++ ${GCCFLAGS} -o main.o -c main.cpp

clean:
	rm *.o 
	rm ../libs/*.o