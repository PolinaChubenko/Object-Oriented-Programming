#include <iostream>
#include <sstream>
#include <vector>
#include <string>

using std::vector;
using std::max;
using std::string;
using std::to_string;
using std::ostream;
using std::istream;
using std::stringstream;

class BigInteger {
private:
    static const int base = 1e4;
    vector<int> bits;
    bool is_positive = true;
    void swap(BigInteger&);
    void remove_extra_zeros();
    void shift_right();
    bool lessAbs(const BigInteger&) const;
    int get_size() const;
public:
    BigInteger();
    BigInteger(const BigInteger&);
    BigInteger(const string&);
    BigInteger(int);
    ~BigInteger() = default;

    BigInteger& operator=(BigInteger);
    BigInteger& operator=(int);

    BigInteger& change_sign();
    explicit operator bool() const;
    friend bool operator==(const BigInteger&, const BigInteger&);
    friend bool operator<(const BigInteger&, const BigInteger&);
    bool isEven() const;

    friend istream& operator >> (istream&, BigInteger&);
    friend ostream& operator << (ostream&, const BigInteger&);

    BigInteger& operator+=(const BigInteger&);
    BigInteger& operator-=(const BigInteger&);
    BigInteger& operator*=(const BigInteger&);
    BigInteger& operator%=(const BigInteger&);
    BigInteger& operator/=(const BigInteger&);
    void div2();
    void subtraction(const BigInteger&, const BigInteger&);

    BigInteger operator-() const;
    //префикс
    BigInteger& operator++();
    BigInteger& operator--();
    //постфикс с фиктивной переменной
    BigInteger operator++(int);
    BigInteger operator--(int);

    BigInteger& parseString(const string&);
    string toString() const;
};


///////////   CONSTRUCTORS   ///////////
BigInteger::BigInteger() {
    bits.push_back(0);
}
BigInteger::BigInteger(const BigInteger& num) {
    is_positive = num.is_positive;
    bits = num.bits;
}
BigInteger::BigInteger(const string& s) {
    parseString(s);
}
BigInteger::BigInteger(int value) {
    if (value == 0) {
        bits.push_back(0);
        is_positive = true;
        return;
    }
    int num = value;
    if (num < 0) {
        is_positive = false;
        num *= -1;
    }
    while (num) {
        bits.push_back(num % base);
        num /= base;
    }
}
int BigInteger::get_size() const {
    int size = bits.size();
    return size;
}


/////////////   COPYING    /////////////
void BigInteger::swap(BigInteger& num) {
    std::swap(bits,num.bits);
    std::swap(is_positive,num.is_positive);
}

BigInteger& BigInteger::operator=(BigInteger num) {
    swap(num);
    return *this;
}
BigInteger& BigInteger::operator=(int value) {
    bits.clear();
    is_positive = true;
    if (value == 0) {
        bits.push_back(0);
        return *this;
    }
    int num = value;
    if (num < 0) {
        is_positive = false;
        num *= -1;
    }
    while (num) {
        bits.push_back(num % base);
        num /= base;
    }
    return *this;
}


/////////////    CAST     /////////////
BigInteger::operator bool() const{
    return bits.back() != 0;
}


/////////////   LOGICAL    /////////////
bool operator==(const BigInteger& num1, const BigInteger& num2) {
    if (num1.bits.size() != num2.bits.size() || num1.is_positive != num2.is_positive) return false;
    for (int i = 0; i < num1.get_size(); ++i) {
        if (num1.bits[i] != num2.bits[i]){
            return false;
        }
    }
    return true;
}
bool operator!=(const BigInteger& num1, const BigInteger& num2) {
    return !(num1 == num2);
}
bool operator<(const BigInteger& num1, const BigInteger& num2) {
    if (!num1.is_positive && num2.is_positive)
        return true;
    if (num1.is_positive && !num2.is_positive)
        return false;
    if (num1.bits.size() < num2.bits.size())
        return num1.is_positive;
    if (num1.bits.size() > num2.bits.size())
        return !num1.is_positive;
    for (int i = num1.get_size() - 1; i >= 0; --i) {
        if (num1.bits[i] != num2.bits[i])
            return (num1.bits[i] < num2.bits[i]) == num1.is_positive;
    }
    return false;
}
bool operator<=(const BigInteger& num1, const BigInteger& num2) {
    return (num1 < num2) || (num1 == num2);
}
bool operator>(const BigInteger& num1, const BigInteger& num2) {
    return !(num1 <= num2);
}
bool operator>=(const BigInteger& num1, const BigInteger& num2) {
    return !(num1 < num2);
}
bool BigInteger::lessAbs(const BigInteger& num) const {
    if (bits.size() < num.bits.size())
        return true;
    if (bits.size() > num.bits.size())
        return false;
    for (int i = get_size() - 1; i >= 0; --i)
        if (bits[i] != num.bits[i])
            return (bits[i] < num.bits[i]);
    return false;
}
bool BigInteger::isEven() const {
    return (bits[0] % 2 == 0);
}


