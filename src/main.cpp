#include <iostream>
#include "fake15f.h"
#include <cstdint>
#include <vector>

B15F& drv = B15F::getInstance();

#define TX_DATA_MASK 0x07
#define TX_CLOCK_BIT 0x08
#define RX_DATA_SHIFT 4

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

void pulseSymbol(uint8_t symbol) {
    uint8_t data = symbol & TX_DATA_MASK;
    uint8_t current = drv.getRegister(&PORTA);
    
    uint8_t out = (current & 0xF0) | data;
    drv.setRegister(&PORTA, out);
    drv.delay_us(50);
    
    out |= TX_CLOCK_BIT;
    drv.setRegister(&PORTA, out);
    drv.delay_us(50);
    
    out &= ~TX_CLOCK_BIT;
    drv.setRegister(&PORTA, out);
    drv.delay_us(50);
}

uint8_t sampleSymbol() {
    uint8_t pins = drv.getRegister(&PINA);
    return (pins >> RX_DATA_SHIFT) & TX_DATA_MASK;
}

void modulate(uint8_t byte) {
    uint8_t s0 = (byte >> 5) & 0x07;
    uint8_t s1 = (byte >> 2) & 0x07;
    uint8_t s2 = (byte & 0x03) | 0x04;
    
    pulseSymbol(s0);
    pulseSymbol(s1);
    pulseSymbol(s2);
}

uint8_t demodulate() {
    drv.delay_ms(2);
    uint8_t s0 = sampleSymbol();
    drv.delay_ms(2);
    uint8_t s1 = sampleSymbol();
    drv.delay_ms(2);
    uint8_t s2 = sampleSymbol();
    
    uint8_t byte = 0;
    byte |= (s0 & 0x07) << 5;
    byte |= (s1 & 0x07) << 2;
    byte |= (s2 & 0x03);
    return byte;
}

int main() {
    setup();
    
    std::string input;
    while(std::getline(std::cin, input)) {
        if (input.empty()) continue;
        
        for (char c : input) {
            modulate((uint8_t)c);
        }
        modulate('\n');
    }
    
    return 0;
}
