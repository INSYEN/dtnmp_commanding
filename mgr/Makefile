SOURCES = ../shared/adm/*.c ../shared/msg/*.c ../shared/utils/*.c ../shared/primitives/*.c
SOURCES += 	mgr_db.c \
			nm_mgr.c \
			nm_mgr_names.c \
			nm_mgr_print.c \
			nm_mgr_rx.c

IONDIR = ../../
OPTS = -Wno-write-strings -g

all:with-netui

with-netui:
	gcc ${OPTS} \
	-I. -I.. \
	-I${IONDIR}/ici/include \
	-I${IONDIR}/ltp/include \
	-I${IONDIR}/ltp/library \
	-I${IONDIR}/bp/include \
	-I${IONDIR}/bp/library \
	-I${IONDIR}/bp/library/ext/bpsec \
	-I${IONDIR}/ici/library \
	-I/usr/local/include \
	-L${IONDIR}/ \
	-L/usr/local/lib \
	-lbp -lici -lltp \
	-DMAXPATHLEN=2048 \
	-DHAVE_NETUI \
	-DAMP_DEBUGGING=1 \
	-c ${SOURCES} nm_mgr_netui.c

	gcc -o nm_mgr *.o -lltp -lbp -lici -lpthread

without-db:
	gcc ${OPTS} \
	-I. -I.. \
	-I${IONDIR}/ici/include \
	-I${IONDIR}/ltp/include \
	-I${IONDIR}/ltp/library \
	-I${IONDIR}/bp/include \
	-I${IONDIR}/bp/library \
	-I${IONDIR}/bp/library/ext/bpsec \
	-I${IONDIR}/ici/library \
	-I/usr/local/include \
	-L${IONDIR}/ \
	-L/usr/local/lib \
	-lbp -lici -lltp \
	-DMAXPATHLEN=2048 \
	-c ${SOURCES} nm_mgr_ui.c ui_input.c

	gcc -o nm_mgr *.o -lltp -lbp -lici -lpthread

with-db:
	gcc ${OPTS} \
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
	-c ${SOURCES} nm_mgr_ui.c ui_input.c nm_mgr_sql.c

	gcc -o nm_mgr *.o -lltp -lbp -lici -lmysqlclient -lpthread

clean:
	rm -f nm_mgr nm_mgr_netui
	rm -f *.o

