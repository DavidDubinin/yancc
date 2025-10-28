#include <iostream>
#include <bitset>
#include <b15f/b15f.h>

B15F& drv = setup();

B15F& setup(){
    B15F& drv = B15F::getInstance();
    drv.setRegister(&DDRA, 0b00001111);
    return drv;
}

uint8_t readData(B15F& drv){ 
    return (drv.getRegister(&PINA) >> 4); 
}

void writeData(B15F& drv, volatile uint8_t value){
    drv.setRegister(&PORTA, value);
}

void exitHandler(int sig){
    writeData(drv,0b0000);
    exit(sig);

}

int main(void){

    signal(SIGINT, exitHandler);

    while(1){
        volatile uint8_t portA = drv.getRegister(&PORTA) ^ 0b1111;
        std::bitset<4> writingData(portA);
        writeData(drv, portA);

        std::bitset<4> receivedData(readData(drv));
        std::cout << "received: " << receivedData << " | sent: " << writingData << std::endl;


        drv.delay_ms(500);
    }

    return 0;
}