/////////////    STREAM    /////////////
istream& operator >> (istream& in, BigInteger& num) {
    string s;
    in >> s;
    num.parseString(s);
    return in;
}
ostream& operator << (ostream& out, const BigInteger& num) {
    out << num.toString();
    return out;
}


/////////////    MATHS    /////////////
void BigInteger::remove_extra_zeros() {
    int zeros = 0;
    for (int i = get_size() - 1; i >= 0; --i) {
        if (bits[i] == 0)
            ++zeros;
        else break;
    }
    if (zeros)
        bits.resize(bits.size() - zeros);
    if (bits.empty())
        bits.push_back(0);
}

BigInteger& BigInteger::change_sign() {
    if (bits.back() == 0) return *this;
    is_positive = (!is_positive);
    return *this;
}
void BigInteger::shift_right() {
    if (bits.empty()) {
        bits.push_back(0);
        return;
    }
    bits.push_back(bits[bits.size() - 1]);
    for (size_t i = bits.size() - 2; i > 0; --i)
        bits[i] = bits[i - 1];
    bits[0] = 0;
}

BigInteger& BigInteger::operator+=(const BigInteger& num) {
    if (*this == 0) {
        *this = num;
        return *this;
    }
    if (num == 0) {
        return *this;
    }
    if ((!is_positive && num.is_positive) || (is_positive && !num.is_positive)) {
        change_sign();
        *this -= num;
        change_sign();
        return *this;
    }
    int carry = 0;
    for (size_t i = 0; i < max(bits.size(), num.bits.size()) || carry; ++i) {
        if (i == bits.size())
            bits.push_back(0);
        bits[i] += carry + (i < num.bits.size() ? num.bits[i] : 0);
        carry = bits[i] >= BigInteger::base;
        if (carry)
            bits[i] -= BigInteger::base;
    }
    return *this;
}
BigInteger operator+(const BigInteger& num1, const BigInteger& num2) {
    BigInteger copy = num1;
    copy += num2;
    return copy;
}
void BigInteger::subtraction(const BigInteger& bigger, const BigInteger& smaller) {
    int carry = 0;
    bits.resize(bigger.bits.size());
    for (size_t i = 0; i < smaller.bits.size() || carry; ++i) {
        bits[i] = bigger.bits[i] - carry - (i < smaller.bits.size() ? smaller.bits[i] : 0);
        carry = bits[i] < 0;
        if (carry)
            bits[i] += BigInteger::base;
    }
    remove_extra_zeros();
}
BigInteger& BigInteger::operator-=(const BigInteger& num) {
    if (*this == 0) {
        *this = num;
        change_sign();
        return *this;
    }
    if (num == 0) {
        return *this;
    }
    if (*this == num) {
        bits.clear();
        bits.push_back(0);
        is_positive = true;
        return *this;
    }
    if ((!is_positive && num.is_positive) || (is_positive && !num.is_positive)) {
        change_sign();
        *this += num;
        change_sign();
        return *this;
    }
    if (lessAbs(num)) {
        subtraction(num, *this);
        change_sign();
        return *this;
    }
    subtraction(*this, num);
    return *this;
}
BigInteger operator-(const BigInteger& num1, const BigInteger& num2) {
    BigInteger copy = num1;
    copy -= num2;
    return copy;
}
BigInteger& BigInteger::operator*=(const BigInteger& num) {
    if (num == 0 || *this == 0) {
        *this = 0;
        return *this;
    }
    BigInteger result;
    result.is_positive = is_positive == num.is_positive;
    result.bits.resize(bits.size() + num.bits.size());
    for (size_t i = 0; i < bits.size(); ++i) {
        int carry = 0;
        for (size_t j = 0; j < num.bits.size() || carry; ++j) {
            long long cur = result.bits[i + j] + bits[i] * (j < num.bits.size() ? num.bits[j] : 0) + carry;
            result.bits[i + j] = static_cast<int>(cur % BigInteger::base);
            carry = static_cast<int>(cur / BigInteger::base);
        }
    }
    result.remove_extra_zeros();
    swap(result);
    return *this;

}
BigInteger operator*(const BigInteger& num1, const BigInteger& num2) {
    BigInteger copy = num1;
    copy *= num2;
    return copy;
}
BigInteger& BigInteger::operator%=(const BigInteger& num) {
    BigInteger tmp = (*this);
    tmp /= num;
    tmp *= num;
    *this -= tmp;
    return *this;
}
BigInteger operator%(const BigInteger& num1, const BigInteger& num2) {
    BigInteger copy = num1;
    copy %= num2;
    return copy;
}
BigInteger& BigInteger::operator/=(const BigInteger& num) {
    if (*this == 0) {
        return *this;
    }
    if (*this == num) {
        *this = 1;
        return *this;
    }
    if (num == 1) return *this;
    if (num == -1) {
        is_positive = !is_positive;
        return *this;
    }
    if (lessAbs(num) || *this == 0) {
        bits.clear();
        bits.push_back(0);
        is_positive = true;
        return *this;
    }
    BigInteger result, current;
    result.is_positive = is_positive == num.is_positive;
    result.bits.resize(bits.size());
    for (long long i = static_cast<long long>(bits.size()) - 1; i >= 0; --i) {
        current.shift_right();
        current.bits[0] = bits[i];
        current.remove_extra_zeros();
        int x = 0, left = 0, right = BigInteger::base;
        while (left <= right) {
            int mid = (left + right) / 2;
            BigInteger t = num;
            t.is_positive = true;
            current.is_positive = true;
            t *= mid;
            if (t <= current) {
                x = mid;
                left = mid + 1;
            }
            else right = mid - 1;
        }
        result.bits[i] = x;
        if (num > 0)
            current -= num * x;
        else
            current -= (-num) * x;
    }
    result.remove_extra_zeros();
    swap(result);
    return *this;
}
BigInteger operator/(const BigInteger& num1, const BigInteger& num2) {
    BigInteger copy = num1;
    copy /= num2;
    return copy;
}
void BigInteger::div2() {
    for (size_t i = 0; i < bits.size(); ++i) {
        if (i != 0)
            bits[i - 1] += 5000 * (bits[i] % 2);
        bits[i] /= 2;
    }
    if (bits[bits.size() - 1] == 0 && bits.size() > 1)
        bits.pop_back();
}

