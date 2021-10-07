FLAGS = -g -Wall -Werror -Isrc
OBJECTS = target/main.o\
 	target/pool.o\
  	target/server.o\
   	target/hashmap.o\
    target/loop.o\
	target/task.o\
	target/worker.o\

BIN = target/server

target/server : ${OBJECTS}
	gcc ${OBJECTS} -lpthread -o ${BIN}

target/%.o : src/%.c
	gcc -c $^ ${FLAGS} -o $@

.PHONY clean:
	rm target/*.o
	rm ${BIN}