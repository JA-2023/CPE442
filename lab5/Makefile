CC = g++
OUT = lab5
SRCS = lab5.cpp
LDFLAGS = -g
CFLAGS = -Werror -std=c++11 -O0 -lpthread -flax-vector-conversions

lab5:
	${CC} -o ${OUT} ${SRCS} `pkg-config --cflags --libs opencv4` ${CFLAGS}

clean:
	rm -f lab5
