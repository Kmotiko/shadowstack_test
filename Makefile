CC = clang++
RUNTIME_SRC = MyRuntime.cpp
MAKELL_SRC = CreateLL.cpp

BIN_DIR = bin
OBJ_DIR = obj
RUNTIME_OBJ = $(OBJ_DIR)/$(RUNTIME_SRC:.cpp=.o)
MAKELL_OBJ = $(BIN_DIR)/$(MAKELL_SRC:.cpp=.o)
SS_BIN = $(BIN_DIR)/gctest


LLVM_CONFIG = llvm-config
LLVM_FLAGS = --cxxflags --ldflags --libs
FLAGS = -g -std=c++11 -stdlib=libstdc++
LIBS = -lncurses -lpthread -lz -lm -ldl 
#LIBS = -ldl 

LLC = llc
LL_FLAGS = -filetype=obj
LL_NAME = test.ll
LL_OBJ = $(OBJ_DIR)/$(LL_NAME:.ll=.o)


all: $(MAKELL_SRC) $(RUNTIME_SRC)
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)
	$(CC) $(FLAGS) -c $(RUNTIME_SRC) -o $(RUNTIME_OBJ)
	$(CC) $(FLAGS) $(MAKELL_SRC) -o $(MAKELL_OBJ)  `$(LLVM_CONFIG) $(LLVM_FLAGS)` $(LIBS) 
	$(MAKELL_OBJ) $(LL_NAME)
	$(LLC) $(LL_NAME) $(LL_FLAGS) -o $(LL_OBJ)
	$(CC) $(LL_OBJ) $(RUNTIME_OBJ) -o $(SS_BIN)

run:
	$(SS_BIN)

clean:
	rm $(RUNTIME_OBJ) $(MAKELL_OBJ) $(LL_NAME) $(LL_OBJ) $(BIN)