/////    ADDITIONAL METHODS    /////
BigInteger greatest_common_divisor(BigInteger num1, BigInteger num2) {
    if (num1 == 0 || num2 == 0) return 1;
    if (num1 < 0) num1.change_sign();
    if (num2 < 0) num2.change_sign();
    BigInteger delta = 1;
    while (num1 != 0 && num2 != 0) {
        while (num1.isEven() && num2.isEven()) {
            delta *= 2;
            num1.div2();
            num2.div2();
        }
        while (num1.isEven())
            num1.div2();
        while (num2.isEven())
            num2.div2();
        if (num1 >= num2) num1 -= num2;
        else num2 -= num1;
    }
    return delta * num2;
}
BigInteger pow(BigInteger& num, int deg) {
    if (deg == 0)
        return 1;
    if (deg % 2 == 0) {
        BigInteger tmp = pow(num, deg / 2);
        return tmp * tmp;
    }
    return num * pow(num, deg - 1);
}


/////    INCREMENT - DECREMENT    /////
BigInteger& BigInteger::operator++() {
    return *this += 1;
}

BigInteger& BigInteger::operator--() {
    return *this -= 1;
}
BigInteger BigInteger::operator++(int) {
    BigInteger tmp = *this;
    ++(*this);
    return tmp;
}
BigInteger BigInteger::operator--(int) {
    BigInteger tmp = *this;
    --(*this);
    return tmp;
}


/////////////    BINARY    /////////////
BigInteger BigInteger::operator-() const {
    BigInteger copy = *this;
    if (copy == 0) return copy;
    copy.is_positive = !(copy.is_positive);
    return copy;
}


