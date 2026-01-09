#include <iostream>
//#include "fake15f.h"
#include <b15f/b15f.h>
#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <chrono>
#include <condition_variable>

B15F& drv = B15F::getInstance();

constexpr uint8_t TX_DATA_BITS = 0x07;
constexpr uint8_t TX_CLOCK_BIT = 0x08;
constexpr uint8_t RX_DATA_BITS = 0x70;
constexpr uint8_t RX_CLOCK_BIT = 0x80;
constexpr uint8_t RX_DATA_SHIFT = 4;
constexpr uint8_t OUTPUT_MASK = 0x0F;
constexpr uint8_t INPUT_MASK = 0xF0;

enum class FrameType : uint8_t {
    START = 0xE0,
    END = 0xE1,
    ACK = 0xE2,
    NAK = 0xE3
};

constexpr size_t BLOCK_SIZE = 64;

std::mutex drvMutex;
std::atomic<bool> running(true);
std::atomic<bool> rxReady(false);

struct Block {
    uint8_t seqNum;
    std::vector<uint8_t> data;
};

void setup() {
    drv.setRegister(&DDRA, OUTPUT_MASK);
    drv.delay_ms(100);
}

void clear() {
    std::lock_guard<std::mutex> lock(drvMutex);
    drv.setRegister(&PORTA, 0x00);
    drv.delay_ms(50);
}

uint8_t computeCRC8(const uint8_t* data, size_t length) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

bool waitForRisingEdge() {
    constexpr int MAX_ATTEMPTS = 10000;
    int attempts = 0;

    uint8_t currentState = 0;
    {
        std::lock_guard<std::mutex> lock(drvMutex);
        currentState = drv.getRegister(&PINA) & RX_CLOCK_BIT;
    }

    while (currentState != 0 && attempts < MAX_ATTEMPTS && running) {
        drv.delay_ms(1);
        std::lock_guard<std::mutex> lock(drvMutex);
        currentState = drv.getRegister(&PINA) & RX_CLOCK_BIT;
        attempts++;
    }

    if (attempts >= MAX_ATTEMPTS) return false;

    attempts = 0;
    while (currentState == 0 && attempts < MAX_ATTEMPTS && running) {
        drv.delay_ms(1);
        std::lock_guard<std::mutex> lock(drvMutex);
        currentState = drv.getRegister(&PINA) & RX_CLOCK_BIT;
        attempts++;
    }

    return attempts < MAX_ATTEMPTS;
}

void sendSymbolClocked(uint8_t symbol) {
    std::lock_guard<std::mutex> lock(drvMutex);
    uint8_t data = symbol & TX_DATA_BITS;
    uint8_t current = drv.getRegister(&PORTA);

    uint8_t out = (current & INPUT_MASK) | data;
    drv.setRegister(&PORTA, out);
    drv.delay_us(50);

    out |= TX_CLOCK_BIT;
    drv.setRegister(&PORTA, out);
    drv.delay_us(50);

    out &= ~TX_CLOCK_BIT;
    drv.setRegister(&PORTA, out);
    drv.delay_us(50);
}

uint8_t recvSymbolClocked() {
    if (!waitForRisingEdge()) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(drvMutex);
    uint8_t pins = drv.getRegister(&PINA);
    uint8_t symbol = (pins & RX_DATA_BITS) >> RX_DATA_SHIFT;
    return symbol;
}

void encodeByte(uint8_t byte) {
    uint8_t s0 = (byte >> 5) & 0x07;
    uint8_t s1 = (byte >> 2) & 0x07;
    uint8_t s2 = (byte & 0x03) | 0x04;

    sendSymbolClocked(s0);
    sendSymbolClocked(s1);
    sendSymbolClocked(s2);
}

uint8_t decodeByte() {
    uint8_t s0 = recvSymbolClocked();
    uint8_t s1 = recvSymbolClocked();
    uint8_t s2 = recvSymbolClocked();

    uint8_t byte = 0;
    byte |= (s0 & 0x07) << 5;
    byte |= (s1 & 0x07) << 2;
    byte |= (s2 & 0x03);
    return byte;
}

bool isControlFrame(uint8_t byte) {
    return (byte == static_cast<uint8_t>(FrameType::START) ||
            byte == static_cast<uint8_t>(FrameType::END) ||
            byte == static_cast<uint8_t>(FrameType::ACK) ||
            byte == static_cast<uint8_t>(FrameType::NAK));
}

void sendBlock(const Block& block) {
    bool acked = false;
    int retries = 0;
    constexpr int MAX_RETRIES = 5;

    while (!acked && retries < MAX_RETRIES && running) {
        encodeByte(block.seqNum);
        encodeByte(static_cast<uint8_t>(block.data.size()));

        for (uint8_t byte : block.data) {
            encodeByte(byte);
        }

        uint8_t crc = computeCRC8(block.data.data(), block.data.size());
        encodeByte(crc);

        auto startTime = std::chrono::steady_clock::now();
        bool timedOut = false;

        while (!timedOut && running) {
            if (waitForRisingEdge()) {
                uint8_t response = decodeByte();

                if (response == static_cast<uint8_t>(FrameType::ACK)) {
                    acked = true;
                    break;
                } else if (response == static_cast<uint8_t>(FrameType::NAK)) {
                    retries++;
                    std::cerr << "[TX] NAK empfangen, retry " << (retries) << "\n";
                    break;
                }
            }

            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
            if (elapsed.count() > 2000) {
                timedOut = true;
                retries++;
                std::cerr << "[TX] Timeout, retry " << retries << "\n";
            }
        }
    }

    if (retries >= MAX_RETRIES) {
        std::cerr << "[TX] Max retries für block " << (int)block.seqNum << "\n";
    }
}

