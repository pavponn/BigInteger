//
// Created by Павел Пономарев on 07.03.2018.
//

#include "BigInteger.h"

const int STANDARD_RADIX = 1000000000;
const int MAX_DIGIT = 9;

template<typename T>
struct Pair {
    T high;
    T low;

    Pair() : high(0), low(0) {}

    Pair(T a, T b) : high(a), low(b) {}

    Pair(T x) : high(0), low(x) {}
};

///
/// Main constructor
///
template<typename T, size_t Base>
BigInteger<T, Base>::BigInteger(long long number) {
    negativeSign = number < 0;
    if (negativeSign) {
        number = -number;
    }
    sizeOfNumber = 0;
    do {
        setDigit(size(), number % Base);
        number = number / Base;
    } while (number > 0);
}

///
/// String constructor
///
template<typename T, size_t Base>
BigInteger<T, Base>::BigInteger(std::string argument) {
    bool negative = false;
    std::vector<uint32_t> bigDigits;
    if (!argument.empty() && (argument[0] == '-' || argument[0] == '+')) {
        negative = (argument[0] == '-');
        argument = argument.substr(1);
    }
    if (argument.empty()) {
        argument = "0";
    }
    u_long pos = std::min(argument.find_first_not_of('0'), argument.size() - 1);
    argument.erase(0, pos);
    if (negative && argument == "0") {
        negative = false;
    }

    for (int i = (int) argument.length(); i > 0; i -= MAX_DIGIT) {
        int32_t x = std::stoi(argument.substr(std::max(0, i - MAX_DIGIT), i - std::max(0, i - MAX_DIGIT)));
        bigDigits.push_back(x);
    }

    BigInteger<uint32_t, STANDARD_RADIX> temp(bigDigits);
    temp.setSign(negative);

    *this = static_cast<BigInteger<T, Base>>(temp);
}


template<typename T, size_t Base>
template<typename digitT>
BigInteger<T, Base>::BigInteger(std::vector<digitT> bigDigits) { ////Construct from vector
    sizeOfNumber = 0;
    negativeSign = false;
    for (auto i : bigDigits) {
        setDigit(size(), i % Base);
    }
}

/////
///// CASTTTING
/////

template<typename T, size_t Base>
template<typename NewT, size_t NewBase>
BigInteger<T, Base>::operator BigInteger<NewT, NewBase>() const {
    std::vector<NewT> newDigits;
    BigInteger<T, Base> temp(*this);
    do {
        std::vector<__int128_t> bigDigits;
        __int128_t numToUse = 0;
        for (int i = int(temp.size() - 1); i >= 0; --i) {
            numToUse = numToUse * Base + temp[i];
            if (numToUse >= NewBase) {
                bigDigits.push_back(numToUse / NewBase);
                numToUse %= NewBase;
            } else if (!bigDigits.empty()) {
                bigDigits.push_back(0);
            }
        }
        if (bigDigits.empty()) {
            bigDigits.push_back(0);
        }
        std::reverse(bigDigits.begin(), bigDigits.end());
        temp = BigInteger<T, Base>(bigDigits);
        temp.setSign(negativeSign);
        newDigits.push_back((NewT) numToUse);
    } while (!temp.isZero());

    BigInteger<NewT, NewBase> result(newDigits);
    result.setSign(negativeSign);
    return result;
}

/////
///// assignment, copy, movement
/////

template<typename T, size_t Base>
BigInteger<T, Base> &BigInteger<T, Base>::operator=(const BigInteger<T, Base> &argument) {
    negativeSign = argument.negativeSign;
    number = argument.number;
    sizeOfNumber = argument.sizeOfNumber;
    return *this;
}

template<typename T, size_t Base>
BigInteger<T, Base>::BigInteger(const BigInteger<T, Base> &argument) {
    number = argument.number;
    negativeSign = argument.negativeSign;
    sizeOfNumber = argument.sizeOfNumber;
}

template<typename T, size_t Base>
BigInteger<T, Base>::BigInteger(BigInteger<T, Base> &&argument) {
    negativeSign = argument.negativeSign;
    number = std::move(argument.number);
    sizeOfNumber = argument.sizeOfNumber;
}


