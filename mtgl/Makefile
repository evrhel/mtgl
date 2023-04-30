BIN := bin
OBJ := gcc -O3 -I include -I ../dependencies/include -I ../mtgl/include -g -O -c
OBJ_DB := gcc -Og -I include -I ../dependencies/include -I ../mtgl/include -g -O -c

all: mtgl_ctx.o mtgl_lock.o mtgl_win.o mtgl.o

debug: mtgl_ctx_db.o mtgl_lock_db.o mtgl_win_db.o mtgl_db.o

clean:
	rm -rf $(BIN)

mtgl_ctx.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl_ctx.o src/mtgl_ctx.c

mtgl_ctx_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_ctx_db.o src/mtgl_ctx.c

mtgl_lock.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl_lock.o src/mtgl_lock.c

mtgl_lock_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_lock_db.o src/mtgl_lock.c

mtgl_win.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl_win.o src/mtgl_win.c

mtgl_win_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_win_db.o src/mtgl_win.c

mtgl.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl.o src/mtgl.c

mtgl_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_db.o src/mtgl.c
