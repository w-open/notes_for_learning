OUTPATH=$(shell pwd)/out
BINPATH=$(shell pwd)/bin

export OUTPATH
export BINPATH
export CC=gcc
export EXEC_NAME=a.out

all:
	@mkdir -p $(OUTPATH)
	@mkdir -p $(BINPATH)
	@${MAKE} -C src
clean:
	-rm -rf bin/
	-rm -rf out/
	-rm -rf log/
help:
	@echo ${OUTPATH}
	@echo "make all"
