BIN := bin
OBJ := gcc -O3 -I ../mtgl/include -I dependencies/include -g -O -c
OBJ_DB := gcc -Og -I ../mtgl/include -I dependencies/include -g -O -c
LNK := gcc -O3 -o
LNK_DB := gcc -Og -o

all: main.o shaders.o glad.o mtgl
	mkdir -p $(BIN)
	$(LNK) $(BIN)/example $(BIN)/main.o $(BIN)/shaders.o $(BIN)/mtgl.o $(BIN)/glad.o

debug: main_db.o shaders_db.o mtgl_db dependencies_db
	mkdir -p $(BIN)
	$(LNK_DB) $(BIN)/example $(BIN)/main_db.o $(BIN)/shaders_db.o $(BIN)/mtgl_db.o $(BIN)/glad_db.o

dependencies:
	make -C ../dependencies

dependencies_db:
	make -C ../dependencies debug

mtgl:
	make -C ../mtgl

mtgl_db:
	make -C ../mtgl debug

main.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/main.o src/main.c

main_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/main_db.o src/main.c

shaders.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/shaders.o src/shaders.c

shaders_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/shaders_db.o src/shaders.c

glad.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/glad.o dependencies/src/glad/glad.c

glad_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/glad_db.o dependencies/src/glad/glad.c
