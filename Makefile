
BUILD   = ./build
BIN     = ./bin
INCLUDE = ./include
SRC = ./src

server: $(BIN)/server  
	

$(BIN)/server: $(SRC)/server.c $(INCLUDE)/messages.h
	gcc $(SRC)/server.c -o $(BIN)/server -I $(INCLUDE)

client: $(BIN)/client  

testRig: $(BIN)/testRig 
	rm log/*

$(BIN)/testRig : server client 
	gcc $(SRC)/testRig.c -o $(BIN)/testRig 

$(BIN)/client: $(SRC)/client.c $(INCLUDE)/messages.h
	gcc $(SRC)/client.c -o $(BIN)/client -I $(INCLUDE)

clean: 
	rm -rf $(BIN)/*
	rm -rf $(BUILD)/*