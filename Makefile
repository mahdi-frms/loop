FLAGS = -g -Wall -Werror -Isrc
LIBOBJECTS = target/pool.o\
  	target/server.o\
   	target/hashmap.o\
    target/loop.o\
	target/task.o\
	target/worker.o\

OBJECTS = target/main.o\
target/server.o

BIN = target/server
LIB = target/libloop.a

${BIN} : ${OBJECTS} ${LIB}
	gcc ${OBJECTS} -lpthread -Ltarget -lloop -o ${BIN}

${LIB} : ${LIBOBJECTS}
	ar rs ${LIB} ${LIBOBJECTS}

target/%.o : src/%.c
	gcc -c $^ ${FLAGS} -o $@

.PHONY clean:
	rm target/*.o
	rm ${BIN}