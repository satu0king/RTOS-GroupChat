
BUILD   = ./build
BIN     = ./bin
INCLUDE = ./include
SRC = ./src

server: $(BIN)/server  
	

$(BIN)/server: $(SRC)/server.c $(INCLUDE)/messages.h $(INCLUDE)/queue.h
	gcc $(SRC)/server.c -o $(BIN)/server -I $(INCLUDE)

client: $(BIN)/client  

testRig: $(BIN)/testRig 
	rm -rf log
	mkdir log

$(BIN)/testRig : server client $(BIN)/fort.o
	gcc $(SRC)/testRig.c -lpthread  $(BIN)/fort.o -o $(BIN)/testRig -I $(INCLUDE)

$(BIN)/client: $(SRC)/client.c $(INCLUDE)/messages.h $(INCLUDE)/ntp.h
	gcc $(SRC)/client.c -lpthread -o $(BIN)/client -I $(INCLUDE)

$(BIN)/fort.o: $(SRC)/fort.c $(INCLUDE)/fort.h
	gcc $(SRC)/fort.c -c -o $(BIN)/fort.o -I $(INCLUDE)

clean: 
	rm -rf $(BIN)/*
	rm -rf $(BUILD)/*