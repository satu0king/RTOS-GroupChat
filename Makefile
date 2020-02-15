
BUILD   = ./build
BIN     = ./bin
INCLUDE = ./include
SRC = ./src

server: $(BIN)/server  
	$(BIN)/server

$(BIN)/server: $(SRC)/server.c $(INCLUDE)/chat.h
	gcc $(SRC)/server.c -o $(BIN)/server -I $(INCLUDE)

client: $(BIN)/client  
	$(BIN)/client

$(BIN)/client: $(SRC)/client.c $(INCLUDE)/chat.h
	gcc $(SRC)/client.c -o $(BIN)/client -I $(INCLUDE)

clean: 
	rm -rf $(BIN)/*
	rm -rf $(BUILD)/*