/////
///// comparision
/////

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator<(const BigInteger &argument) const {
    if (negativeSign != argument.negativeSign) {
        return negativeSign;
    }
    if ((*this).size() != argument.size()) {
        return (*this).size() < argument.size();
    }

    for (int i = int(argument.size()) - 1; i >= 0; i--) {
        if ((*this)[i] != argument[i]) {
            return (*this)[i] * (!negativeSign ? 1 : -1) < argument[i] * (!argument.negativeSign ? 1 : -1);
        }
    }
    return false;
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator>(const BigInteger &argument) const {
    return argument < *this;
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator==(const BigInteger &argument) const {
    return !(argument < *this) && !(argument > *this);
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator!=(const BigInteger &argument) const {
    return !(*this == argument);
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator>=(const BigInteger &argument) const {
    return *this > argument || *this == argument;
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator<=(const BigInteger &argument) const {
    return *this < argument || *this == argument;
}


/////
///// Arithmetic operations
/////


template<typename T, size_t Base>
Pair<T> safeAdd(Pair<T> a, Pair<T> b) {
    Pair<T> ans;
    if (a.low >= T(Base) - b.low) {
        ans.low += a.low - (Base - b.low);
        ans.high += 1;
    } else {
        ans.low += a.low + b.low;
    }
    ans.high += a.high + b.high;
    return ans;
}

template<typename T, size_t Base>
Pair<T> safeMultiply(T a, T b) {
    Pair<T> ans, curA(a);
    while (b) {
        if (b & 1) {
            ans = safeAdd<T, Base>(ans, curA);
        }
        curA = safeAdd<T, Base>(curA, curA);
        b >>= 1;
    }
    return ans;
}

template<typename T, size_t Base>
BigInteger<T, Base> &BigInteger<T, Base>::operator*=(const BigInteger<T, Base> &argument) {
    bool negativeSign = isNegative() ^argument.isNegative();
    std::vector<T> multiplyResult(size() + argument.size(), 0);
    std::vector<T> first(size());
    std::vector<T> second(argument.size());

    ////Copy vectors
    for (size_t i = 0; i < size(); ++i) {
        first[i] = (*this)[i];
    }
    for (size_t i = 0; i < argument.size(); ++i) {
        second[i] = argument[i];
    }
    ///

    for (size_t i = 0; i < first.size(); ++i) {
        T carryDigit = 0;
        for (size_t j = 0; j < second.size() || carryDigit; ++j) {
            Pair<T> current = safeAdd<T, Base>(safeAdd<T, Base>(multiplyResult[i + j], carryDigit),
                                               safeMultiply<T, Base>(first[i], j < second.size() ? second[j] : 0));
            multiplyResult[i + j] = current.low;
            carryDigit = current.high;
        }
    }

    BigInteger<T, Base> result(multiplyResult);
    result.trim();
    result.setSign(negativeSign);

    return *this = result;
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator*(const BigInteger<T, Base> &argument) const {
    return BigInteger(*this) *= argument;
}

template<typename T, size_t Base>
BigInteger<T, Base> &BigInteger<T, Base>::operator+=(const BigInteger<T, Base> &argument) {
    if (negativeSign == argument.negativeSign) {
        __int128_t carry = 0;
        for (size_t i = 0; i < argument.size() || carry; ++i) {
            __int128_t first = i < size() ? (*this)[i] : 0;
            __int128_t second = i < argument.size() ? argument[i] : 0;

            __int128_t current;
            if (first >= Base - second - carry) {
                current = first - (Base - second - carry);
                carry = 1;
            } else {
                current = first + second + carry;
                carry = 0;
            }
            setDigit(i, current);
        }
    } else if (negativeSign && !argument.negativeSign) {
        *this = (argument) + (*this);
    } else if (number.size() < argument.number.size() ||
               number.size() == argument.number.size() && abs() < argument.abs()) {
        *this = -(-argument + -(*this));
    } else {
        __int128_t carryDigit = 0;
        for (size_t i = 0; i < argument.size() || carryDigit; ++i) {
            __int128_t first = (*this)[i];
            __int128_t second;
            if (i < argument.size()) {
                second = argument[i];
            } else {
                second = 0;
            }
            if (carryDigit > first) {
                first += Base - carryDigit;
                carryDigit = 1;
            } else {
                first -= carryDigit;
                carryDigit = 0;
            }
            __int128_t currentDigit;
            if (first < second) {
                currentDigit = first + (Base - second);
                carryDigit = 1;
            } else {
                currentDigit = first - second;
            }
            setDigit(i, currentDigit);
        }

        trimZeros();

        size_t usedBlocks = (size_t) std::ceil(double(size()) / digitsInType());
        if (usedBlocks < number.size()) {
            number.resize(usedBlocks);
        }
    }

    return *this;
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator+(const BigInteger<T, Base> &argument) const {
    return BigInteger<T, Base>(*this) += argument;
}

template<typename T, size_t Base>
BigInteger<T, Base> &BigInteger<T, Base>::operator-=(const BigInteger<T, Base> &argument) {
    return *this += (-argument);
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator-(const BigInteger<T, Base> &argument) const {
    return BigInteger(*this) -= argument;
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator-() const {
    BigInteger temp = *this;
    temp.negativeSign = !(negativeSign);
    return temp;
}

/////
///// auxiliary functions
/////

template<typename T, size_t Base>
void BigInteger<T, Base>::trimZeros() {
    for (size_t i = size() - 1; !isZero() && i >= 0 && !(*this)[i]; --i) {
        --sizeOfNumber;
    }
}

template<typename T, size_t Base>
void BigInteger<T, Base>::trim() {
    trimZeros();
    size_t usedBlocks = std::ceil(double(size()) / digitsInType());
    if (usedBlocks < number.size()) {
        number.resize(usedBlocks);
    }
}

template<typename T, size_t Base>
size_t BigInteger<T, Base>::size() const {
    return sizeOfNumber;
}

template<typename T, size_t Base>
T BigInteger<T, Base>::operator[](size_t i) const {
    T result = number[i / digitsInType()];
    result >>= (i % digitsInType()) * bitsUsedForOneDigit();
    if (digitsInType() == 1) {
        return result % Base;
    } else {
        return result & ((1 << bitsUsedForOneDigit()) - 1);
    }
}

template<typename T, size_t Base>
void BigInteger<T, Base>::setDigit(size_t pos, T digit) { ///change or put on  digit
    if (pos >= size()) {
        for (size_t t = size(); t <= pos; ++t) {
            if (t % digitsInType() == 0) {
                number.push_back(0);
            }
            ++sizeOfNumber;
        }
    }
    size_t j = pos / digitsInType();
    if (digitsInType() == 1) {
        number[j] = digit % Base;
    } else {
        size_t shift = (pos % digitsInType()) * bitsUsedForOneDigit();
        T mask = (1 << bitsUsedForOneDigit()) - 1;
        digit &= mask;

        mask = ~(mask << shift);
        number[j] &= mask;
        number[j] |= digit << shift;
    }
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::abs() const {
    BigInteger res = *this;
    if (isNegative()) {
        res.setSign(false);
    }
    return res;
}


template<typename T, size_t Base>
bool BigInteger<T, Base>::isNegative() const {
    return negativeSign;
}

template<typename T, size_t Base>
void BigInteger<T, Base>::setSign(bool negative) {
    if (!isZero()) {
        negativeSign = negative;
    }
}


template<typename T, size_t Base>
bool BigInteger<T, Base>::isZero() const {
    return size() == 1 && !(*this)[0];
}

template<typename T, size_t Base>
constexpr int BigInteger<T, Base>::digitsInType() {
    return (sizeof(T) * 8) / bitsUsedForOneDigit();
}

template<typename T, size_t Base>
constexpr int BigInteger<T, Base>::bitsUsedForOneDigit() {
    return (int) std::ceil(std::log2(Base));
}

///
/// Input-output streams
///

template<typename T, size_t Base>
std::ostream &operator<<(std::ostream &stream, const BigInteger<T, Base> &bigInteger) {
    if (bigInteger.negativeSign) {
        stream << '-';
    }

    BigInteger<uint32_t, STANDARD_RADIX> current = static_cast<BigInteger<uint32_t, STANDARD_RADIX>>(bigInteger);

    stream << current[current.size() - 1];

    for (int i = current.size() - 2; i >= 0; --i) {
        stream << std::setfill('0') << std::setw(MAX_DIGIT) << current[i];
    }
    return stream;
}

template<typename T, size_t Base>
std::istream &operator>>(std::istream &stream, BigInteger<T, Base> &bigInteger) {
    std::string current;
    stream >> current;
    bigInteger = BigInteger<T, Base>(current);
    return stream;
}
