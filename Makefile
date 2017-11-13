CPP = g++
CC = gcc
CXXFLAGS = -std=c++11
OBJ = main.o bs_util.o assembler.o snapshot.o preprocessor.o
LINKOBJ = main.o bs_util.o assembler.o snapshot.o preprocessor.o
BIN = siasm
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(CXXFLAGS) $(LINKOBJ) -o $(BIN)

main.o: main.cpp
	$(CPP) $(CXXFLAGS) -c main.cpp -o main.o

bs_util.o: bs_util.cpp
	$(CPP) $(CXXFLAGS) -c bs_util.cpp -o bs_util.o

assembler.o: assembler.cpp
	$(CPP) $(CXXFLAGS) -c assembler.cpp -o assembler.o

snapshot.o: snapshot.cpp
	$(CPP) $(CXXFLAGS) -c snapshot.cpp -o snapshot.o
	
preprocessor.o: preprocessor.cpp
	$(CPP) $(CXXFLAGS) -c preprocessor.cpp -o preprocessor.o
