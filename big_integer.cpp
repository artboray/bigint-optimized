#include "big_integer.h"

#include <iostream>

big_integer::big_integer() : sign(0)
{
    _digits.clear();
    _digits.push_back(0);
}

big_integer::big_integer(big_integer const& other) : sign(other.sign)
{
    _digits = other._digits;
}

big_integer::big_integer(uint32_t a) : sign(0)
{
    _digits.push_back(a);
}

big_integer::big_integer(int a) : sign(a < 0)
{
    _digits.push_back(std::abs(a));
}

big_integer::~big_integer()
{
    _digits.clear();
}

big_integer& big_integer::operator=(big_integer const& rhs)
{
    _digits = rhs._digits;
    sign = rhs.sign;
    return *this;
}

const big_integer TEN = big_integer(10), ONE = big_integer(1);

big_integer::big_integer(std::string const& str)
{
    int q = 0;
    if (str[0] == '-')
        q = 1;
    _digits.clear();
    for (int i = q; i < (int)str.size(); i++)
    {
        int cur = (str[i] - '0');
        *this = *this * TEN + big_integer(cur);
    }
    if ((str[0] == '-') && (*this != 0))
        sign = 1;
}

void big_integer::remove_zero()
{
    while ((_digits.size() > 1) && (_digits[_digits.size() - 1] == 0))
        _digits.pop_back();
}

big_integer& big_integer::operator+=(big_integer const& rhs)
{
    if (sign != rhs.sign)
    {
        if (rhs.sign)
        {
            big_integer tmp = rhs;
            tmp.sign = 0;
            return *this -= tmp;
        }
        else
        {
            big_integer tmp = *this;
            tmp.sign = 0;
            return (*this = rhs - tmp);
        }
    }

    _digits.resize(std::max(_digits.size(), rhs._digits.size()));
    bool carry = 0;
    for (int i = 0; i < _digits.size() || carry; i++)
    {
        if (i >= _digits.size())
            _digits.push_back(0);
        uint64_t cur = _digits[i];
        if (i >= rhs._digits.size())
            cur += carry;
        else
            cur += rhs._digits[i] + carry;
        carry = cur >= BASE;
        if (carry) cur -= BASE;
        _digits[i] = (uint32_t)cur;
    }

    remove_zero();
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs)
{
    big_integer other = rhs;
    if (sign != other.sign)
    {
        big_integer tmp = other;
        tmp.sign = sign;
        *this += tmp;
        return *this;
    }
    if (sign)
    {
        big_integer tmp = *this;
        tmp.sign = 0;
        *this = other;
        sign = 0;
        *this -= tmp;
        return *this;
    }
    if (other > *this) {
        big_integer tmp = *this;
        *this = other - tmp;
        sign = 1;
        return *this;
    }
    _digits.resize(std::max(_digits.size(), other._digits.size()));
    bool carry = 0;
    for (int i = 0; i < _digits.size(); i++)
    {
        if (i >= other._digits.size())
            other._digits.push_back(0);
        int64_t a = _digits[i];
        a -= (int64_t)(other._digits[i] + carry);
        carry = a < 0;
        if (carry) a += (int64_t)BASE;
        _digits[i] = (uint32_t)a;
    }
    remove_zero();

    return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs)
{
    big_integer tmp = big_integer();
    if (sign != rhs.sign)
        tmp.sign = 1;

    tmp._digits.resize(_digits.size() + rhs._digits.size() + 2);

    for (int i = 0; i < _digits.size(); i++)
    {
        uint64_t carry = 0;
        for (int j = 0; j < rhs._digits.size() || carry; j++)
        {
            uint32_t oth = 0;
            if (j < rhs._digits.size())
                oth = rhs._digits[j];
            uint64_t cur = (uint64_t)tmp._digits[i + j] + (uint64_t)_digits[i] * (uint64_t)oth + (uint64_t)carry;
            tmp._digits[i + j] = (uint32_t)(cur % BASE);
            carry = (uint64_t)(cur / BASE);
        }
    }

    tmp.remove_zero();

    *this = tmp;
    return *this;
}

big_integer abs(big_integer const& a)
{
    big_integer b = a;
    b.sign = 0;
    return b;
}

