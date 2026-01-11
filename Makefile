CC=gcc
AR=ar
AS=as

CFLAGS=-Wall -Wpedantic -Wextra -ggdb
LFLAGS=

PREFIX=
OBJ_DIR=$(PREFIX)/obj
INC_DIR=$(PREFIX)/include
LIB_DIR=$(PREFIX)/lib

OBJ=queue.o threadpool.o context.o
INC=threadpool.h
EXE=libthreadpool.a

EXP_EXE=$(LIB_DIR)/$(EXE)
EXP_OBJ=$(addprefix $(OBJ_DIR)/,$(OBJ))
EXP_INC=$(addprefix $(INC_DIR)/,$(INC))

#.SILENT:

.PHONY: build clean

build: $(INC_DIR) $(LIB_DIR) $(EXP_OBJ) $(EXP_INC) $(EXP_EXE)

$(EXP_EXE): $(LIB_DIR) $(EXP_OBJ) $(EXP_INC)
	$(AR) -rcs $(EXP_EXE) $(EXP_OBJ)


$(OBJ_DIR)/%.o: %.s $(OBJ_DIR)
	$(AS) $< -o $@

$(OBJ_DIR)/%.o: %.c *.h $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(INC_DIR)/%.h: %.h $(INC_DIR)
	cp -v $< $@


$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(INC_DIR):
	mkdir -p $(INC_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

clean:
	rm -rv $(OBJ_DIR)
	rm -rv $(INC_DIR)
	rm -rv $(LIB_DIR)
