#include <iostream>
#include <bitset>
#include <b15f/b15f.h>

B15F& setup(){
    B15F& drv = B15F::getInstance();
    drv.setRegister(&DDRA, 0b00001111);
    return drv;
}

uint8_t readData(B15F& drv){ 
    return (drv.getRegister(&PINA)); 
}

void writeData(B15F& drv, volatile uint8_t value){
    drv.setRegister(&PORTA, value);
}


int main(void){
    B15F& drv = setup();

    while(1){
        volatile uint8_t portA = drv.getRegister(&PORTA) ^ 0b1111;
        std::bitset<8> writingData(portA);
        //writeData(drv, portA);

        std::bitset<8> receivedData(readData(drv));
        std::cout << "received: " << receivedData << " | sent: " << writingData << std::endl;


        drv.delay_ms(500);
    }

    return 0;
}