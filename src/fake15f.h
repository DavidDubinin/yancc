// fake15f.h
#pragma once
// "Virtuelles" B15F damit man Code für's B15F schreiben kann
// und trotzdem zu Hause ohne B15F getestet werden kann :v

#include <cstdint>
#include <iostream>
#include <bitset>
#include <queue>
#include <csignal>
#include <chrono>
#include <ratio>
#include <sys/types.h>
#include <thread>
#include <functional>
#include <map>

volatile uint8_t __fake_PINA = 0;
volatile uint8_t __fake_PORTA = 0;
volatile uint8_t __fake_DDRA = 0;

volatile uint8_t __fake_PINB = 0;
volatile uint8_t __fake_PORTB = 0;
volatile uint8_t __fake_DDRB = 0;

volatile uint8_t __fake_PINC = 0;
volatile uint8_t __fake_PORTC = 0;
volatile uint8_t __fake_DDRC = 0;

#define PINA __fake_PINA
#define PORTA __fake_PORTA
#define DDRA __fake_DDRA

#define PINB __fake_PINB
#define PORTB __fake_PORTB
#define DDRB __fake_DDRB

#define PINC __fake_PINC
#define PORTC __fake_PORTC
#define DDRC __fake_DDRC

class B15F {
private:
    static B15F* instance;
    static B15F* peer;
    std::queue<uint8_t> rxSchlange;
    bool connected = false;
    std::chrono::steady_clock::time_point startTime;

    void receiveData(uint8_t daten) {
        rxSchlange.push(daten);
    }

public:
    B15F(bool isPeer = false) {
        startTime = std::chrono::steady_clock::now();
        if (isPeer) {}
    }

    static B15F& getInstance() {
        if (!instance) {
            instance = new B15F();
            std::thread([](){
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if (instance && !instance -> connected) {
                    instance -> peer = new B15F(true);
                    instance -> peer -> peer = instance;
                    instance -> connected = true;
                    instance -> peer -> connected = true;
                    std::cout << "[FAKE15] Verbindung hergestellt" << std::endl;
                }
            }).detach();
        }
        return *instance;
    }

    void setRegister(volatile uint8_t* reg, uint8_t value) {
        if (reg == &PORTA && peer && connected) {
            uint8_t txData = value & 0x0F;
            peer -> receiveData(txData);
        }
        if (reg == &PORTA) __fake_PORTA = value;
        else if (reg == &DDRA) __fake_DDRA = value;

        else if (reg == &PORTB) __fake_PORTB = value;
        else if (reg == &DDRB) __fake_DDRB = value;

        else if (reg == &PORTC) __fake_PORTC = value;
        else if (reg == &DDRC) __fake_DDRC = value;
    }

    uint8_t getRegister(volatile uint8_t* reg) {
        if (reg == &PINA)  {
            if (!rxSchlange.empty()) {
                uint8_t daten = rxSchlange.front();
                rxSchlange.pop();
                return daten << 4;
            }
            return 0;
        }
        else if (reg == &PORTA) return __fake_PORTA;
        else if (reg == &DDRA) return __fake_DDRA;

        else if (reg == &PINB) return __fake_PINB;
        else if (reg == &PORTB) return __fake_PORTB;
        else if (reg == &DDRB) return __fake_DDRB;

        else if (reg == &PINC) return __fake_PINC;
        else if (reg == &PORTC) return __fake_PORTC;
        else if (reg == &DDRC) return __fake_DDRC;

        return 0;
    }

    void delay_ms(uint16_t ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void delay_us(uint16_t us) {
        std::this_thread::sleep_for(std::chrono::microseconds(us));
    }

    void digitalWrite0(uint8_t value) {
        setRegister(&PORTA, value);
    }

    void digitalWrite1(uint8_t value) {
        setRegister(&PORTB, value);
    }

    uint8_t digitalRead0() {
        return getRegister(&PINA);
    }

    uint8_t digitalRead1() {
        return getRegister(&PINB);
    }

    uint16_t analogRead(uint8_t channel) {
        (void)channel;
        static uint16_t fakeValue = 512;
        fakeValue = (fakeValue + 74) % 1024; // 0-1023
        return fakeValue;
    }

    void analogWrite0(uint16_t value) { // basic glaub wir brauchen es eh nicht xD
        std::cout << "[FAKE15] analogWrite0: " << value << std::endl;
    }

    void analogWrite1(uint16_t value) {
        std::cout << "[FAKE15] analogWrite1: " << value << std::endl;
    }

    void flush() {
        while (!rxSchlange.empty()) {
            rxSchlange.pop();
        }
        std::cout << "[FAKE15] Buffer geleert" << std::endl;
    }

    void reverse(uint8_t& b) {
        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    }
};

B15F* B15F::instance = nullptr;
B15F* B15F::peer = nullptr;
