FLAGS = -g -Wall -Werror -Isrc
OBJECTS = target/main.o target/pool.o target/server.o
BIN = target/server

target/server : ${OBJECTS}
	gcc ${OBJECTS} -lpthread -o ${BIN}

target/%.o : src/%.c
	gcc -c $^ ${FLAGS} -o $@

.PHONY clean:
	rm target/*.o
	rm ${BIN}