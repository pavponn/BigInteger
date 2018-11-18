//
// Created by Павел Пономарев on 07.03.2018.
//

#ifndef BIGINTEGER_H
#define BIGINTEGER_H

#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <cmath>



template<typename T, size_t Base = std::numeric_limits<T>::max()>
class BigInteger;

template<typename T, size_t Base>
std::ostream & operator <<(std::ostream &, const BigInteger<T, Base> &);
template<typename T, size_t Base>
std::istream & operator >>(std::istream &, BigInteger<T, Base> &);

template<typename T, size_t Base>
class BigInteger {
    friend std::ostream & operator << <>(std::ostream &, const BigInteger<T, Base> &);
    friend std::istream & operator >> <>(std::istream &, BigInteger<T, Base> &);
    template<typename T1, size_t Base1> friend class BigInteger;

public:
    explicit BigInteger(long long number = 0);
    explicit BigInteger(std::string argument);
    template<typename digitT>
    explicit BigInteger(std::vector<digitT> bigDigits);
    template<typename NewT, size_t NewBase>
    explicit operator BigInteger<NewT, NewBase>() const;

    BigInteger(const BigInteger &);
    BigInteger(BigInteger &&);
    BigInteger& operator =(const BigInteger &);

    BigInteger & operator +=(const BigInteger &);
    BigInteger operator +(const BigInteger &) const;

    BigInteger & operator *=(const BigInteger &);
    BigInteger operator *(const BigInteger &) const;

    BigInteger & operator -=(const BigInteger &);
    BigInteger operator -(const BigInteger &) const;
    BigInteger operator -() const;

    bool operator<(const BigInteger &) const;
    bool operator>(const BigInteger &) const;
    bool operator<=(const BigInteger &) const;
    bool operator>=(const BigInteger &) const;
    bool operator==(const BigInteger &) const;
    bool operator!=(const BigInteger &) const;

    T operator[](size_t i) const;

    size_t size() const;
    BigInteger abs() const;
    bool isNegative() const;
    void setSign(bool negative = true);
    bool isZero() const;


private:
    void trimZeros();
    void trim();
    void setDigit(size_t pos, T digit);
    constexpr static int digitsInType();
    constexpr static int bitsUsedForOneDigit();
    std::vector<T> number;
    size_t sizeOfNumber;
    bool negativeSign;
};



#endif // BIGINTEGER_H