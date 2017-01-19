# for build library of mpd parser

root := $(shell pwd)


SRC := $(wildcard $(root)/src/playlist/src/*.cpp)
SRC += $(wildcard $(root)/src/parser/*.cpp)

OBJ := $(SRC:.cpp=.o)

INC := -I$(root)/include \
       -I$(root)/src/parser \
       -I$(root)/src/playlist/include \
       -I$(root)/../lib/xerces/include \
       -I$(root)/../lib/uriparser/include/uriparser

CXX_FLAGS := -W -Wall -g -fPIC -pipe -D__STDC_LIMIT_MACROS

CXX = g++
all:
	make clean
	make libMPDPlaylists.so


libMPDPlaylists.so:$(OBJ)
	$(CXX) -shared -Wl,-export-dynamic $^ -o $@

%.o:%.cpp
	$(CXX) -c $< -o $@ $(INC) $(CXX_FLAGS)

clean:
	rm -rf libMPDPlaylists.so $(OBJ)