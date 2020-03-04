#include "immu/vector.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "Hello, World!" << std::endl;

    auto start = std::chrono::steady_clock::now();
    immu::vector<int> c(1, 1000000);
    auto end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "\n";
    immu::vector<int> a;
    for (int i = 0; i < 5; ++i)
        a = std::move(a).push_back(i);
    auto b = a.push_back(10);
    auto d = b.push_back(7)
            .push_back(9)
            .push_back(12);
    std::cout << a[4] << "\n";
    std::cout << b[3] << "\n";
    return 0;
}