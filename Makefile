OPS = -std=c99 -Wall -Wextra -pedantic
OPT = -O3 -march=native -mtune=native
DBG = -ggdb
LIBS = -lglfw -lGLEW -lGL -lm -lpng -lnoise
DEST = main
CC = cc

all: obj/main.o obj/glStuff.o obj/game.o
	${CC} ${LIBS} ${OPT} -o ${DEST} $?

debug: obj/main.dbg.o obj/glStuff.dbg.o obj/game.dbg.o
	${CC} ${LIBS} ${DBG} -o ${DEST}.dbg $?

obj/main.o: src/main.c obj
	${CC} ${OPS} ${OPT} -c $< -o $@

obj/glStuff.o: src/glStuff.c obj
	${CC} ${OPS} ${OPT} -c $< -o $@

obj/game.o: src/game.c obj
	${CC} ${OPS} ${OPT} -c $< -o $@

obj/main.dbg.o: src/main.c obj
	${CC} ${OPS} ${DBG} -c $< -o $@

obj/glStuff.dbg.o: src/glStuff.c obj
	${CC} ${OPS} ${DBG} -c $< -o $@

obj/game.dbg.o: src/game.c obj
	${CC} ${OPS} ${DBG} -c $< -o $@

obj:
	mkdir obj
