#include "immu/vector.h"
#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;

    immu::vector<int> a;
    for (int i = 0; i < 5; ++i)
        a = std::move(a).push_back(i);
    auto b = a.push_back(10);
    immu::vector<int> c(1, 100000);
    auto d = b.push_back(7)
            .push_back(9)
            .push_back(12);
    std::cout << a[4] << "\n";
    std::cout << b[3] << "\n";
    return 0;
}