std::mutex blockQueueMutex;
std::condition_variable blockQueueCV;
std::queue<Block> blockQueue;

void transmitThread() {
    std::cerr << "[TX] Thread startet\n";

    while (running) {
        std::unique_lock<std::mutex> lock(blockQueueMutex);
        blockQueueCV.wait(lock, []{ return !blockQueue.empty() || !running; });

        if (!running && blockQueue.empty()) break;

        if (!blockQueue.empty()) {
            Block block = blockQueue.front();
            blockQueue.pop();
            lock.unlock();

            if (isControlFrame(block.seqNum)) {
                encodeByte(block.seqNum);
            } else {
                sendBlock(block);
            }
        }
    }

    std::cerr << "[TX] Thread stoppt\n";
}

void receiveThread() {
    std::cerr << "[RX] Thread startet\n";
    rxReady = true;

    std::vector<uint8_t> buffer;
    bool inTransmission = false;

    while (running) {
        if (!waitForRisingEdge()) {
            continue;
        }

        uint8_t byte = decodeByte();

        if (byte == static_cast<uint8_t>(FrameType::START)) {
            inTransmission = true;
            buffer.clear();
            std::cerr << "[RX] Transmision startet\n";
            continue;
        }

        if (byte == static_cast<uint8_t>(FrameType::END)) {
            inTransmission = false;
            std::cerr << "[RX] Transmision endet, " << buffer.size() << " bytes\n";

            for (uint8_t b : buffer) {
                std::cout << (char)b;
            }
            std::cout.flush();
            buffer.clear();
            continue;
        }

        if (inTransmission && !isControlFrame(byte)) {
            uint8_t seqNum = byte;

            if (!waitForRisingEdge() || !running) break;
            uint8_t blockSize = decodeByte();

            std::vector<uint8_t> blockData;
            blockData.reserve(blockSize);

            for (uint8_t i = 0; i < blockSize; i++) {
                if (!waitForRisingEdge() || !running) break;
                uint8_t dataByte = decodeByte();
                blockData.push_back(dataByte);
            }

            if (!waitForRisingEdge() || !running) break;
            uint8_t receivedCRC = decodeByte();
            uint8_t computedCRC = computeCRC8(blockData.data(), blockData.size());

            if (receivedCRC == computedCRC && blockData.size() == blockSize) {
                buffer.insert(buffer.end(), blockData.begin(), blockData.end());
                encodeByte(static_cast<uint8_t>(FrameType::ACK));
            } else {
                std::cerr << "[RX] Block " << (int)seqNum << " CRC fehler\n";
                encodeByte(static_cast<uint8_t>(FrameType::NAK));
            }
        }
    }

    std::cerr << "[RX] Thread wird gestoppt :wilted_rose:\n";
}

void enqueueBlock(const Block& block) {
    std::lock_guard<std::mutex> lock(blockQueueMutex);
    blockQueue.push(block);
    blockQueueCV.notify_one();
}

int main() {
    setup();

    std::cerr << "[YANCC] Netcats Doppelgänger wird gestartet\n";

    std::thread rxThread(receiveThread);

    while (!rxReady) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::thread txThread(transmitThread);

    Block startBlock;
    startBlock.seqNum = static_cast<uint8_t>(FrameType::START);
    enqueueBlock(startBlock);

    std::vector<uint8_t> buffer;
    char c;
    while (std::cin.read(&c, 1)) {
        buffer.push_back((uint8_t)c);
    }

    size_t totalBytes = buffer.size();
    size_t bytesSent = 0;
    uint8_t seqNum = 0;

    while (bytesSent < totalBytes) {
        size_t blockSize = std::min((size_t)BLOCK_SIZE, totalBytes - bytesSent);

        Block block;
        block.seqNum = seqNum;
        block.data.reserve(blockSize);

        for (size_t i = 0; i < blockSize; i++) {
            block.data.push_back(buffer[bytesSent + i]);
        }

        enqueueBlock(block);

        bytesSent += blockSize;
        seqNum++;

        if (bytesSent % 1024 == 0 || bytesSent == totalBytes) {
            std::cerr << "[MAIN] " << bytesSent << " / " << totalBytes << " bytes\n";
        }
    }

    Block endBlock;
    endBlock.seqNum = static_cast<uint8_t>(FrameType::END);
    enqueueBlock(endBlock);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    running = false;
    blockQueueCV.notify_all();

    if (txThread.joinable()) txThread.join();
    if (rxThread.joinable()) rxThread.join();

    clear();

    std::cerr << "[YANCC] windows xp wird geruntergerfahren\n";

    return 0;
}