/////////////    PARSING    /////////////
BigInteger& BigInteger::parseString(const string& s) {
    bits.clear();
    is_positive = true;
    int last = 0;
    if (s[0] == '-') {
        is_positive = false;
        last = 1;
    }
    int s_len = s.size();
    for (int i = s_len - 1; i >= last; i -= 4) {
        size_t len = (i >= 4) ? 4 : i - last + 1;
        bits.push_back(stoi(s.substr(i - len + 1, len)));
    }
    return *this;
}

string BigInteger::toString() const {
    string s;
    if (!is_positive) s += "-";
    for (int i = get_size() - 1; i >= 0; --i) {
        if (i != get_size() - 1 && bits[i] == 0)
            s += "0000";
        else if (i == get_size() - 1)
            s += to_string(bits[i]);
        else {
            if (bits[i] > 999)
                s += to_string(bits[i]);
            else if (bits[i] > 99)
                s += "0" + to_string(bits[i]);
            else if (bits[i] > 9)
                s += "00" + to_string(bits[i]);
            else if (bits[i] > 0)
                s += "000" + to_string(bits[i]);
        }
    }
    return s;
}


/*******************************************************/
///////////////////   RATIONAL   ////////////////////////
/*******************************************************/

class Rational {
private:
    BigInteger numerator = 0;
    BigInteger denominator = 1;
    void simplify();
    void swap(Rational&);
public:
    Rational() = default;
    Rational(const BigInteger&, const BigInteger&);
    Rational(const int);
    ~Rational() = default;

    Rational& operator=(Rational);

    explicit operator double() const;
    explicit operator bool() const;
    friend bool operator==(const Rational&, const Rational&);
    friend bool operator!=(const Rational&, const Rational&);
    friend bool operator<(const Rational&, const Rational&);
    friend bool operator<=(const Rational&, const Rational&);
    friend bool operator>(const Rational&, const Rational&);
    friend bool operator>=(const Rational&, const Rational&);

    Rational& operator+=(const Rational&);
    Rational& operator-=(const Rational&);
    Rational& operator*=(const Rational&);
    Rational& operator/=(const Rational&);

    Rational operator-() const;

    friend istream& operator >> (istream&, Rational&);
    friend ostream& operator << (ostream&, const Rational&);

    string asDecimal(size_t) const;
    string toString() const;
};



///////////   CONSTRUCTORS   ///////////
void Rational::simplify() {
    if (numerator == 0) denominator = 1;
    else {
        BigInteger common = greatest_common_divisor(numerator, denominator);
        numerator /= common;
        denominator /= common;
        if (denominator < 0) {
            numerator.change_sign();
            denominator.change_sign();
        }
    }
}
Rational::Rational(const BigInteger& num, const BigInteger& den = 1): numerator(num), denominator(den) {
    if (num == 0) {
        denominator = 1;
        return;
    }
    simplify();
}
Rational::Rational(const int n): Rational(n, 1) {}


/////////////   COPYING    /////////////
void Rational::swap(Rational& q) {
    std::swap(numerator, q.numerator);
    std::swap(denominator, q.denominator);
}
Rational& Rational::operator=(Rational q) {
    swap(q);
    return *this;
}


/////////////    CAST     /////////////
Rational::operator double() const{
    stringstream buffer;
    buffer << asDecimal(16);
    double answer;
    buffer >> answer;
    return answer;
}
Rational::operator bool() const {
    return *this != 0;
}


/////////////   LOGICAL    /////////////
bool operator==(const Rational& q1, const Rational& q2) {
    return (q1.numerator == q2.numerator) && (q1.denominator == q2.denominator);
}
bool operator!=(const Rational& q1, const Rational& q2) {
    return !(q1 == q2);
}
bool operator<(const Rational& q1, const Rational& q2) {
    return q1.numerator * q2.numerator < q2.denominator * q1.denominator;
}
bool operator<=(const Rational& q1, const Rational& q2) {
    return (q1 == q2) || (q1 < q2);
}
bool operator>(const Rational& q1, const Rational& q2) {
    return !(q1 <= q2);
}
bool operator>=(const Rational& q1, const Rational& q2) {
    return !(q1 < q2);
}


