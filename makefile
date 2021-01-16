CPP = g++
CC = gcc
CXXFLAGS = -std=c++11
OBJ = bin/assembler.o bin/bs_util.o bin/preprocessor.o bin/snapshot.o bin/main.o
LINKOBJ = bin/assembler.o bin/bs_util.o bin/preprocessor.o bin/snapshot.o bin/main.o
BIN = test/siasm
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN)

bin/assembler.o: assembler.cpp
	$(CPP) -c assembler.cpp -o assembler.o $(CXXFLAGS)
    
bin/bs_util.o: bs_util.cpp
	$(CPP) -c bs_util.cpp -o bs_util.o $(CXXFLAGS)

bin/preprocessor.o: preprocessor.cpp
	$(CPP) -c preprocessor.cpp -o preprocessor.o $(CXXFLAGS)
    
bin/snapshot.o: snapshot.cpp
	$(CPP) -c snapshot.cpp -o snapshot.o $(CXXFLAGS)
	
bin/main.o: main.cpp
	$(CPP) -c main.cpp -o main.o $(CXXFLAGS)