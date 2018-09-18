GPP=g++
BIN_DIR=./bin
SRC_DIR=./src
BUILD_DIR=build
all: 
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)
	mkdir $(BUILD_DIR)
	mkdir $(BIN_DIR)
	$(GPP) -c $(SRC_DIR)/Message.cpp -o $(BIN_DIR)/message.o
	$(GPP) -c $(SRC_DIR)/WrapperSocket.cpp -o $(BIN_DIR)/WrapperSocket.o
	$(GPP) -c $(SRC_DIR)/Packet.cpp -o $(BIN_DIR)/Packet.o
	$(GPP) -o $(BUILD_DIR)/client $(BIN_DIR)/message.o $(BIN_DIR)/WrapperSocket.o $(BIN_DIR)/Packet.o client.cpp -std=c++11 -Wall

clean:
	rm -rf *.o *~ build bin