INC_DIR= -I. -I..
SRC_DIR = ./deamon ./face ./common ./table 
OBJ_DIR = ./objs

#SRC=$(wildcard $(SRC_DIR)/*.cpp)   

SRC=  ServerMain.cpp \
	  Socks5Server.cpp \
	  ../ccn_p2p/r_queue.cpp \
	  ../ccn_p2p/ndn_socket.cpp


OBJS := $(patsubst %.cpp, %.o,$(SRC))

CC := g++ 
CFLAGS := -g -std=c++14 ${INC_DIR}
LFLAGS := -lpthread -lpcap -lndn-cxx -lboost_system -ljsoncpp
TARGET := server

all: ${TARGET} 

endndc : ./endndc/endndc.cpp
	g++ ./endndc/endndc.cpp -o ./endndc/endndc

$(TARGET): ${OBJS}
	$(CC) ${OBJS} -o ${TARGET}  $(LFLAGS)

${OBJS} : %.o: %.cpp %.h
	${CC} ${CFLAGS} -c $< -o $@
	@#echo $^


#$@  表示目标文件
#$^  表示所有依赖文件
#$<  表示第一个依赖文件
#$?  表示比目标新的依赖文件列表

clean:  
	rm -rf $(TARGET) *.o ${OBJS} 

test:
	echo $(SRC)
	echo $(OBJS)
	echo $(CFLAGS)