big_integer divide(big_integer a, int64_t const& b)
{
    uint64_t carry = 0;
    for (int i = (int)a._digits.size() - 1; i >= 0; i--)
    {
        uint64_t cur = (uint64_t)a._digits[i] + carry * BASE;
        a._digits[i] = (uint32_t)(cur / b);
        carry = (uint64_t)(cur % b);
    }
    a.remove_zero();
    return a;
}

uint32_t mod(big_integer a, int64_t const& b)
{
    uint64_t carry = 0;
    for (int i = (int)a._digits.size() - 1; i >= 0; i--)
        carry = (carry * BASE + a._digits[i]) % b;
    return (uint32_t)carry;
}


big_integer& big_integer::operator/=(big_integer const& rhs)
{
    big_integer b = rhs;

    bool sgn = sign ^ b.sign;
    sign = 0;
    b.sign = 0;

    if (*this < b)
    {
        _digits.clear();
        _digits.push_back(0);
        return *this;
    }

    if (*this == b)
    {
        _digits.clear();
        _digits.push_back(1);
        sign = sgn;
        return *this;
    }

    if (b._digits.size() == 1)
    {
        *this = divide(*this, b._digits[0]);
        sign = sgn;
        return *this;
    }

    uint32_t d = (uint32_t)(BASE / (uint64_t)(b._digits[b._digits.size() - 1] + 1));
    *this *= big_integer(d);
    b *= big_integer(d);
    uint32_t len = (uint32_t)(_digits.size() - b._digits.size() + 1);
    size_t n = _digits.size(), m = b._digits.size();
    big_integer c;
    c._digits.resize(len);
    big_integer tmp;
    tmp._digits.resize(m + 1);

    for (int i = 0; i < m; i++)
        tmp._digits[i] = _digits[n + i - m];
    tmp._digits[m] = 0;
    for (int i = 0; i < len; i++)
    {
        tmp._digits[0] = _digits[n - m - i];
        uint32_t cur = (uint32_t) std::min(((uint64_t)(m < tmp._digits.size() ? tmp._digits[m] : 0) * BASE +
                (uint64_t)(m - 1 < tmp._digits.size() ? tmp._digits[m - 1] : 0)) / (uint64_t)b._digits[b._digits.size() - 1], BASE - (uint64_t)1);

        c._digits[len - i - 1] = cur;

        big_integer div = b;
        div *= big_integer(cur);

        while (tmp < div)
        {
            cur--;
            div -= b;
        }

        tmp -= div;
        while (tmp._digits.size() < m + 1)
            tmp._digits.push_back(0);
        for (size_t j = m; j > 0; j--)
            tmp._digits[j] = tmp._digits[j - 1];
        tmp.remove_zero();
        c._digits[len - i - 1] = cur;
    }

    c.remove_zero();
    c.sign = sgn;
    *this = c;
    return *this;
}


big_integer& big_integer::operator%=(big_integer const& rhs)
{
    *this = *this - rhs * (*this / rhs);
    return *this;
}

big_integer invert(big_integer a)
{
    for (int i = 0; i < a._digits.size(); i++)
        a._digits[i] = ~a._digits[i];
    return a;
}

big_integer bool_operation_prepare(big_integer a)
{
    if (a.sign)
    {
        a = invert(a);
        a.sign = 0;
        a += ONE;
    }
    return a;
}

big_integer bool_operation_end(big_integer a, bool sgn)
{
    if (sgn)
    {
        a -= ONE;
        a = invert(a);
        a.sign = 1;
    }
    else
        a.sign = 0;
    if (abs(a) == 0)
        a.sign = 0;
    return a;
}

big_integer& big_integer::operator&=(big_integer const& rhs)
{
    big_integer b = rhs;
    bool sgn = sign & b.sign;
    _digits.resize(std::max(_digits.size(), b._digits.size()));
    b._digits.resize(std::max(_digits.size(), b._digits.size()));
    *this = bool_operation_prepare(*this);
    b = bool_operation_prepare(b);
    for (int i = 0; i < _digits.size(); i++)
        _digits[i] &= b._digits[i];
    remove_zero();
    *this = bool_operation_end(*this, sgn);
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs)
{
    big_integer b = rhs;
    bool sgn = sign | b.sign;
    _digits.resize(std::max(_digits.size(), b._digits.size()));
    b._digits.resize(std::max(_digits.size(), b._digits.size()));
    *this = bool_operation_prepare(*this);
    b = bool_operation_prepare(b);
    for (int i = 0; i < _digits.size(); i++)
        _digits[i] |= b._digits[i];
    remove_zero();
    *this = bool_operation_end(*this, sgn);
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs)
{
    big_integer b = rhs;
    bool sgn = sign ^ b.sign;
    _digits.resize(std::max(_digits.size(), b._digits.size()));
    b._digits.resize(std::max(_digits.size(), b._digits.size()));
    *this = bool_operation_prepare(*this);
    b = bool_operation_prepare(b);
    for (int i = 0; i < _digits.size(); i++)
        _digits[i] ^= b._digits[i];
    remove_zero();
    *this = bool_operation_end(*this, sgn);
    return *this;
}

