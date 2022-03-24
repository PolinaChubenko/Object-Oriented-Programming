#include <iostream>
#include <utility>
#include <vector>
#include <type_traits>

template <typename T>
class Deque {
private:
    static const size_t bucket_sz = 10;

    std::vector<T*> arr;
    size_t sz = 0;
    size_t cap = bucket_sz;
    int n_buckets = 0;
    std::pair<int, int> begin_pos;
    std::pair<int, int> end_pos;

    void double_size();
    void move_pos_forward(std::pair<int, int>&);
    void move_pos_back(std::pair<int, int>&);
    void move_pos(std::pair<int, int>&, int) const;

public:

    ////////////   ITERATORS   ////////////
    template <bool IsConst>
    class common_iterator {
    private:
        std::conditional_t<IsConst, const T*, T*> ptr;
        std::pair<int, int> it_pos;
        std::conditional_t<IsConst, const std::vector<T*>, std::vector<T*>>* arr_ptr;

        void move_pos(std::pair<int, int>&, int) const;
    public:
        common_iterator(std::conditional_t<IsConst, const std::vector<T*>,
                std::vector<T*>>&, std::pair<int, int>);
        std::conditional_t<IsConst, const T&, T&> operator*();
        std::conditional_t<IsConst, const T*, T*> operator->();

        common_iterator& operator++();
        common_iterator& operator--();
        common_iterator operator++(int);
        common_iterator operator--(int);


        common_iterator operator+(int) const;
        common_iterator operator-(int) const;
        common_iterator& operator+=(int);
        common_iterator& operator-=(int);
        int operator-(const common_iterator&) const;


        bool operator==(const common_iterator&) const;
        bool operator!=(const common_iterator&) const;
        bool operator<(const common_iterator&) const;
        bool operator<=(const common_iterator&) const;
        bool operator>(const common_iterator&) const;
        bool operator>=(const common_iterator&) const;


        using T_ptr_type = std::conditional_t<IsConst, const T*, T*>;
        using T_type = std::conditional_t<IsConst, const T&, T&>;

        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using iterator_category = std::random_access_iterator_tag;
        using pointer = T_ptr_type;
        using reference = T_type;

    };

    
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    ////////////   ITER METHODS   ////////////
    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    const_iterator cbegin() const;
    const_iterator cend() const;

    reverse_iterator rbegin();
    reverse_iterator rend();

    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;

    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;


    ////////////   DEQUE METHODS   ////////////
    Deque();
    explicit Deque(int n, const T& value = T());
    Deque(const Deque&);
    ~Deque();

    Deque<T>& operator=(const Deque<T>&);

    [[nodiscard]] size_t size() const;
    [[nodiscard]] size_t capacity() const;
    bool empty() const;

    T& operator[](size_t);
    const T& operator[](size_t) const;

    T& at(size_t);
    const T& at(size_t) const;

    void push_front (const T&);
    void push_back (const T&);
    void pop_front ();
    void pop_back ();

    void insert(iterator, const T&);
    void erase(iterator);

};

bool operator<(std::pair<int, int> p1, std::pair<int, int> p2) {
    return p1.first < p2.first || (p1.first == p2.first && p1.second < p2.second);
}

bool operator<=(std::pair<int, int> p1, std::pair<int, int> p2) {
    return p1.first < p2.first || (p1.first == p2.first && p1.second <= p2.second);;
}

/*********************************************************/
////////////////////   ITERATORS   ///////////////////////
/*********************************************************/
template<typename T>
template<bool IsConst>
Deque<T>::common_iterator<IsConst>::common_iterator(std::conditional_t<IsConst, const std::vector<T*>,
        std::vector<T*>>& arr_ptr, std::pair<int, int> pos): it_pos(pos), arr_ptr(&arr_ptr) {
    ptr = arr_ptr[it_pos.first] + it_pos.second;
}

template<typename T>
template<bool IsConst>
std::conditional_t<IsConst, const T&, T&> Deque<T>::common_iterator<IsConst>::operator*() {
    return *ptr;
}

template<typename T>
template<bool IsConst>
std::conditional_t<IsConst, const T*, T*> Deque<T>::common_iterator<IsConst>::operator->() {
    return ptr;
}

