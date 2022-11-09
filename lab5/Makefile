CC = g++
OUT = lab5
SRCS = lab5.cpp
LDFLAGS = -g
CFLAGS = -Werror -std=c++11 -O0 -lpthread -flax-vector-conversions

lab5:
	${CC} ${CFLAGS} `pkg-config opencv4 --cflags --libs` ${SRCS} -o ${OUT}

# lab4:
# 	g++ -o lab5 lab5.cpp `pkg-config opencv4 --cflags --libs` -lpthread -O0
	

clean:
	rm -f lab5
