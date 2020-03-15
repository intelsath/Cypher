GCCFLAGS=-Wall

all: IPC.o fhandler.o stopwatch.o nfcnci.o NFC.o main.o
	g++ -o nfcApp ../libs/IPC.o ../libs/fhandler.o ../libs/stopwatch.o ../libs/nfcnci.o -pthread -ldl -lrt -lnfc_nci_linux
	g++ -o NFC ../libs/IPC.o ../libs/fhandler.o ../libs/NFC.o main.o 

fhandler.o: ../libs/fhandler.cpp
	g++ ${GCCFLAGS} -o ../libs/fhandler.o -c ../libs/fhandler.cpp

IPC.o: ../libs/IPC.cpp
	g++ ${GCCFLAGS} -o ../libs/IPC.o -c ../libs/IPC.cpp

stopwatch.o: ../libs/stopwatch.cpp
	g++ ${GCCFLAGS} -o ../libs/stopwatch.o -c ../libs/stopwatch.cpp 

nfcnci.o: ../libs/nfcnci.cpp
	g++ ${GCCFLAGS} -o ../libs/nfcnci.o -c ../libs/nfcnci.cpp

NFC.o: ../libs/NFC.cpp
	g++ ${GCCFLAGS} -o ../libs/NFC.o -c ../libs/NFC.cpp

main.o: main.cpp
	g++ ${GCCFLAGS} -o main.o -c main.cpp

clean:
	rm *.o 