/////////////    MATHS    /////////////
Rational& Rational::operator+=(const Rational& q) {
    if (this == &q) {
        numerator *= 2;
        simplify();
        return *this;
    }
    numerator = numerator * q.denominator + denominator * q.numerator;
    denominator *= q.denominator;
    simplify();
    return *this;
}
Rational operator+(const Rational& q1, const Rational& q2) {
    Rational copy = q1;
    copy += q2;
    return copy;
}
Rational& Rational::operator-=(const Rational& q) {
    if (this == &q) {
        *this = 0;
        return *this;
    }
    numerator.change_sign();
    (*this) += q;
    numerator.change_sign();
    return *this;
}
Rational operator-(const Rational& q1, const Rational& q2) {
    Rational copy = q1;
    copy -= q2;
    return copy;
}
Rational& Rational::operator*=(const Rational& q) {
    numerator *= q.numerator;
    denominator *= q.denominator;
    simplify();
    return *this;
}
Rational operator*(const Rational& q1, const Rational& q2) {
    Rational copy = q1;
    copy *= q2;
    return copy;
}
Rational& Rational::operator/=(const Rational& q) {
    if (this == &q) {
        *this = 1;
        return *this;
    }
    if (*this == 0)
        return *this;
    numerator *= q.denominator;
    denominator *= q.numerator;
    simplify();
    return *this;
}
Rational operator/(const Rational& q1, const Rational& q2) {
    Rational copy = q1;
    copy /= q2;
    return copy;
}

/////////////    BINARY    /////////////
Rational Rational::operator-() const {
    Rational copy = *this;
    if (*this == 0) return copy;
    copy.numerator.change_sign();
    return copy;
}


/////////////    STREAM    /////////////
istream& operator >> (istream& in, Rational& q) {
    int val;
    in >> val;
    q = val;
    /*
    string s, parse = "/";
    in >> s;
    q.numerator.parseString(s.substr(0, s.find(parse)));
    q.denominator.parseString(s.substr(s.find(parse)+1));
    q.simplify();
    */
    return in;
}
ostream& operator << (ostream& out, const Rational& q) {
    out << q.toString();
    return out;
}


/////////////    PARSING    /////////////
string Rational::asDecimal(size_t precision = 0) const {
    string s;
    if (numerator < 0)
        s += '-';
    BigInteger n = (numerator > 0 ? numerator : -numerator);
    BigInteger tmp = n / denominator;
    BigInteger remains = n % denominator;
    s += tmp.toString();
    if (precision > 0) {
        s += '.';
        for (size_t i = 0; i < precision; ++i) {
            remains *= 10;
            BigInteger t = remains / denominator;
            s += t.toString();
            remains %= denominator;
        }
    }
    return s;
}
string Rational::toString() const {
    if (denominator == 1)
        return numerator.toString();
    return numerator.toString() + "/" + denominator.toString();
}


/*******************************************************/
/////////////////////   FINITE   ////////////////////////
/*******************************************************/

template <int N>
class Finite {
private:
    int n = 0;
    void fix(long long n);
public:
    void swap(Finite<N>&);

    Finite() = default;
    Finite(int);
    ~Finite() = default;

    Finite<N>& operator=(Finite<N>);

    Finite<N>& operator+=(const Finite<N>&);
    Finite<N>& operator-=(const Finite<N>&);
    Finite<N>& operator*=(const Finite<N>&);
    Finite<N>& operator/=(const Finite<N>&);

    Finite<N> operator-() const;

    Finite<N>& operator++();
    Finite<N>& operator--();
    Finite<N> operator++(int);
    Finite<N> operator--(int);

    bool operator==(const Finite<N>&) const;
    bool operator!=(const Finite<N>&) const;

    explicit operator int() const;
    explicit operator double() const;
    explicit operator bool() const;

    Finite<N> binary_pow(size_t deg) const;
    Finite<N> get_opposite() const;
};


///////////   CONSTRUCTORS   ///////////
template<int N>
void Finite<N>::fix(long long num) {
    n = ((num % N) + N) % N;
}
template<int N>
Finite<N>::Finite(int val) {
    fix(val);
}


/////////////   COPYING    /////////////
template<int N>
void Finite<N>::swap(Finite<N>& el) {
    std::swap(n, el.n);
}
template<int N>
Finite<N>& Finite<N>::operator=(Finite<N> el) {
    swap(el);
    return *this;
}


/////////////    STREAM    /////////////
template<int N>
ostream& operator<<(ostream& out, const Finite<N>& el) {
    out << (int)el;
    return out;
}


