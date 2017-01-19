
root := $(shell pwd)
INC := -I$(root)/../include

LDFLAGS := -L$(root)/../ -lMPDPlaylists -L$(root)/../../lib/uriparser/lib -luriparser \
-L$(root)/../../lib/xerces/lib -lxerces-c



OBJ := main.o
SRC := main.cpp


CXX_FLAGS := -W -Wall -g -fPIC -pipe


.PHONY: all clean

all:
	make clean
	cd .. && make
	make main

main: $(OBJ)
	$(CXX) -fPIC $(OBJ) -o main $(LDFLAGS)


%.o: %.cpp
	$(CXX) -c $< -o $@ $(INC) $(CXX_FLAGS)


clean:
	rm -rf main main.o
	cd .. && make clean


