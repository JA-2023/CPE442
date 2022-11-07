CC = g++
OUT = lab5
SRCS = lab5.cpp
LDFLAGS = -g
CFLAGS = -Werror -mfpu=neon -std=c++11 -O0 -lpthread

lab5:
	${CC} -o ${OUT} ${SRCS} `pkg-config opencv4 --cflags --libs` ${CFLAGS}

# lab4:
# 	g++ -o lab5 lab5.cpp `pkg-config opencv4 --cflags --libs` -lpthread -O0
	

clean:
	rm -f lab5