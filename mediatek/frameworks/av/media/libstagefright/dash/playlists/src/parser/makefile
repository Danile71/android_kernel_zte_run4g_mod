
ROOT   := $(shell pwd)
CONTROLLER_ROOT := $(ROOT)/../../dash_controller

DASH_SO_SRC := $(CONTROLLER_ROOT)/src/dash_common/IMtkDashPb_Common.c \
       $(CONTROLLER_ROOT)/src/dash_ctrl/dash_ctrl.c \
       $(CONTROLLER_ROOT)/src/dash_parser_glue/IMtkDashPb_ParserGlue.c \
       $(CONTROLLER_ROOT)/src/dash_table/IMtkDashPb_URLTable.c \
       $(CONTROLLER_ROOT)/src/mpd_fetcher/mpd_fecther.c 
DASH_SO_SRC1  := $(wildcard $(ROOT)/*.cpp)
MPD_OBJ := $(DASH_SO_SRC1:.cpp=.o)
MPD_OBJ += $(DASH_SO_SRC:.c=.o)

LIBMPDPLAYER := libdashplayer.so

export CFLAGS += -W -Wall -g -fPIC -pipe 
export CXXFLAGS += -W -Wall -g  -fPIC -pipe

export EXPAT_INC := -I$(CONTROLLER_ROOT)/../../../../../BDP_Generic/build/sysroot/usr/include
URIPARSER_INC := $(EXPAT_INC)/uriparser
CXX_FLAGS := -I$(CONTROLLER_ROOT)/src/dash_common \
             -I$(CONTROLLER_ROOT)/src/dash_parser_glue \
             -I$(CONTROLLER_ROOT)/src/dash_table \
             -I$(CONTROLLER_ROOT)/src/mpd_fetcher \
             -I$(CONTROLLER_ROOT)/include          \
             -I$(CONTROLLER_ROOT)/../../../../../BDP_Generic/src_ind/so/inc/imtkpbctrl_client/ \
             -I$(CONTROLLER_ROOT)/../../../../../BDP_Generic/src_ind/so/inc/imtkpbctrl_client/IMtkPb/ \
             -I. \
             $(EXPAT_INC) \
             $(URIPARSER_INC)
						

TARGET1 := $(ROOT)/libdashplayer.so

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++

.PHONY:clean config all

all: $(LIBMPDPLAYER)

$(LIBMPDPLAYER):$(MPD_OBJ)
	$(CXX) -shared -Wl,-export-dynamic $^ -o $@
	
%.o:%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(CXX_FLAGS)
	
%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS) $(DASH_SO_CFLAGS) $(URIPARSER_INC) $(CXX_FLAGS)

clean:
	rm -rf $(MPD_OBJ) $(LIBMPDPLAYER)
	rm -rf $(ROOT)/lib
	
