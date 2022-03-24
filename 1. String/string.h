#include <iostream>
#include <cstring>

using std::ostream;
using std::istream;
using std::initializer_list;
using std::copy;

class String {
    size_t len = 0;
    size_t cap = 0;
    char* str = nullptr;

    void swap(String& s);
    char* increase_cap(size_t);
    char* decrease_cap(size_t);

public:
    String() = default;
    String(const String&);
    String(const char);
    String(const char*);
    String(size_t, char);
    String(size_t);
    String(initializer_list<char>);
    String& operator=(String);
    ~String();

    size_t length() const;
    char& operator[] (size_t);
    const char& operator[] (size_t) const;

    friend istream& operator >> (istream&, String&);
    friend ostream& operator << (ostream&, const String&);

    String& operator+=(const String&);
    String& operator+=(char);
    String& decrease_size(size_t);

    char& front();
    char& back();
    const char& front() const;
    const char& back() const;

    bool empty() const;
    void clear();

    void push_back(const char);
    void pop_back();

    String substr(int, int) const;
    size_t find(const String&) const;
    size_t rfind(const String&) const;
};

String::String(const char c) : len(1), cap(1), str(new char[1]) {
    str[0] = c;
}

String::String(const char* s) : len(strlen(s)), cap(len), str(new char[len]) {
    memcpy(str, s, len);
}

String::String(const String& s) : String(s.len) {
    memcpy(str, s.str, len);
}

String::String(size_t len) : len(len), cap(len), str(new char[len]) {}

String::String(size_t len, char c) : len(len), cap(len), str(new char[len]) {
    memset(str, c, len);
}

String::String(initializer_list<char> lst) {
    len = lst.size();
    cap = 2 * len;
    str = new char[len];
    copy(lst.begin(), lst.end(), str);
}

String::~String() {
    delete[] str;
}

char* String::increase_cap(size_t add_len) {
    if(len + add_len >= cap) {
        cap = 2 * (len + add_len);
        char* caped_str = new char[cap];
        memcpy(caped_str, str, len);
        return caped_str;
    }
    return nullptr;
}

char* String::decrease_cap(size_t remove_len) {
    if (len - remove_len <= (cap / 4)) {
        cap /= 2;
        char* caped_str = new char[cap];
        memcpy(caped_str, str, len - remove_len);
        return caped_str;
    }
    return nullptr;

}

void String::swap(String& s) {
    std::swap(len, s.len);
    std::swap(cap, s.cap);
    std::swap(str, s.str);
}

String& String::operator=(String s) {
    swap(s);
    return *this;
}

size_t String::length() const {
    return len;
}

char& String::operator [](size_t pos) {
    return str[pos];
}

const char& String::operator [](size_t pos) const {
    return str[pos];
}

bool operator==(const String& s1, const String& s2) {
    if (s1.length() == s2.length()) {
        for (int i = 0; i < s1.length(); ++i) {
            if (s1[i] != s2[i]) return false;
        }
        return true;
    }
    return false;
}

istream& operator>>(istream& in, String& s) {
    delete[] s.str;
    char c;
    while (in.read(&c, 1)) {
        if (c == ' ' || c == '\0' || c == '\n') break;
        s.push_back(c);
    }
    return in;
}

ostream& operator<<(ostream& out, const String& s) {
    for (size_t i = 0; i < s.len; i++)
        out << s.str[i];
    return out;
}

String& String::operator+=(const String& s) {
    size_t new_len = len + s.len;
    char* new_str = increase_cap(s.len);
    if (new_str != nullptr) {
        memcpy(new_str + len, s.str, s.len);
        delete[] str;
        str = new_str;
        len = new_len;
        return *this;
    }
    memcpy(str + len, s.str, s.len);
    len = new_len;
    return *this;
}

String& String::operator+=(char c) {
    size_t new_len = len + 1;
    char* new_str = increase_cap(1);
    if (new_str != nullptr) {
        new_str[len] = c;
        delete[] str;
        str = new_str;
        len = new_len;
        return *this;
    }
    str[len] = c;
    len = new_len;
    return *this;
}

String operator+(const String& s1, const String& s2) {
    String copy = s1;
    copy += s2;
    return copy;
}

String& String::decrease_size(size_t n) {
    size_t new_len = len - n;
    char* new_str = decrease_cap(n);
    if (new_str != nullptr) {
        delete[] str;
        str = new_str;
        len = new_len;
        return *this;
    }
    len = new_len;
    return *this;
}

bool String::empty() const {
    return len == 0;
}

void String::clear() {
    delete[] str;
    len = 0;
    cap = 0;
    str = nullptr;
}

char& String::front() {
    return str[0];
}

char& String::back() {
    return str[len - 1];
}

const char& String::front() const {
    return str[0];
}

const char& String::back() const {
    return str[len - 1];
}

void String::push_back(const char c) {
    *this += c;
}

void String::pop_back() {
    decrease_size(1);
}

String String::substr(int pos, int n) const {
    String sub_str((size_t)n);
    for (int i = 0; i < n; ++i)
        sub_str[i] = str[i + pos];
    return sub_str;
}

size_t String::find(const String& s) const {
    bool is_found = false;
    size_t pos = len;
    for (size_t i = 0; i < len; ++i) {
        if (is_found) break;
        if (pos == len && str[i] == s[0]) pos = i;
        else if (pos != len) {
            if (s.len == i - pos) is_found = true;
            else if (str[i] != s[i - pos]) {
                i = pos;
                pos = len;
            }
        }
    }
    if (s.len == len - pos) is_found = true;
    if (is_found) return pos;
    return len;
}


size_t String::rfind(const String& s) const {
    bool is_found = false;
    size_t rpos = len;
    for (int i = (int)len - 1; i >= 0; --i) {
        if (is_found) break;
        if (rpos == len && str[i] == s[s.len - 1]) rpos = i;
        else if (rpos != len) {
            if (s.len == rpos - i) is_found = true;
            else if (str[i] != s[s.len - 1 - (rpos - i)]) {
                i = rpos;
                rpos = len;
            }
        }
    }
    if (s.len == rpos + 1) is_found = true;
    if (is_found) return size_t(rpos - s.len + 1);
    return len;
}
