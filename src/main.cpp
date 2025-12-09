#include <iostream>
#include <b15f/b15f.h>
#include <cstdint>
#include <vector>

B15F& drv = B15F::getInstance();
void setup() {
    drv.setRegister(&DDRA, 0x0F);
    drv.delay_ms(100);
}


std::vector<uint8_t> charToNibbles(char ch) {
    std::vector<uint8_t> nibbles;
    uint8_t byte = (uint8_t)ch;

    uint8_t upperNibble = (byte >> 4) & 0x0F;
    uint8_t lowerNibble = byte & 0x0F;

    nibbles.push_back(upperNibble);
    nibbles.push_back(lowerNibble);

    return nibbles;
}


std::vector<uint8_t> stringToNibbles(const std::string& text) {
    std::vector<uint8_t> allNibbles;

    for (char ch : text) {
        std::vector<uint8_t> nibbles = charToNibbles(ch);
        allNibbles.insert(allNibbles.end(), nibbles.begin(), nibbles.end());
    }

    allNibbles.push_back(0x0F);
    return allNibbles;
}


void sendNibble(uint8_t nibble) {
    uint8_t current = drv.getRegister(&PORTA);
    uint8_t msg = (current & 0xF0) | (nibble & 0x0F);
    drv.setRegister(&PORTA, msg);
    drv.delay_ms(2);
}


int main() {
    std::cout << "FUNKTIONIERE BITTE" << std::endl;
    setup();

    while(1) {
        // TODO: Schleife, die nach Input fragt und string in Nibble umwandelt. Danach kommt das Senden usw.
        std::cout << "Geben Sie eine Nachricht an: " << std::endl;
        std::string input;
        std::getline(std::cin, input);
    }
}
