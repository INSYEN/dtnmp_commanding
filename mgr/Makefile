SOURCES = ../shared/adm/*.c ../shared/msg/*.c ../shared/utils/*.c ../shared/primitives/*.c ./*.c 
IONDIR = ../ion-open-source/
OPTS = -Wno-write-strings -g 

Debug:all

all:without-db

without-db:
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
	-lbp -lici -lltp \
	-DMAXPATHLEN=2048 \
	-DHAVE_NETUI \
	-DDTNMP_DEBUGGING \
	-c ${SOURCES} 

	g++ -o nm_mgr_netui *.o -lltp -lbp -lici

with-db:
	g++ ${OPTS} \
	-I. -I.. \
	-I${IONDIR}/ici/include \
	-I${IONDIR}/ltp/include \
	-I${IONDIR}/ltp/library \
	-I${IONDIR}/bp/include \
	-I${IONDIR}/bp/library \
	-I${IONDIR}/ici/library \
	-I/usr/local/include \
	-I/usr/include/mysql \
	-L${IONDIR}/ \
	-L/usr/local/lib \
	-L/usr/lib \
	-lbp -lici -lltp -lmysqlclient \
	-DMAXPATHLEN=2048 \
	-DHAVE_MYSQL=1 \
	-c ${SOURCES} 

	g++ -o nm_mgr *.o -lltp -lbp -lici -lmysqlclient



