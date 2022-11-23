CC = g++
OUT = final
SRCS = final.cpp
LDFLAGS = -g
CFLAGS = -Werror -std=c++11 -O0 -lpthread -flax-vector-conversions

final:
	${CC} -o ${OUT} ${SRCS} `pkg-config --cflags --libs opencv4` ${CFLAGS}

profiler:
	${CC} -o ${OUT} ${SRCS} `pkg-config --cflags --libs opencv4` ${CFLAGS} -lprofiler

clean:
	rm -f final