/////////////    MATHS    /////////////
template<int N>
Finite<N>& Finite<N>::operator+=(const Finite<N>& el) {
    fix(n + el.n);
    return *this;
}
template<int N>
Finite<N>& Finite<N>::operator-=(const Finite<N>& el) {
    fix(n - el.n);
    return *this;
}
template<int N>
Finite<N>& Finite<N>::operator*=(const Finite<N>& el) {
    fix((long long)n * el.n);
    return *this;
}
template<int N> // N - простое число
Finite<N>& Finite<N>::operator/=(const Finite<N>& el) {
    *this *= el.get_opposite();
    return *this;
}
template <int N>
Finite<N> operator+(const Finite<N>& el1, const Finite<N>& el2) {
    Finite<N> copy = el1;
    copy += el2;
    return copy;
}
template <int N>
Finite<N> operator-(const Finite<N>& el1, const Finite<N>& el2) {
    Finite<N> copy = el1;
    copy -= el2;
    return copy;
}
template <int N>
Finite<N> operator*(const Finite<N>& el1, const Finite<N>& el2) {
    Finite<N> copy = el1;
    copy *= el2;
    return copy;
}
template <int N> // N - простое число
Finite<N> operator/(const Finite<N>& el1, const Finite<N>& el2) {
    Finite<N> copy = el1;
    copy /= el2;
    return copy;
}


/////////////    BINARY    /////////////
template <int N>
Finite<N> Finite<N>::operator-() const {
    Finite<N> copy = *this;
    if (copy == 0) return copy;
    copy.n = -copy.n;
    copy.fix(copy.n);
    return copy;
}


/////    INCREMENT - DECREMENT    /////
template <int N>
Finite<N>& Finite<N>::operator++() {
    return *this += 1;
}
template <int N>
Finite<N>& Finite<N>::operator--() {
    return *this -= 1;
}
template <int N>
Finite<N> Finite<N>::operator++(int) {
    Finite<N> tmp = *this;
    ++(*this);
    return tmp;
}
template <int N>
Finite<N> Finite<N>::operator--(int) {
    Finite<N> tmp = *this;
    --(*this);
    return tmp;
}


/////////////   LOGICAL    /////////////
template <int N>
bool Finite<N>::operator==(const Finite<N> &el) const {
    return n == el.n;
}
template <int N>
bool Finite<N>::operator!=(const Finite<N> &el) const {
    return !(*this == el);
}


/////////////    CAST     /////////////
template<int N>
Finite<N>::operator int() const {
    return n;
}
template<int N>
Finite<N>::operator double() const {
    return (double)n;
}
template<int N>
Finite<N>::operator bool() const {
    return n != 0;
}


///////    ADDITIONAL METHODS    ///////
template<int N>
Finite<N> Finite<N>::binary_pow(size_t deg) const {
    Finite<N> result = 1;
    Finite<N> copy = *this;
    while (deg) {
        if (deg & 1) result *= copy;
        copy *= copy;
        deg >>= 1;
    }
    return result;
}
template<int N>
Finite<N> Finite<N>::get_opposite() const {
    return binary_pow(N - 2);
}


/*******************************************************/
/////////////////////   MATRIX   ////////////////////////
/*******************************************************/

template <unsigned M, unsigned N, typename Field = Rational>
class Matrix {
private:
    vector<vector<Field>> table;
public:
    Matrix();
    Matrix(const vector<vector<Field>>&);
    Matrix(const vector<vector<int>>&);
    ~Matrix() = default;

    vector<Field>& operator[](size_t);
    const vector<Field>& operator[](size_t) const;
    vector<Field> getRow(unsigned index);
    vector<Field> getColumn(unsigned index);

    Matrix<M, N, Field>& operator+=(const Matrix<M, N, Field>&);
    Matrix<M, N, Field>& operator-=(const Matrix<M, N, Field>&);
    Matrix<M, N, Field>& operator*=(const Field&);
    Matrix<M, N, Field>& operator*=(const Matrix<M, N, Field> &);

    Matrix<N, M, Field> transposed() const;
    Matrix<M, N, Field> inverted() const;
    Matrix<M, N, Field>& invert();
    size_t rank() const;
    Field trace() const;
    Field det() const;

    void swapRows(size_t i, size_t j);
    void swapColumns (size_t i, size_t j);
};


