#include "fake15f.h"
#include <cstdint>
#include <iostream>
#include <bitset>
#include <ostream>
#include <string>
#include <thread>
#include <vector>
//#include <b15f/b15f.h>

//CONSTS
constexpr std::bitset<4> SILLYSEQUENCE(0b1010);
B15F* drv = nullptr;

B15F* setup(){
    B15F& drv = B15F::getInstance();
    drv.setRegister(&DDRA, 0b00001111);
    return &drv;
}


uint8_t readNibble(){
    return (drv->getRegister(&PINA) >> 4);
}

std::bitset<4> readNibbleBitset(){
    return std::bitset<4>(drv->getRegister(&PINA) >> 4);
}

void writeData(uint8_t value) {
    uint8_t current = drv -> getRegister(&PORTA) & 0xF0; // RX
    uint8_t newValue = current | (value & 0x0F); // + TX, nur 4 bits
    drv -> setRegister(&PORTA, newValue);
}

void writeData(std::bitset<4> value){
    writeData(value.to_ulong());
}

void exitHandler(int sig){
    writeData(0b0000);
    exit(sig);
}


std::vector<std::bitset<4>> stringToNibbles(std::string& input){
    std::vector<std::bitset<4>> stringNibbles;
    stringNibbles.reserve(3*input.size() + 1); // hab hier +1 gemacht

    for(size_t i = 0; i < input.size(); i++){
        uint8_t byteData = (uint8_t)input.at(i);

        std::bitset<4> nibble_lower(byteData & 0b00001111);
        std::bitset<4> nibble_upper((byteData & 0b11110000) >> 4);


        stringNibbles.push_back(SILLYSEQUENCE);
        stringNibbles.push_back(nibble_upper);
        stringNibbles.push_back(nibble_lower);
    }

    stringNibbles.push_back(SILLYSEQUENCE);
    stringNibbles.push_back(std::bitset<4>(0)); //NIBBLER_UPER
    stringNibbles.push_back(std::bitset<4>(0)); //NIBBLE_LOWER

    return stringNibbles;
}

void sendStringNibbles(std::vector<std::bitset<4>>& data){
    for(std::bitset<4> bits : data){
        writeData(bits);
        drv -> delay_ms(10);
    }
}


std::string readStringNibbles(){
    std::string msg;
    int status = 0; // 0 sucht nach SILLYSEQUENCE, 1=upperNibble, 2=lowerNibble
    std::bitset<4> upperNibble;

    while(1) {
        std::bitset<4> currentNibble = readNibbleBitset();

        switch(status) {
            case 0:
                if (currentNibble == SILLYSEQUENCE) {
                    status = 1;
                    std::cout << "SILLYSEQUENCE empfangen, upperNibble folgt" << std::endl;
                }
                break;
            case 1:
                upperNibble = currentNibble;
                status = 2;
                std::cout << "upperNibble empfangen: " << upperNibble << " folgt lowerNibble." << std::endl;
                break;
            case 2:
                std::bitset<4> lowerNibble = currentNibble;
                std::cout << "lowerNibble empfanfen: " << lowerNibble << std::endl;

                uint8_t byte = (upperNibble.to_ulong() << 4) | lowerNibble.to_ulong();
                char character = (char)byte;

                if (byte == 0x00) {
                    std::cout << "msg::end;" << std::endl;
                    return msg;
                }

                std::cout << "[ByteToChar]: " << int(byte) << " -> " << character << std::endl;
                msg += character;
                status = 0;
                break;
        }
    }
}


int main(void){
    signal(SIGINT, exitHandler);
    drv = setup();
    drv->delay_ms(1300); // FAKE15, eigentlich unnötig aber bessere Ausgabe im Terminal

    std::cout << "Empfang wird gestartet..." << std::endl;
    std::thread receiver([](){
        while(1) {
            std::string received = readStringNibbles();
            if (!received.empty()) {
                std::cout << "\n[RX] " << received << std::endl;
            }
        }
    });

    std::string inputString;
    while(1) {
        std::cout << "INPUT STRING: ";
        std::getline(std::cin, inputString);

        if (inputString == "exit") break;

        std::vector<std::bitset<4>> daten = stringToNibbles(inputString);
        sendStringNibbles(daten);
        std::cout << "[TX] Gesendet: " << inputString << std::endl;
    }
    receiver.detach();
    return 0;
}
