CC = g++
OUT = final
SRCS = final.cpp
LDFLAGS = -g
CFLAGS = -Werror -std=c++17 -O0 -lpthread -flax-vector-conversions
CGFLAGS = -Werror -std=c++17 -O0 -flax-vector-conversions

final:
	${CC} -o ${OUT} ${SRCS} `pkg-config --cflags --libs opencv4` ${CFLAGS}

profiler:
	${CC} -o ${OUT} ${SRCS} `pkg-config --cflags --libs opencv4` ${CFLAGS} -lprofiler -pg

gpu:
	${CC} -o ${OUT} ${SRCS} `pkg-config --cflags --libs opencv4` ${CGFLAGS} -IV3DLib

clean:
	rm -f final
