GPP=g++ -std=c++11 -Wall
BIN_DIR=./bin
SRC_DIR=./src
BUILD_DIR=build
all:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)
	mkdir $(BUILD_DIR)
	mkdir $(BIN_DIR)
	$(GPP) -c $(SRC_DIR)/WrapperSocket.cpp -o $(BIN_DIR)/WrapperSocket.o	
	$(GPP) -c $(SRC_DIR)/Client.cpp -o $(BIN_DIR)/Client.o
	$(GPP) -c $(SRC_DIR)/Server.cpp -o $(BIN_DIR)/Server.o
	$(GPP) -c $(SRC_DIR)/User.cpp -o $(BIN_DIR)/User.o
	$(GPP) -c $(SRC_DIR)/MessageData.cpp -o $(BIN_DIR)/MessageData.o
	$(GPP) -c $(SRC_DIR)/FileRecord.cpp -o $(BIN_DIR)/FileRecord.o
	$(GPP) -c $(SRC_DIR)/Operations.cpp -o $(BIN_DIR)/Operations.o
	$(GPP) -o $(BUILD_DIR)/client $(BIN_DIR)/WrapperSocket.o $(BIN_DIR)/Client.o $(BIN_DIR)/Server.o $(BIN_DIR)/User.o $(BIN_DIR)/MessageData.o $(BIN_DIR)/FileRecord.o $(BIN_DIR)/Operations.o client.cpp -pthread
	$(GPP) -o $(BUILD_DIR)/server $(BIN_DIR)/WrapperSocket.o $(BIN_DIR)/Client.o $(BIN_DIR)/Server.o $(BIN_DIR)/User.o $(BIN_DIR)/MessageData.o $(BIN_DIR)/FileRecord.o $(BIN_DIR)/Operations.o server.cpp -pthread

clean:
	rm -rf *.o *~ build bin
