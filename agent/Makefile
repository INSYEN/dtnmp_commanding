SOURCES = ../shared/adm/*.c ../shared/msg/*.c ../shared/utils/*.c ../shared/primitives/*.c ./*.c 
IONDIR = ./ion-open-source
LIBPATH = /usr/local/lib
OPTS = -Wno-write-strings -g 
Debug: all

all:
	g++ ${OPTS} \
	-I. -I.. \
	-I${IONDIR}/ici/include \
	-I${IONDIR}/ltp/include \
	-I${IONDIR}/ltp/library \
	-I${IONDIR}/bp/include \
	-I${IONDIR}/bp/library \
	-I${IONDIR}/ici/library \
	-I/usr/local/include \
	-L${IONDIR}/ \
	-L/usr/local/lib \
	-L${LIBPATH}/ \
	-lbp -lici -lltp \
	-lipnfw \
	-DMAXPATHLEN=2048 \
	-c ${SOURCES} 

	g++ -L${LIBPATH}/ -o nm_agent *.o -lltp -lbp -lici -lipnfw

