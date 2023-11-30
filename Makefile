CC=g++
C_FLAGS=-O3 -std=c++17

SRC_FOLDER=src
INC_FOLDER=include
OUT_FILE=output.out

SRCS = $(wildcard $(SRC_FOLDER)/*.cpp)

run: $(SRCS)
	$(CC) $(C_FLAGS) -I$(INC_FOLDER) $^ -o  $(OUT_FILE)
	./$(OUT_FILE)

clean:
	rm $(OUT_FILE)