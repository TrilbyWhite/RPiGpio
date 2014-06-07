
BASE   =  RPiGpio
PROG   =  ${BASE} RPiOperant
CONFS  =  ${BASE}.sh RPiOperant.bashrc
PREFIX ?= /usr

all: ${PROG}

install: ${PROG}
	install -Dm755 -t ${DESTDIR}${PREFIX}/bin ${PROG}
	install -Dm644 -t ${DESTDIR}${PREFIX}/include ${BASE}.h
	mkdir -p ${DESTDIR}/etc/${BASE}
	install -Dm644 -t ${DESTDIR}/etc/${BASE} ${CONFS}

clean:
	rm -f ${PROG}