///////////   CONSTRUCTORS   ///////////
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field>::Matrix() {
    table.assign(M, vector<Field>(N, 0));
    if (M == N) {
        size_t i = 0;
        while (i < M) {
            table[i][i] = 1;
            ++i;
        }
    }
}
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field>::Matrix(const vector<vector<Field>>& v) {
    table = v;
}
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field>::Matrix(const vector<vector<int>>& v) {
    for (size_t i = 0; i < M; ++i) {
        vector<Field> tmp;
        for (size_t j = 0; j < N; ++j) {
            tmp.push_back(v[i][j]);
        }
        table.push_back(tmp);
    }

}

///////////    GET ELEMENT    ///////////
template<unsigned int M, unsigned int N, typename Field>
vector<Field>& Matrix<M, N, Field>::operator[](size_t pos) {
    return table[pos];
}
template<unsigned int M, unsigned int N, typename Field>
const vector<Field>& Matrix<M, N, Field>::operator[](size_t pos) const {
    return table[pos];
}
template<unsigned int M, unsigned int N, typename Field>
vector<Field> Matrix<M, N, Field>::getRow(unsigned index) {
    return table[index];
}
template<unsigned int M, unsigned int N, typename Field>
vector<Field> Matrix<M, N, Field>::getColumn(unsigned index) {
    vector<Field> result;
    for (size_t i = 0; i < M; ++i)
        result.push_back(table[i][index]);
    return result;
}


/////////////    STREAM    /////////////
template<unsigned int M, unsigned int N, typename Field>
ostream& operator<<(ostream& out, const Matrix<M, N, Field>& matrix) {
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            out << matrix[i][j] << " ";
        }
        out << "\n";
    }
    return out;
}


/////////////    MATHS    /////////////
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field>& Matrix<M, N, Field>::operator+=(const Matrix<M, N, Field>& matrix) {
    for (size_t i = 0; i < M; ++i)
        for (size_t j = 0; j < N; ++j)
            table[i][j] += matrix.table[i][j];
    return *this;
}
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field>& Matrix<M, N, Field>::operator-=(const Matrix<M, N, Field>& matrix) {
    for (size_t i = 0; i < M; ++i)
        for (size_t j = 0; j < N; ++j)
            table[i][j] -= matrix.table[i][j];
    return *this;
}
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field> operator+(const Matrix<M, N, Field>& matrix1, const Matrix<M, N, Field>& matrix2) {
    Matrix<M, N, Field> copy = matrix1;
    copy += matrix2;
    return copy;
}
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field> operator-(const Matrix<M, N, Field>& matrix1, const Matrix<M, N, Field>& matrix2) {
    Matrix<M, N, Field> copy = matrix1;
    copy -= matrix2;
    return copy;
}
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field>& Matrix<M, N, Field>::operator*=(const Field &f) {
    for (size_t i = 0; i < M; ++i)
        for (size_t j = 0; j < N; ++j)
            table[i][j] *= f;
    return *this;
}
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field> operator*(const Matrix<M, N, Field>& matrix, const Field& f) {
    Matrix<M, N, Field> copy = matrix;
    copy *= f;
    return copy;
}
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field> operator*(const Field& f, const Matrix<M, N, Field>& matrix) {
    Matrix<M, N, Field> copy = matrix;
    copy *= f;
    return copy;
}
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field>& Matrix<M, N, Field>::operator*=(const Matrix<M, N, Field>& matrix) {
    static_assert(M == N, "Square Matrix needed");
    vector<vector<Field>> copy = table;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            table[i][j] = 0;
            for (size_t p = 0; p < N; ++p) {
                table[i][j] += copy[i][p] * matrix[p][j];
            }
        }
    }
    return *this;
}
template <unsigned int M, unsigned int N, unsigned int K, typename Field = Rational>
Matrix<M, K, Field> operator*(const Matrix<M, N, Field>& matrix1, const Matrix<N, K, Field>& matrix2) {
    Matrix<M, K, Field> result;
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < K; ++j) {
            result[i][j] = 0;
            for (size_t p = 0; p < N; ++p) {
                result[i][j] += matrix1[i][p] * matrix2[p][j];
            }
        }
    }
    return result;
}


