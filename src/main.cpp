//#include "fake15f.h"
#include <cstdint>
#include <iostream>
#include <bitset>
#include <ostream>
#include <string>
#include <thread>
#include <vector>
#include <b15f/b15f.h>

//CONSTS UND GLOBALE
constexpr std::bitset<4> SILLYSEQUENCE(0b1010); // SYNC
std::bitset<4> MSGEND(0); // 0000 am Ende, damit man erkennen kann, wann eine Nachricht zu Ende ist
volatile bool running = true;

B15F& drv = B15F::getInstance();
//void setup() {
//    drv.setRegister(&DDRA, 0b00001111); // vier Bits Input / 4 Bits Output
//}

//R/W++
std::bitset<4> readNibble() {
    uint8_t msg = drv.digitalRead0();
    return std::bitset<4>(msg >> 4);
}

void writeNibble(std::bitset<4> nibble) {
    uint8_t status = drv.digitalRead0() & 0xF0;
    drv.digitalWrite0(status | nibble.to_ulong());
}

void exitHandler(int sig){
    running = false;
    drv.digitalWrite0(0x00);
    std::cout << "tschüss" << std::endl;
    exit(sig);
}

std::vector<std::bitset<4>> stringToNibbles(const std::string& input){
    std::vector<std::bitset<4>> nibbles;

    for(char ch : input) {
        uint8_t byte = (uint8_t)ch;
        std::bitset<4> upper_nibble ((byte >> 4) & 0x0F); // xxxxXXXX
        std::bitset<4> lower_nibble (byte & 0x0F); // XXXXxxxx

        nibbles.push_back(SILLYSEQUENCE);
        nibbles.push_back(upper_nibble);
        nibbles.push_back(lower_nibble);
    }

    nibbles.push_back(SILLYSEQUENCE);
    nibbles.push_back(MSGEND/*upper*/);
    nibbles.push_back(MSGEND/*lower*/);

    return nibbles;
}

void sendNibbles(const std::vector<std::bitset<4>>& nibbles){
    for(const auto& nibble : nibbles){
        writeNibble(nibble);
        drv.delay_ms(5);
    }
}


std::string receiveStringMsg() {
    static int status = 0; //[0] = wartet auf silly | [1] = read upper ibble | [2] = read lower
    static std::bitset<4> upper_nibble;
    static std::bitset<4> lower_nibble;
    static std::string msg;

    std::bitset<4> current_nibble = readNibble();
    if (current_nibble.none()) return "";

    switch(status) {
        case 0:
            if (current_nibble == SILLYSEQUENCE) {
                status = 1;
            }
            break;
        case 1:
            upper_nibble = current_nibble;
            status = 2;
            break;
        case 2:
            lower_nibble = current_nibble;

            if (upper_nibble.none() && lower_nibble.none()) {
                std::string msg_full = msg;
                msg.clear();
                status = 0;
                return msg_full;
            }

            uint8_t byte = (upper_nibble.to_ulong() << 4) | lower_nibble.to_ulong();
            msg += (char)byte;
            status = 0;
            break;
    }
    return "";
}

void receiverThread() {
    while(running) {
        std::string msg = receiveStringMsg();
        if (!msg.empty()) {
            std::cout << "\n[RX] " << msg << std::endl;
            std::cout << "TX >> " << std::flush;
        }
        drv.delay_ms(1);
    }
}


int main(void){
    signal(SIGINT, exitHandler);

    //setup();
    std::cout << "'exit' um das Programm zu beeenden." << std::endl;
    drv.delay_ms(1300); // FAKE15, eigentlich unnötig aber bessere Ausgabe im Terminal

    std::cout << "Empfang wird gestartet..." << std::endl;
    std::thread receiver(receiverThread);

    std::string inputString;
    while(running) {
        std::cout << "TX >> ";
        if (!std::getline(std::cin, inputString)) break;
        if (inputString == "exit") {
            running = false;
            break;
        }
        if (!inputString.empty()) {
            auto nibbles = stringToNibbles(inputString);
            sendNibbles(nibbles);
            std::cout << "[TX] Nachricht gesendet: " << inputString << std::endl;
        }
    }
    running = false;

    if (receiver.joinable()) receiver.join();
    drv.digitalWrite0(0x00);

    return 0;
}
