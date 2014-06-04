
BASE  =  RPiGpio
PROG  =  ${BASE} RPiOperant
CONFS =  ${BASE}.sh RPiOperant.bashrc

all: ${PROG}

install: ${PROG}
	install -Dm755 -t /usr/bin ${PROG}
	install -Dm644 -t /usr/include ${BASE}.h
	install -Dm644 -t /etc/${BASE} ${SHARE}

clean:
	rm -f ${PROG}
