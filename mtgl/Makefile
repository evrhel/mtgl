BIN := bin
OBJ := gcc -O3 -I include -I ../dependencies/include -I ../mtgl/include -g -O -c
OBJ_DB := gcc -Og -I include -I ../dependencies/include -I ../mtgl/include -g -O -c
LNK := gcc -O3 -o
LNK_DB := gcc -Og -o

all: glctx.o gllock.o glwin.o mtgl_input_private.o mtgl.o
	mkdir -p $(BIN)
	$(LNK) $(BIN)/mtgl $(BIN)/glctx.o $(BIN)/gllock.o $(BIN)/glwin.o $(BIN)/mtgl_input_private.o $(BIN)/mtgl.o

debug: glctx_db.o gllock_db.o glwin_db.o mtgl_input_private_db.o mtgl_db.o
	mkdir -p $(BIN)
	$(LNK_DB) $(BIN)/mtgl $(BIN)/glctx_db.o $(BIN)/gllock_db.o $(BIN)/glwin_db.o $(BIN)/mtgl_input_private_db.o $(BIN)/mtgl_db.o

glctx.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/glctx.o src/glctx.c

glctx_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/glctx_db.o src/glctx.c

gllock.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/gllock.o src/gllock.c

gllock_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/gllock_db.o src/gllock.c

glwin.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/glwin.o src/glwin.c

glwin_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/glwin_db.o src/glwin.c

mtgl_input_private.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl_input_private.o src/mtgl_input_private.c

mtgl_input_private_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_input_private_db.o src/mtgl_input_private.c

mtgl.o:
	mkdir -p $(BIN)
	$(OBJ) -o $(BIN)/mtgl.o src/mtgl.c

mtgl_db.o:
	mkdir -p $(BIN)
	$(OBJ_DB) -o $(BIN)/mtgl_db.o src/mtgl.c
