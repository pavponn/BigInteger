#include "biginteger.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <cmath>

//
// Class implementation
//

const int IO_RADIX = 1000000000;

template<typename T, size_t Base>
template<typename T1, class>
BigInteger<T, Base>::BigInteger(T1 value) : m_negative(value < 0)
{
    if (m_negative) {
        value = -value;
    }

    m_digits = 0;

    do {
        setDigit(size(), value % Base);
        value /= Base;
    } while (value);
}

/*template<typename T, size_t Base>
template<typename NewT, size_t NewBase>
BigInteger<T, Base>::operator BigInteger<NewT, NewBase>() const
{
    BigInteger<NewT, NewBase> temp(0), deg(1), base(Base);
    for (size_t i = 0; i < size(); ++i) {
        temp += deg * BigInteger<NewT, NewBase>(getDigit(i));
        deg *= base;
    }
    temp.setNegative(isNegative());
    return temp;
}*/

template<typename T, size_t Base>
template<typename NewT, size_t NewBase>
BigInteger<T, Base>::operator BigInteger<NewT, NewBase>() const
{
    if (std::is_same<NewT, T>::value && NewBase == Base) {
        return BigInteger<NewT, NewBase>(*this);
    }

    vector<NewT> newDigits;
    BigInteger<T, Base> temp(*this);

    do {
        __uint128_t collect = 0;

        vector<__uint128_t> sparseDigits;

        for (int i = temp.size() - 1; i >= 0; --i) {
            collect = collect * Base + temp.getDigit(i);

            if (collect >= NewBase) {
                sparseDigits.push_back(collect / NewBase);
                collect %= NewBase;
            } else if (!sparseDigits.empty()){
                sparseDigits.push_back(0);
            }
        }

        if (sparseDigits.empty()) {
            sparseDigits.push_back(0);
        }

        std::reverse(sparseDigits.begin(), sparseDigits.end());
        temp = BigInteger<T, Base>(sparseDigits);
        temp.setNegative(m_negative);

        newDigits.push_back((NewT)collect);
    } while (!temp.isZero());

    BigInteger<NewT, NewBase> result(newDigits);
    result.setNegative(m_negative);
    return result;
}

template<typename T, size_t Base>
BigInteger<T, Base>::BigInteger(string value)
{
    bool negative = false;

    if (!value.empty() && (value[0] == '-' || value[0] == '+')) {
        negative = (value[0] == '-');
        value = value.substr(1);
    }

    if (value.empty()) {
        value = "0";
    }

    for (auto c : value) {
        if (c < '0' || c > '9') {
            throw std::ios_base::failure("Provided string is not a number");
        }
    }

    // Trim leading zeros
    value.erase(0, std::min(value.find_first_not_of('0'), value.size()-1));

    // 0 (zero) is not negative
    if (negative && value == "0") {
        negative = false;
    }

    vector<uint32_t> sparseDigits;
    for (int i = value.length(); i > 0; i -= 9) {
        int32_t x = std::stoi(value.substr(std::max(0, i - 9), i - std::max(0, i - 9)));
        sparseDigits.push_back(x);
    }

    BigInteger<uint32_t, IO_RADIX> temp(sparseDigits);
    temp.setNegative(negative);

    *this = static_cast<BigInteger<T, Base>>(temp);
}

template<typename T, size_t Base>
template<typename digitT>
BigInteger<T, Base>::BigInteger(vector<digitT> sparseDigits) : m_digits(0), m_negative(false)
{
    for (auto i : sparseDigits) {
        setDigit(size(), i % Base);
    }
}

template<typename T, size_t Base>
BigInteger<T, Base>::BigInteger(const BigInteger<T, Base> & bigint)
{
    m_negative = bigint.m_negative;
    m_number = bigint.m_number;
    m_digits = bigint.m_digits;
}

template<typename T, size_t Base>
BigInteger<T, Base>::BigInteger(BigInteger<T, Base> && bigint)
{
    m_negative = bigint.m_negative;
    m_number = std::move(bigint.m_number);
    m_digits = bigint.m_digits;
}

template<typename T, size_t Base>
BigInteger<T, Base>& BigInteger<T, Base>::operator =(const BigInteger<T, Base> & other)
{
    m_negative = other.m_negative;
    m_number = other.m_number;
    m_digits = other.m_digits;
    return *this;
}

