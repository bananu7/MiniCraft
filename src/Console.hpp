#pragma once
#include <glm/glm.hpp>
#include <functional>
#include <deque>

class Console {
    Console(Console const&)/* = delete*/;
    Console& operator= (Console const&)/* = delete*/;

    glm::ivec2 cursor;
    glm::ivec2 bufferSize;

    std::string inputBuffer;
    std::deque<std::string> buffer;

    std::function<void(std::string const&)> enterCallback;

public:
    void write(std::string s) {
        buffer.emplace_front(std::move(s));
    }

    void inputCharacter (char c) {
        inputBuffer += c;
    }
    // Reasoning behind creating different functions : 
    // 1. Enter/Bksp key can be os-specific
    // 2. User might want to have an enter/bksp button
    void enter () {
        buffer.push_front(inputBuffer);

        if (enterCallback)
            enterCallback(inputBuffer);

        inputBuffer = "";
    }
    void backspace () {
        if (!inputBuffer.empty())
            inputBuffer.pop_back();
    }
    void setCallback(std::function<void(std::string)> cb) {
        enterCallback = std::move(cb);
    }
    
    std::string const& getInputBuffer() const { return inputBuffer; }
    std::deque<std::string> const& getBuffer() const { return buffer; }

    Console(glm::ivec2 bufSize) : bufferSize(std::move(bufSize)) {
    }
    /*Console(Console&& other) {
    }
    Console& operator=(Console&& other) {
        return *this;
    }*/
};