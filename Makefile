all: clean server client

server:
	cd src/server; make all

client:
	cd src/client; make all

clean:
	cd src/server; make clean 
	cd src/client; make clean

.PHONY: server client all clean