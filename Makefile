
BASE   =  RPiGpio
PROG   =  ${BASE} RPiOperant
CONFS  =  ${BASE}.sh RPiOperant.bashrc
PREFIX ?= /usr

all: ${PROG}

install: ${PROG}
	mkdir -p ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${PREFIX}/include
	mkdir -p ${DESTDIR}${PREFIX}/lib/systemd/system
	mkdir -p ${DESTDIR}/etc/${BASE}
	install -Dm755 -t ${DESTDIR}${PREFIX}/bin ${PROG}
	install -Dm644 -t ${DESTDIR}${PREFIX}/include ${BASE}.h
	install -Dm644 -t ${DESTDIR}/etc/${BASE} ${CONFS}
	install -Dm644 ${PROG}.service ${DESTDIR}${PREFIX}/lib/systemd/system/${PROG}.service

clean:
	rm -f ${PROG}
