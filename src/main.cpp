#include <iostream>
#include <b15f/b15f.h>

B15F& setup(){
    B15F& drv = B15F::getInstance();
    drv.setRegister(&DDRA, 0b00001111);
    return drv;
}

uint8_t readData(B15F& drv){ 
    std::cout << drv.getRegister(&PORTA);
    return (drv.getRegister(&PORTA) >> 4); 
}

void writeData(B15F& drv, volatile uint8_t value){
    drv.setRegister(&PORTA, value);
}


int main(void){
    B15F& drv = setup();

    while(1){
        writeData(drv, drv.getRegister(&PORTA) ^ 0b1111);

        int8_t data = readData(drv);

        if(data){
            std::cout << (int)data << std::endl;
        }

        drv.delay_ms(500);
    }

    return 0;
}