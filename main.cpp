#include <iostream>
#include "BigInteger.cpp"

int main() {
    BigInteger<unsigned long long> a, b;
    std::cin >> a >> b;

    const int newBase1 = 3;

    BigInteger<uint64_t, newBase1> a1 = static_cast< BigInteger<uint64_t, newBase1> >(a);
    BigInteger<uint64_t, newBase1> b1 = static_cast< BigInteger<uint64_t, newBase1> >(b);
    std::cout << "In RADIX: " << newBase1 <<std::endl;
    std::cout << "a + b: " << a1 + b1 << std::endl;
    std::cout << "a - b: " << a1 - b1 << std::endl;
    std::cout << "a * b: " << a1 * b1 << std::endl;
    std::cout << "-----------------------"<< std::endl;

    const int newBase2 = 8;
    std::cout << "In RADIX: " << newBase2 <<std::endl;
    BigInteger<uint64_t, newBase2> a2 = static_cast< BigInteger<uint64_t, newBase2> >(a1);
    BigInteger<uint64_t, newBase2> b2 = static_cast< BigInteger<uint64_t, newBase2> >(b1);

    std::cout << "a + b: " << a2 + b2 << std::endl;
    std::cout << "a - b: " << a2 - b2 << std::endl;
    std::cout << "a * b: " << a2 * b2 << std::endl;
    std::cout << "-----------------------"<< std::endl;

    const int newBase3 = 212010;
    std::cout << "In RADIX: " << newBase3 <<std::endl;
    BigInteger<uint64_t, newBase3> a3 = static_cast< BigInteger<uint64_t, newBase3> >(a2);
    BigInteger<uint64_t, newBase3> b3 = static_cast< BigInteger<uint64_t, newBase3> >(b2);

    std::cout << "a + b: " << a3 + b3 << std::endl;
    std::cout << "a - b: " << a3 - b3 << std::endl;
    std::cout << "a * b: " << a3 * b3 << std::endl;
    std::cout << "-----------------------"<< std::endl;

    const int newBase4 = 2147483647;
    std::cout << "In RADIX: " << newBase4 <<std::endl;
    BigInteger<uint64_t, newBase4> a4 = static_cast< BigInteger<uint64_t, newBase4> >(a3);
    BigInteger<uint64_t, newBase4> b4 = static_cast< BigInteger<uint64_t, newBase4> >(b3);

    std::cout << "a + b: " << a4 + b4 << std::endl;
    std::cout << "a - b: " << a4 - b4 << std::endl;
    std::cout << "a * b: " << a4 * b4 << std::endl;
    std::cout << "-----------------------"<< std::endl;
    return 0;
}