big_integer& big_integer::operator<<=(int rhs)
{
    int shl = rhs / 32;
    _digits.resize(std::max(_digits.size(), _digits.size() + shl));
    for (int i = (int)_digits.size() + shl - 1; i >= shl; i--)
        _digits[i] = _digits[i - shl];
    for (int i = 0; i < shl; i++)
        _digits[i] = 0;
    remove_zero();
    *this *= (1 << (rhs % 32));
    if (abs(*this) == 0)
        sign = 0;
    return *this;
}

big_integer& big_integer::operator>>=(int rhs)
{
    int shl = rhs / 32;
    for (int i = 0; i < _digits.size() - shl; i++)
        _digits[i] = _digits[i + shl];
    _digits.resize(std::min(_digits.size(), _digits.size() - shl));
    remove_zero();
    if (mod(*this, (uint32_t)(1 << (rhs % 32))) != 0 && sign)
        *this -= big_integer((uint32_t)(1ll << (rhs % 32)));
    *this = divide(*this, (uint32_t)(1ll << (rhs % 32)));
    if (abs(*this) == 0)
        sign = 0;
    return *this;
}

big_integer big_integer::operator+() const
{
    return *this;
}

big_integer big_integer::operator-() const
{
    big_integer r = *this;
    if (r == big_integer(0))
        return r;
    r.sign = !r.sign;
    return r;
}

big_integer big_integer::operator~() const
{
    return -(*this + 1);
}

big_integer& big_integer::operator++()
{
    *this += ONE;
    return *this;
}

big_integer big_integer::operator++(int)
{
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer& big_integer::operator--()
{
    *this -= ONE;
    return *this;
}

big_integer big_integer::operator--(int)
{
    big_integer r = *this;
    --*this;
    return r;
}

big_integer operator+(big_integer a, big_integer const& b)
{
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b)
{
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b)
{
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b)
{
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b)
{
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b)
{
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b)
{
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b)
{
    return a ^= b;
}

big_integer operator<<(big_integer a, int b)
{
    return a <<= b;
}

big_integer operator>>(big_integer a, int b)
{
    return a >>= b;
}

bool operator==(big_integer const& a, big_integer const& b)
{
    if (a.sign != b.sign || a._digits.size() != b._digits.size())
        return false;
    for (int i = 0; i < a._digits.size(); i++)
        if (a._digits[i] != b._digits[i])
            return false;
    return true;
}

bool operator!=(big_integer const& a, big_integer const& b)
{
    return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b)
{
    if (a.sign & !b.sign)
        return true;
    if (!a.sign & b.sign)
        return false;
    if (a == b)
        return false;
    if (a._digits.size() < b._digits.size())
        return !a.sign;
    if (a._digits.size() > b._digits.size())
        return a.sign;
    size_t pos = a._digits.size() - 1;
    while (a._digits[pos] == b._digits[pos])
        pos--;
    if (a._digits[pos] < b._digits[pos])
        return !a.sign;
    return false;
}

bool operator>(big_integer const& a, big_integer const& b)
{
    return !((a < b) || (a == b));
}

bool operator<=(big_integer const& a, big_integer const& b)
{
    return !(a > b);
}

bool operator>=(big_integer const& a, big_integer const& b)
{
    return !(a < b);
}

std::string to_string(big_integer const& a)
{
    std::string s = "";
    big_integer cur = a;
    cur.sign = 0;
    if (cur == 0)
        s += '0';
    while (cur > 0)
    {
        uint32_t ch = mod(cur, 10);
        s += (ch + '0');
        cur = divide(cur, 10);
    }
    if (a.sign)
        s += '-';
    std::reverse(s.begin(), s.end());
    return s;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a)
{
    return s << to_string(a);
}