template<typename T>
template<bool IsConst>
typename Deque<T>::template common_iterator<IsConst> &Deque<T>::common_iterator<IsConst>::operator++() {
    if (it_pos.second + 1 == bucket_sz) {
        ++it_pos.first;
        it_pos.second = 0;
        ptr = (*arr_ptr)[it_pos.first] + it_pos.second;
        return *this;
    }
    ++ptr;
    ++it_pos.second;
    return *this;
}
template<typename T>
template<bool IsConst>
typename Deque<T>::template common_iterator<IsConst> &Deque<T>::common_iterator<IsConst>::operator--() {
    if (it_pos.second - 1 == -1) {
        --it_pos.first;
        it_pos.second = bucket_sz - 1;
        ptr = (*arr_ptr)[it_pos.first] + it_pos.second;
        return *this;
    }
    --ptr;
    --it_pos.second;
    return *this;
}
template<typename T>
template<bool IsConst>
typename Deque<T>::template common_iterator<IsConst> Deque<T>::common_iterator<IsConst>::operator++(int) {
    common_iterator<IsConst> copy_it = *this;
    ++(*this);
    return copy_it;
}
template<typename T>
template<bool IsConst>
typename Deque<T>::template common_iterator<IsConst> Deque<T>::common_iterator<IsConst>::operator--(int) {
    common_iterator<IsConst> copy_it = *this;
    --(*this);
    return copy_it;
}

template<typename T>
template<bool IsConst>
void Deque<T>::common_iterator<IsConst>::move_pos(std::pair<int, int>& pos, int step) const {
    int absolut_position = pos.first * bucket_sz + pos.second;
    absolut_position += step;
    pos.first = absolut_position / bucket_sz;
    pos.second = absolut_position % bucket_sz;
}

template<typename T>
template<bool IsConst>
typename Deque<T>::template common_iterator<IsConst> Deque<T>::common_iterator<IsConst>::operator+(int val) const {
    common_iterator<IsConst> copy_it = *this;
    move_pos(copy_it.it_pos, val);
    copy_it.ptr = (*copy_it.arr_ptr)[copy_it.it_pos.first] + copy_it.it_pos.second;
    return copy_it;
}
template<typename T>
template<bool IsConst>
typename Deque<T>::template common_iterator<IsConst> Deque<T>::common_iterator<IsConst>::operator-(int val) const {
    common_iterator<IsConst> copy_it = *this;
    move_pos(copy_it.it_pos, -val);
    copy_it.ptr = (*copy_it.arr_ptr)[copy_it.it_pos.first] + copy_it.it_pos.second;
    return copy_it;
}
template<typename T>
template<bool IsConst>
typename Deque<T>::template common_iterator<IsConst>& Deque<T>::common_iterator<IsConst>::operator+=(int val) {
    move_pos(it_pos, val);
    ptr = (*arr_ptr)[it_pos.first] + it_pos.second;
    return *this;
}
template<typename T>
template<bool IsConst>
typename Deque<T>::template common_iterator<IsConst>& Deque<T>::common_iterator<IsConst>::operator-=(int val) {
    move_pos(it_pos, -val);
    ptr = (*arr_ptr)[it_pos.first] + it_pos.second;
    return *this;

}
template<typename T>
template<bool IsConst>
int Deque<T>::common_iterator<IsConst>::operator-(const common_iterator& it) const {
    int x = it_pos.first - it.it_pos.first;
    int y = it_pos.second - it.it_pos.second;
    return x * bucket_sz + y;
}


template<typename T>
template<bool IsConst>
bool Deque<T>::common_iterator<IsConst>::operator==(const common_iterator& it) const {
    return ptr == it.ptr;
}
template<typename T>
template<bool IsConst>
bool Deque<T>::common_iterator<IsConst>::operator!=(const common_iterator& it) const {
    return !(*this == it);
}
template<typename T>
template<bool IsConst>
bool Deque<T>::common_iterator<IsConst>::operator<(const common_iterator& it) const {
    return ptr < it.ptr;
}
template<typename T>
template<bool IsConst>
bool Deque<T>::common_iterator<IsConst>::operator<=(const common_iterator& it) const {
    return (*this == it) || (*this < it);
}
template<typename T>
template<bool IsConst>
bool Deque<T>::common_iterator<IsConst>::operator>(const common_iterator& it) const {
    return !(*this <= it);
}
template<typename T>
template<bool IsConst>
bool Deque<T>::common_iterator<IsConst>::operator>=(const common_iterator& it) const {
    return !(*this < it);
}



/*********************************************************/
////////////////////   DEQUE ITER   ///////////////////////
/*********************************************************/
template<typename T>
typename Deque<T>::iterator Deque<T>::begin() {
    return Deque::iterator(arr, begin_pos);
}

