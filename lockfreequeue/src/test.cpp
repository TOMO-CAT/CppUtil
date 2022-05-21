#include <iostream>

int main() {
    int N = 10;

    for (int i = 0; i < 100; i++) {
        std::cout << i << ": " << (i % 9) << std::endl;
    }
}