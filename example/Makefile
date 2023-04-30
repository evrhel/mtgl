BIN := bin
OBJ := gcc -O3 -D GL_SILENCE_DEPRECATION -I include -I dependencies/include -I ../mtgl/include -g -O -c
OBJ_DB := gcc -Og -D GL_SILENCE_DEPRECATION -I include -I dependencies/include -I ../mtgl/include -g -O -c
LNK := gcc -framework Cocoa -framework OpenGL -O3 -o
LNK_DB := gcc -framework Cocoa -framework OpenGL -Og -o

all: main.o shaders.o glad.o mtgl_ctx.o mtgl_lock.o mtgl_win.o mtgl.o mtgl_win_cocoa.o mtgl_ctx_cocoa.o
	mkdir -p $(BIN)
	$(LNK) $(BIN)/example $(BIN)/main.o $(BIN)/shaders.o $(BIN)/glad.o \
		$(BIN)/mtgl_ctx.o $(BIN)/mtgl_lock.o $(BIN)/mtgl_win.o $(BIN)/mtgl.o $(BIN)/mtgl_win_cocoa.o $(BIN)/mtgl_ctx_cocoa.o

debug: main_db.o shaders_db.o mtgl_db dependencies_db mtgl_ctx_db.o mtgl_lock_db.o mtgl_win_db.o mtgl_db.o mtgl_win_cocoa_db.os mtgl_ctx_cocoa_db.o
	mkdir -p $(BIN)
	$(LNK_DB) $(BIN)/example $(BIN)/main_db.o $(BIN)/shaders_db.o $(BIN)/glad_db.o \
		$(BIN)/mtgl_ctx_db.o $(BIN)/mtgl_lock_db.o $(BIN)/mtgl_win_db.o $(BIN)/mtgl_db.o $(BIN)/mtgl_win_cocoa_db.o $(BIN)/mtgl_ctx_cocoa_db.o

clean:
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

mtgl_ctx.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl_ctx.o ../mtgl/src/mtgl_ctx.c

mtgl_ctx_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_ctx_db.o ../mtgl/src/mtgl_ctx.c

mtgl_lock.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl_lock.o ../mtgl/src/mtgl_lock.c

mtgl_lock_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_lock_db.o ../mtgl/src/mtgl_lock.c

mtgl_win.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl_win.o ../mtgl/src/mtgl_win.c

mtgl_win_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_win_db.o ../mtgl/src/mtgl_win.c

mtgl.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl.o ../mtgl/src/mtgl.c

mtgl_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_db.o ../mtgl/src/mtgl.c

mtgl_win_cocoa.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl_win_cocoa.o ../mtgl/src/mtgl_win_cocoa.m

mtgl_win_cocoa_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_win_cocoa_db.o ../mtgl/src/mtgl_win_cocoa.m

mtgl_ctx_cocoa.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl_ctx_cocoa.o ../mtgl/src/mtgl_ctx_cocoa.m

mtgl_ctx_cocoa_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_ctx_cocoa_db.o ../mtgl/src/mtgl_ctx_cocoa.m