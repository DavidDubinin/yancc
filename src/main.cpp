#include <iostream>
#include <bitset>
#include <b15f/b15f.h>


constexpr std::bitset<4> SILLYSEQUENCE(1010);

B15F& setup(){
    B15F& drv = B15F::getInstance();
    drv.setRegister(&DDRA, 0b00001111);
    return drv;
}


B15F& drv = setup();

uint8_t readNibble(B15F& drv){ 
    return (drv.getRegister(&PINA) >> 4); 
}

std::bitset<4> readNibbleBitset(B15F& drv){
    return std::bitset<4>(drv.getRegister(&PINA) >> 4);
}

void writeData(B15F& drv, volatile uint8_t value){
    drv.setRegister(&PORTA, value);
}

void writeData(B15F& drv, std::bitset<4> value){
    drv.setRegister(&PORTA, value.to_ullong());
}

void exitHandler(int sig){
    writeData(drv,0b0000);
    exit(sig);
}


std::vector<std::bitset<4>> stringToNibble(std::string& input){
    std::vector<std::bitset<4>> stringNibbles;
    stringNibbles.reserve(3*input.size());

    for(size_t i = 0; i < input.size(); i++){
        uint8_t byteData = (uint8_t)input.at(i);
        std::bitset<4> nibble_lower(byteData & 0b00001111 );
        std::bitset<4> nibble_upper((byteData & 0b11110000) >> 4);

        stringNibbles.push_back(SILLYSEQUENCE);
        stringNibbles.push_back(nibble_lower);
        stringNibbles.push_back(nibble_upper);
    }
    return stringNibbles;
}

void sendStringNibbles(std::vector<std::bitset<4>>& data){
    for(std::bitset<4> bits : data){
        writeData(drv, bits);
    }
}


int main(void){ 
    signal(SIGINT, exitHandler);

    std::string inputString;

    std::cin >> inputString;

    std::vector<std::bitset<4>> data = stringToNibble(inputString);

    sendStringNibbles(data);
    
    

    return 0;
}