template<typename T, size_t Base>
BigInteger<T, Base> & BigInteger<T, Base>::operator +=(const BigInteger<T, Base> &other)
{
    if (isNegative() == other.isNegative()) {
        // neg + neg or pos + pos;
        T carry = 0;

        for (size_t i = 0; i < other.size() || carry; ++i) {
            T a = i < size() ? getDigit(i) : 0;
            T b = i < other.size() ? other.getDigit(i) : 0;

            T current;
            if (a >= T(Base) - b - carry) {
                current = a - (Base - b - carry);
                carry = 1;
            } else {
                current = a + b + carry;
                carry = 0;
            }

            setDigit(i, current);
        }
    } else if (isNegative() && !other.isNegative()) {
        // neg + pos = pos + neg
        *this = (other) + (*this);
    } else if (other.abs() > (*this).abs()) {
        // pos + neg, where abs(pos) < abs(neg)
        // Swap to substract smaller from bigger
        *this = -(-other + -(*this));
    } else {
        // pos + neg = subtraction
        T carry = 0;

        for (size_t i = 0; i < other.size() || carry; ++i) {
            T a = getDigit(i);
            T b = i < other.size() ? other.getDigit(i) : 0;

            if (carry > a) {
                a += Base - carry;
                carry = 1;
            } else if (carry) {
                a -= carry;
                carry = 0;
            }

            if (b > a) {
                a += Base - b;
                carry = 1;
            } else {
                a -= b;
            }

            setDigit(i, a);
        }
        trim();
    }

    return *this;
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator +(const BigInteger<T, Base> &other) const
{
    return BigInteger<T, Base>(*this) += other;
}

template<typename T, size_t Base>
BigInteger<T, Base> & BigInteger<T, Base>::operator -=(const BigInteger<T, Base> &other)
{
    return *this += (-other);
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator -(const BigInteger<T, Base> &other) const
{
    return BigInteger(*this) -= other;
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator -() const
{
    BigInteger temp = *this;
    temp.m_negative = !(m_negative);
    return temp;
}

template<typename T>
struct CarryNum
{
    T h, l;
    CarryNum() : h(0), l(0) {}
    CarryNum(T x) : h(0), l(x) {}
};

template<typename T, size_t Base>
inline CarryNum<T> add(CarryNum<T> a, CarryNum<T> b)
{
    CarryNum<T> temp;
    if (a.l >= T(Base) - b.l) {
        temp.l += a.l - (Base - b.l);
        temp.h += 1;
    } else {
        temp.l += a.l + b.l;
    }
    temp.h += a.h + b.h;
    return temp;
}

template<typename T, size_t Base>
inline CarryNum<T> multiply(T a, T b)
{
    CarryNum<T> ca(a), temp;
    while (b) {
        if (b & 1) {
            temp = add<T, Base>(temp, ca);
        }
        ca = add<T, Base>(ca, ca);
        b >>= 1;
    }
    return temp;
}

template<typename T, size_t Base>
BigInteger<T, Base> & BigInteger<T, Base>::operator *=(const BigInteger<T, Base> &other)
{
    bool negative = (isNegative() != other.isNegative());

    vector<T> av(size()), bv(other.size()), resv(size() + other.size(), 0);

    for (size_t i = 0; i < size(); ++i) {
        av[i] = (*this)[i];
    }

    for (size_t i = 0; i < other.size(); ++i) {
        bv[i] = other[i];
    }

    for (size_t i = 0; i < av.size(); ++i) {
        T carry = 0;
        for (size_t j = 0; j < bv.size() || carry; ++j) {
            CarryNum<T> temp = add<T, Base>(add<T, Base>(resv[i + j], carry),
                                            multiply<T, Base>(av[i], j < bv.size() ? bv[j] : 0));

            carry = temp.h;
            resv[i + j] = temp.l;
        }
    }

    BigInteger<T, Base> res(resv);
    res.trim();
    res.setNegative(negative);

    return *this = res;
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::operator *(const BigInteger<T, Base> &other) const
{
    return BigInteger(*this) *= other;
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator==(const BigInteger<T, Base> &other) const {
    return m_negative == other.m_negative
            && m_digits == other.m_digits
            && m_number == other.m_number;
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator>(const BigInteger<T, Base> &other) const {
    if (*this == other) {
        return false;
    }

    if (!isNegative() && other.isNegative()) {
        return true;
    }

    if (isNegative() && !other.isNegative()) {
        return false;
    }

    bool absGreater = false;
    if (size() > other.size()) {
        absGreater = true;
    } else if (size() == other.size()) {
        for (int i = size() - 1; i >= 0; --i) {
            T a = getDigit(i);
            T b = other.getDigit(i);
            if (a > b) {
                absGreater = true;
                break;
            } else if (a < b) {
                break;
            }
        }
    }

    return isNegative() ^ absGreater;
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::operator<(const BigInteger<T, Base> &other) const {
    return !(*this == other) && !(*this > other);
}

template<typename T, size_t Base>
size_t BigInteger<T, Base>::size() const
{
    return m_digits;
}

template<typename T, size_t Base>
T BigInteger<T, Base>::getDigit(size_t i) const
{
    if (i >= size()) {
        std::ostringstream error;
        error << "Requested digit " << i << ", but size is " << size();
        throw std::out_of_range(error.str());
    }
    return (*this)[i];
}

template<typename T, size_t Base>
T BigInteger<T, Base>::operator[](size_t i) const
{
    T result = m_number[i / digitsInT()];
    result >>= (i % digitsInT()) * bitsPerDigit();

    if (digitsInT() == 1) {
        return result % Base;
    } else {
        return result & ((1 << bitsPerDigit()) - 1);
    }
}

template<typename T, size_t Base>
string BigInteger<T, Base>::str() const
{
    std::ostringstream result;

    if (m_negative) {
        result << '-';
    }

    BigInteger<uint32_t, IO_RADIX> temp = static_cast<BigInteger<uint32_t, IO_RADIX>>(*this);

    result << temp[temp.size() - 1];

    for (int i = temp.size() - 2; i >= 0; --i) {
        result << std::setfill('0') << std::setw(9) << temp[i];
    }

    return result.str();
}

template<typename T, size_t Base>
string BigInteger<T, Base>::repr() const
{
    std::ostringstream result;

    if (m_negative) {
        result << '-';
    }

    for (int i = size() - 1; i >= 0; --i) {
        result << getDigit(i);
    }

    return result.str();
}

template<typename T, size_t Base>
void BigInteger<T, Base>::setDigit(size_t i, T digit)
{
    if (i >= size()) {
        for (size_t t = size(); t <= i; ++t) {
            if (t % digitsInT() == 0) {
                m_number.push_back(0);
            }
            ++m_digits;
        }
    }

    size_t j = i / digitsInT();

    if (digitsInT() == 1) {
        m_number[j] = digit % Base;
    } else {
        size_t shift = i % digitsInT() * bitsPerDigit();
        T clearMask = (1 << bitsPerDigit()) - 1;
        digit &= clearMask;

        clearMask = ~(clearMask << shift);
        m_number[j] &= clearMask;
        m_number[j] |= digit << shift;
    }
}

template<typename T, size_t Base>
void BigInteger<T, Base>::trim()
{
    // Trim leading zeros
    for (size_t i = size() - 1; !isZero() && i >= 0 && !getDigit(i); --i) {
        --m_digits;
    }

    // Remove empty cells
    size_t usedCells = std::ceil(double(size()) / digitsInT());
    if (usedCells < m_number.size()) {
        m_number.resize(usedCells);
    }
}

template<typename T, size_t Base>
BigInteger<T, Base> BigInteger<T, Base>::abs() const
{
    return isNegative() ? -(*this) : *this;
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::isNegative() const
{
    return m_negative;
}

template<typename T, size_t Base>
void BigInteger<T, Base>::setNegative(bool negative)
{
    if (!isZero()) {
        m_negative = negative;
    }
}

template<typename T, size_t Base>
bool BigInteger<T, Base>::isZero() const
{
    return size() == 1 && !getDigit(0);
}

template<typename T, size_t Base>
constexpr int BigInteger<T, Base>::digitsInT()
{
    return (sizeof(T) * 8) / bitsPerDigit();
}

template<typename T, size_t Base>
constexpr int BigInteger<T, Base>::bitsPerDigit()
{
    return std::ceil(std::log2(Base));
}

//
// Input-output stream implementations
//

template<typename T, size_t Base>
ostream & operator << (ostream &stream, const BigInteger<T, Base> &bigint)
{
    stream << bigint.str();
    return stream;
}

template<typename T, size_t Base>
istream & operator >>(istream &stream, BigInteger<T, Base> &bigint)
{
    string temp;
    stream >> temp;
    bigint = BigInteger<T, Base>(temp);
    return stream;
}
