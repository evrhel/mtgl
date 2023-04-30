BIN := bin
MTGL_BIN := ../mtgl/bin
OBJ := gcc -O3 -I include -I dependencies/include -I ../mtgl/include -g -O -c
OBJ_DB := gcc -Og -I include -I dependencies/include -I ../mtgl/include -g -O -c
LNK := gcc -O3 -o
LNK_DB := gcc -Og -o

all: main.o shaders.o glad.o mtgl
	mkdir -p $(BIN)
	$(LNK) $(BIN)/example $(BIN)/main.o $(BIN)/shaders.o $(BIN)/glad.o \
		$(MTGL_BIN)/mtgl_ctx.o $(MTGL_BIN)/mtgl_lock.o $(MTGL_BIN)/mtgl_win.o $(MTGL_BIN)/mtgl.o 

debug: main_db.o shaders_db.o mtgl_db dependencies_db
	mkdir -p $(BIN)
	$(LNK_DB) $(BIN)/example $(BIN)/main_db.o $(BIN)/shaders_db.o $(BIN)/glad_db.o \
		$(MTGL_BIN)/mtgl_ctx_db.o $(MTGL_BIN)/mtgl_lock_db.o $(MTGL_BIN)/mtgl_win_db.o $(MTGL_BIN)/mtgl_db.o

clean:
	make -C ../mtgl clean
	rm -rf $(BIN)

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
