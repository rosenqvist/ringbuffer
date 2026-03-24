#include "RingBufferCount.hpp"

#include <iostream>

int main() {
    RingBufferCount<int, 4> rb;

    rb.push(10);
    rb.push(20);
    rb.push(30);

    std::cout << "size: " << rb.size() << "\n";
    std::cout << "full: " << rb.full() << "\n";

    if (auto val = rb.pop()) {
        std::cout << "popped: " << *val << "\n";
    }

    std::cout << "size after pop: " << rb.size() << "\n";

    return 0;
}