template<typename T>
typename Deque<T>::iterator Deque<T>::end() {
    return Deque::iterator(arr, end_pos);
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const {
    return Deque::const_iterator(arr, begin_pos);
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::end() const {
    return Deque::const_iterator(arr, end_pos);
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const {
    return Deque::const_iterator(arr, begin_pos);
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const {
    return Deque::const_iterator(arr, end_pos);
}

template<typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() {
    return Deque::reverse_iterator(end());
}

template<typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend() {
    return Deque::reverse_iterator(begin());
}

template<typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rbegin() const {
    return Deque::const_reverse_iterator(end());
}

template<typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rend() const {
    return Deque::const_reverse_iterator(begin());
}

template<typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() const {
    return Deque::const_reverse_iterator(end());
}

template<typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() const {
    return Deque::const_reverse_iterator(begin());
}



/********************************************************/
//////////////////////   DEQUE   /////////////////////////
/********************************************************/
template<typename T>
Deque<T>::Deque() {
    begin_pos = std::make_pair(0, bucket_sz / 2);
    end_pos = begin_pos;
    arr.resize(1);
    arr[0] = reinterpret_cast<T*>(new int8_t[bucket_sz * sizeof(T)]);
    n_buckets = 1;
}

template<typename T>
Deque<T>::Deque(int n, const T& value) {
    if (n < 0) throw std::bad_alloc();
    sz = n;
    n_buckets = sz / bucket_sz + (sz % bucket_sz ? 1 : 0) + 2;
    arr.resize(n_buckets);
    size_t i = 0, j;
    try {
        for (; i < arr.size(); ++i) {
            arr[i] = reinterpret_cast<T*>(new int8_t[bucket_sz * sizeof(T)]);
            T *pT = (T*)arr[i];
            for (j = 0; j < bucket_sz; ++j) {
                new (pT + j) T(value);
            }
        }
    } catch (...) {
        for (int k = 0; k <= i; ++k) {
            for (size_t t = 0; t < bucket_sz; ++k) {
                if (k == i && t == j) break;
                (arr[k] + t)->~T();
            }
            delete[] reinterpret_cast<T*>(arr[k]);
        }
        throw;
    }
    cap = n_buckets * bucket_sz;
    begin_pos = std::make_pair(1, 0);
    if (sz % bucket_sz) {
        end_pos = std::make_pair(n_buckets - 2, sz % bucket_sz);
    }
    else end_pos = std::make_pair(n_buckets - 1, 0);
}

template<typename T>
Deque<T>::Deque(const Deque& deque) {
    arr.resize(deque.n_buckets);
    int i = 0;
    try {
        for (; i < deque.n_buckets; ++i) {
            arr[i] = reinterpret_cast<T*>(new int8_t[bucket_sz * sizeof(T)]);
            std::uninitialized_copy(deque.arr[i], deque.arr[i] + bucket_sz, arr[i]);
        }
        n_buckets = deque.n_buckets;
        sz = deque.sz;
        cap = deque.cap;
        begin_pos = deque.begin_pos;
        end_pos = deque.end_pos;
    } catch (...) {
        for (int j = 0; j < i; ++j) {
            for (size_t t = 0; t < deque.bucket_sz; ++t) {
                (arr[j] + t)->~T();
            }
            delete[] reinterpret_cast<T*>(arr[j]);
        }
        throw;
    }
}

template<typename T>
Deque<T>::~Deque() {
    for (size_t i = 0; i < arr.size(); ++i) {
        for (size_t j = 0; j < bucket_sz; ++j) {
            if (begin_pos <= std::make_pair(i, j) && std::make_pair(i, j) < end_pos)
                (arr[i] + j)->~T();
        }
    }
    for (size_t i = 0; i < arr.size(); ++i) {
        delete[] reinterpret_cast<int8_t *>(arr[i]);
    }
}


template<typename T>
Deque<T> &Deque<T>::operator=(const Deque<T>& deque) {
    std::vector<T*> new_arr(deque.n_buckets);
    int i = 0;
    try {
        for (; i < deque.n_buckets; ++i) {
            new_arr[i] = reinterpret_cast<T*>(new int8_t[bucket_sz * sizeof(T)]);
            std::uninitialized_copy(deque.arr[i], deque.arr[i] + bucket_sz, new_arr[i]);
        }
        n_buckets = deque.n_buckets;
        sz = deque.sz;
        cap = deque.cap;
        begin_pos = deque.begin_pos;
        end_pos = deque.end_pos;
    } catch (...) {
        for (int j = 0; j < i; ++j) {
            for (size_t t = 0; t < deque.bucket_sz; ++j) {
                (new_arr[j] + t)->~T();
            }
            delete[] reinterpret_cast<T*>(new_arr[j]);
        }
        throw;
    }
    for (int i = 0; i < n_buckets; ++i) {
        for (size_t j = 0; j < bucket_sz; ++j)
            (arr[i] + j)->~T();
        delete[] reinterpret_cast<T*>(arr[i]);
    }
    arr = new_arr;
    return *this;
}

template<typename T>
void Deque<T>::double_size() {
    int old_n_buckets = n_buckets;
    n_buckets = 2 * n_buckets + (n_buckets % 2 ? 1 : 0);
    std::vector<T*> new_arr(n_buckets);
    int delta = (n_buckets - old_n_buckets) / 2;
    for (int i = 0; i < static_cast<int>(new_arr.size()); ++i) {
        new_arr[i] = reinterpret_cast<T*>(new int8_t[bucket_sz * sizeof(T)]);
        if (i >= delta && i < n_buckets - delta)
            new_arr[i] = arr[i - delta];
    }
    begin_pos.first += delta;
    end_pos.first += delta;
    cap = n_buckets * bucket_sz;
    arr = new_arr;
}

template<typename T>
size_t Deque<T>::size() const {
    return sz;
}
template<typename T>
size_t Deque<T>::capacity() const {
    return cap;
}

template<typename T>
bool Deque<T>::empty() const {
    return sz == 0;
}

template<typename T>
T& Deque<T>::operator[](size_t i) {
    std::pair<int, int> copy_pos = begin_pos;
    move_pos(copy_pos, i);
    return arr[copy_pos.first][copy_pos.second];
}

template<typename T>
const T& Deque<T>::operator[](size_t i) const {
    std::pair<int, int> copy_pos = begin_pos;
    move_pos(copy_pos, i);
    return arr[copy_pos.first][copy_pos.second];
}

template<typename T>
T& Deque<T>::at(size_t i) {
    if (i >= sz) throw std::out_of_range("deque.at()");
    return (*this)[i];
}

template<typename T>
const T& Deque<T>::at(size_t i) const {
    if (i >= sz) throw std::out_of_range("deque.at()");
    return (*this)[i];
}

template<typename T>
void Deque<T>::push_front(const T& value) {
    ++sz;
    if (begin_pos.first == 0 && begin_pos.second == 1) {
        double_size();
    }
    move_pos_back(begin_pos);
    new (arr[begin_pos.first] + begin_pos.second) T(value);
}

template<typename T>
void Deque<T>::push_back(const T& value) {
    ++sz;
    if (end_pos.first == n_buckets - 1 && end_pos.second == bucket_sz - 2) {
        double_size();
    }
    new (arr[end_pos.first] + end_pos.second) T(value);
    move_pos_forward(end_pos);
}

template<typename T>
void Deque<T>::pop_front() {
    --sz;
    (arr[begin_pos.first] + begin_pos.second)->~T();
    move_pos_forward(begin_pos);
}

template<typename T>
void Deque<T>::pop_back() {
    --sz;
    move_pos_back(end_pos);
    (arr[end_pos.first] + end_pos.second)->~T();
}

template<typename T>
void Deque<T>::move_pos_forward(std::pair<int, int> &pos) {
    pos.second = pos.second + 1;
    if (pos.second == bucket_sz) {
        ++pos.first;
        pos.second = 0;
    }
}
template<typename T>
void Deque<T>::move_pos_back(std::pair<int, int> &pos) {
    if (pos.second == 0) {
        --pos.first;
        pos.second = bucket_sz - 1;
    }
    else pos.second = pos.second - 1;
}

template<typename T>
void Deque<T>::move_pos(std::pair<int, int>& pos, int step) const {
    int absolut_position = pos.first * bucket_sz + pos.second;
    absolut_position += step;
    pos.first = absolut_position / bucket_sz;
    pos.second = absolut_position % bucket_sz;
}

template<typename T>
void Deque<T>::insert(Deque::iterator it, const T &el) {
    try {
        iterator begin_it = this->begin();
        int num = it - begin_it;
        T x = (*this)[num];
        (*this)[num] = el;
        for (size_t i = num + 1; i < sz; ++i) {
            std::swap((*this)[i], x);
        }
        this->push_back(x);
    } catch (...) {
        throw;
    }
}

template<typename T>
void Deque<T>::erase(Deque::iterator it) {
    iterator begin_it = this->begin();
    int num = it - begin_it;
    for (size_t i = num; i < sz - 1; ++i)
        (*this)[i] = (*this)[i + 1];
    this->pop_back();
}
