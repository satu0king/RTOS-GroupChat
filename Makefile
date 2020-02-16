
BUILD   = ./build
BIN     = ./bin
INCLUDE = ./include
SRC = ./src

all: server client testRig

server: $(BIN)/server  
	
client: $(BIN)/client  

testRig: $(BIN)/testRig server client 
	rm -rf log
	mkdir log

$(BIN)/testRig: $(BUILD)/fort.o $(SRC)/testRig.c
	gcc $(SRC)/testRig.c -lpthread  $(BUILD)/fort.o -o $(BIN)/testRig -I $(INCLUDE)

$(BIN)/client: $(SRC)/client.c $(INCLUDE)/messages.h $(INCLUDE)/ntp.h
	gcc $(SRC)/client.c -lpthread -o $(BIN)/client -I $(INCLUDE)

$(BIN)/server: $(SRC)/server.c $(INCLUDE)/messages.h $(INCLUDE)/queue.h
	gcc $(SRC)/server.c -lpthread -o $(BIN)/server -I $(INCLUDE)

$(BUILD)/fort.o: $(SRC)/fort.c $(INCLUDE)/fort.h
	gcc $(SRC)/fort.c -c -o $(BUILD)/fort.o -I $(INCLUDE)

clean: 
	rm -rf $(BIN)/*
	rm -rf $(BUILD)/*