///////////    METHODS    ///////////
template<unsigned int M, unsigned int N, typename Field>
Matrix<N, M, Field> Matrix<M, N, Field>::transposed() const {
    Matrix<N, M, Field> result;
    for (size_t i = 0; i < M; ++i)
        for (size_t j = 0; j < N; ++j)
            result[j][i] = table[i][j];
    return result;
}
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field> Matrix<M, N, Field>::inverted() const {
    static_assert(M == N, "Square Matrix needed");
    Matrix<M, N, Field> inverted = *this;
    return inverted.invert();
}
template<unsigned int M, unsigned int N, typename Field>
Matrix<M, N, Field>& Matrix<M, N, Field>::invert() {
    static_assert(M == N, "Square Matrix needed");
    Matrix<M, N, Field> inverted;
    for (size_t i = 0; i < N; ++i) {
        size_t this_row = i;
        for (size_t j = i; j < N; ++j) {
            if (table[j][i]) {
                this_row = j;
                break;
            }
        }
        if (table[this_row][i] == 0)
            return *this = inverted;
        inverted.swapRows(i, this_row);
        swapRows(i, this_row);
        for (size_t j = 0; j < N; ++j) {
            if (j != i) {
                Field k = table[j][i] / table[i][i];
                for (size_t p = 0; p < N; ++p) {
                    table[j][p] -= table[i][p] * k;
                    inverted[j][p] -= inverted[i][p] * k;
                }
            }
        }
        Field k = table[i][i];
        for (size_t j = 0; j < N; ++j) {
            table[i][j] /= k;
            inverted[i][j] /= k;
        }
    }
    return *this = inverted;
}
template<unsigned int M, unsigned int N, typename Field>
size_t Matrix<M, N, Field>::rank() const {
    if (M < N) return transposed().rank();
    Matrix<M, N, Field> copy = *this;
    size_t rank = N;
    for (size_t i = 0; i < N; ++i) {
        size_t this_row = i;
        for (size_t j = i; j < M; ++j) {
            if (copy[j][i]) {
                this_row = j;
                break;
            }
        }
        if (copy[this_row][i] == 0)
            --rank;
        else {
            copy.swapRows(i, this_row);
            for (size_t j = i + 1; j < M; ++j) {
                Field k = copy[j][i] / copy[i][i];
                for (size_t p = i; p < N; ++p)
                    copy[j][p] -= copy[i][p] * k;
            }
        }
    }
    return rank;
}
template<unsigned int M, unsigned int N, typename Field>
Field Matrix<M, N, Field>::trace() const {
    Field trace;
    size_t i = 0;
    while (i < M) {
        trace += table[i][i];
        ++i;
    }
    return trace;
}
template<unsigned int M, unsigned int N, typename Field>
Field Matrix<M, N, Field>::det() const {
    static_assert(M == N, "Square Matrix needed");
    Matrix<M, N, Field> copy = *this;
    Field det = 1;
    for (size_t i = 0; i < N; ++i) {
        size_t this_row = i;
        for (size_t j = i; j < N; ++j) {
            if (copy[j][i]) {
                this_row = j;
                break;
            }
        }
        if (copy[this_row][i] == 0) return 0;
        copy.swapRows(i, this_row);
        if (i != this_row)
            det = -det;
        det *= copy[i][i];
        for (size_t j = i + 1; j < N; ++j) {
            Field k = copy[j][i] / copy[i][i];
            for (size_t p = i; p < N; ++p)
                copy[j][p] -= copy[i][p] * k;
        }
    }
    return det;
}


/////////////   LOGICAL    /////////////
template<unsigned int M, unsigned int N, typename Field>
bool operator==(const Matrix<M, N, Field>& matrix1, const Matrix<M, N, Field>& matrix2) {
    for (size_t i = 0; i < M; ++i)
        for (size_t j = 0; j < N; ++j)
            if (matrix1[i][j] != matrix2[i][j]) return false;
    return true;
}
template<unsigned int M, unsigned int N, typename Field>
bool operator!=(const Matrix<M, N, Field>& matrix1, const Matrix<M, N, Field>& matrix2) {
    return !(matrix1 == matrix2);
}


///////    ADDITIONAL METHODS    ///////
template<unsigned int M, unsigned int N, typename Field>
void Matrix<M, N, Field>::swapRows(size_t i, size_t j) {
    std::swap(table[i], table[j]);
}
template<unsigned int M, unsigned int N, typename Field>
void Matrix<M, N, Field>::swapColumns(size_t i, size_t j) {
    for (size_t k = 0; k < M; ++k)
        std::swap(table[k][i], table[k][j]);
}


///////////    SQUARE MATRIX    ///////////
template<unsigned N, typename Field = Rational>
using SquareMatrix = Matrix<N, N, Field>;
