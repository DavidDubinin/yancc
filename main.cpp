#include <iostream>
#include <b15f/b15f.h>

int main(void){
    B15F& drv = B15F::getInstance();
    
    drv.setRegister(&DDRA, 0x1);

    while(1){
        drv.setRegister(&PORTA, drv.getRegister(&PORTA) ^ 0x1);
        drv.delay_ms(1000);
    }
   
    return 0;
}