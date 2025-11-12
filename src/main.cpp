#include <iostream>
#include <bitset>
#include <b15f/b15f.h>
// DECLARATIONS
B15F* setup();

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

void writeData(uint8_t value){
    drv->setRegister(&PORTA, value);
}

void writeData(std::bitset<4> value){
    drv->setRegister(&PORTA, value.to_ullong());
}

void exitHandler(int sig){
    writeData(0b0000);
    exit(sig);
}


std::vector<std::bitset<4>> stringToNibbles(std::string& input){
    std::vector<std::bitset<4>> stringNibbles;
    stringNibbles.reserve(3*input.size());

    for(size_t i = 0; i < input.size(); i++){
        uint8_t byteData = (uint8_t)input.at(i);
        
        std::bitset<4> nibble_lower(byteData & 0b00001111);
        std::bitset<4> nibble_upper((byteData & 0b11110000) >> 4);


        stringNibbles.push_back(SILLYSEQUENCE);
        stringNibbles.push_back(nibble_upper);
        stringNibbles.push_back(nibble_lower);
    }
    return stringNibbles;
}

void sendStringNibbles(std::vector<std::bitset<4>>& data){
    for(std::bitset<4> bits : data){
        writeData(bits);
    }
}


std::string readStringNibbles(){
    //TODO...
}


int main(void){ 
    signal(SIGINT, exitHandler);

    std::string inputString;

    std::cin >> inputString;
    std::cout << std::endl;

    std::vector<std::bitset<4>> data = stringToNibbles(inputString);

    for(size_t i = 0; i < data.size(); i++){
        std::bitset<4> nibble(data.at(i));
        if(i%3==0 && i!=0){
            std::cout << std::endl;
        }
        std::cout << nibble.test(3) << nibble.test(2) << nibble.test(1) << nibble.test(0);
    }

    //sendStringNibbles(data);
    
    return 0;
}