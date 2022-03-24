#include <iostream>
#include <vector>
#include <type_traits>
#include <ctime>


/********************************************************/
//////////////////   FixedAllocator   ////////////////////
/********************************************************/
template <size_t chunkSize>
class FixedAllocator {
private:
    // vector of free blocks of memory
    std::vector<void*> pool;
    static const size_t size = 150;
public:
    FixedAllocator() = default;
    ~FixedAllocator() = default;
    static FixedAllocator<chunkSize>& get_fixed_alloc();
    void* allocate();
    void deallocate(void*, size_t);
};

/////////////   DEFINITIONS   /////////////
template<size_t chunkSize>
FixedAllocator<chunkSize> &FixedAllocator<chunkSize>::get_fixed_alloc() {
    static FixedAllocator<chunkSize> f_alloc;
    return f_alloc;
}

template<size_t chunkSize>
void *FixedAllocator<chunkSize>::allocate() {
    if (pool.empty()) {
        auto *ptr = static_cast<int8_t*>(::operator new(size * chunkSize));
        for(size_t i = 0; i < size; ++i)
            pool.push_back(static_cast<void*>(ptr + i * chunkSize));
    }
    void *last_ptr = pool.back();
    pool.pop_back();
    return last_ptr;
}

template<size_t chunkSize>
void FixedAllocator<chunkSize>::deallocate(void *el, size_t n) {
    if (n == chunkSize)
        pool.push_back(el);
}



/********************************************************/
//////////////////   FastAllocator   /////////////////////
/********************************************************/
template <typename T>
class FastAllocator {
private:
    int nearest_power2(int x);
public:
    FastAllocator() = default;
    ~FastAllocator() = default;
    T* allocate(size_t);
    void deallocate(T*, size_t);

    template<typename U>
    struct rebind {
        using other = FastAllocator<U>;
    };
    template<typename U>
    FastAllocator(const FastAllocator<U>&) {}

    // named requirements
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using value_type = T;
};

/////////////   DEFINITIONS   /////////////
template<typename T>
int FastAllocator<T>::nearest_power2(int x) { // nearest_size
    if (x <= 4) return 4;
    if (x <= 16) return 16;
    if (x <= 24) return 24;
    return 33; // The Age of Christ

}

template<typename T>
T *FastAllocator<T>::allocate(size_t n) {
    const int expected_size = nearest_power2(n * sizeof(T));
    if (expected_size == 4)
        return static_cast<T*>(FixedAllocator<4>::get_fixed_alloc().allocate());
    else if (expected_size == 16)
        return static_cast<T*>(FixedAllocator<16>::get_fixed_alloc().allocate());
    else if (expected_size == 24)
        return static_cast<T*>(FixedAllocator<24>::get_fixed_alloc().allocate());
    else
        return static_cast<T*>(::operator new(n * sizeof(T)));
}
template<typename T>
void FastAllocator<T>::deallocate(T *ptr, size_t n) {
    const int expected_size = nearest_power2(n * sizeof(T));
    if (expected_size == 4)
        FixedAllocator<4>::get_fixed_alloc().deallocate(ptr, expected_size);
    else if (expected_size == 16)
        FixedAllocator<16>::get_fixed_alloc().deallocate(ptr, expected_size);
    else if (expected_size == 24)
        FixedAllocator<24>::get_fixed_alloc().deallocate(ptr, expected_size);
    else
        ::operator delete(ptr);
}
template<typename T, typename  U>
bool operator==(const FastAllocator<T>&, const FastAllocator<U>&) noexcept {
    return true;
}
template<typename T, typename  U>
bool operator!=(const FastAllocator<T>&, const FastAllocator<U>&) noexcept {
    return false;
}



/********************************************************/
///////////////////////   LIST   /////////////////////////
/********************************************************/
template<typename T, typename Alloc = std::allocator<T>>
class List {
private:

    struct Node {
        T val;
        Node* next = nullptr;
        Node* prev = nullptr;

        Node() = default;
        explicit Node(const T& val) : val(val) {}
        ~Node() = default;
    };

    size_t sz = 0;
    Alloc t_allocator;
    typename Alloc::template rebind<Node>::other allocator;
    Node* head;

    void link_nodes(Node* v1, Node* v2) const {
        v2->next = v1->next;
        v2->prev = v1;
        (v1->next)->prev = v2;
        v1->next = v2;
    }
    void del_node(const Node* v) const {// unlink
        (v->prev)->next = v->next;
        (v->next)->prev = v->prev;
    }

public:

    ////////////   ITERATORS   ////////////
    template <bool IsConst>
    class common_iterator {
    public:
        std::conditional_t<IsConst, const Node*, Node*> ptr_node;

    public:
        operator common_iterator<true>();

        common_iterator();
        common_iterator(std::conditional_t<IsConst, const Node*, Node*>);
        // менять её точно не нужно
        common_iterator(const common_iterator<IsConst>&);
        std::conditional_t<IsConst, const T&, T&> operator*();  // reference
        std::conditional_t<IsConst, const T*, T*> operator->(); // pointer

        common_iterator& operator++();
        common_iterator& operator--();
        common_iterator operator++(int);
        common_iterator operator--(int);

        bool operator==(const common_iterator&) const;
        bool operator!=(const common_iterator&) const;


        using T_ptr_type = std::conditional_t<IsConst, const T*, T*>;
        using T_type = std::conditional_t<IsConst, const T&, T&>;

        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using allocator_type = Alloc;
        using iterator_category = std::bidirectional_iterator_tag;
        using pointer = T_ptr_type;
        using reference = T_type;

    };

    using t_node_alloc = std::allocator_traits<Alloc>;
    using node_alloc = std::allocator_traits<typename Alloc::template rebind<Node>::other>;
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


    ////////////   LIST METHODS   ////////////
    explicit List(const Alloc& alloc = Alloc());
    explicit List(size_t, const T& val, const Alloc& alloc = Alloc());
    explicit List(size_t, const Alloc& alloc = Alloc());
    List(const List&);
    ~List();

    List& operator =(const List&);

    Alloc get_allocator() const;

    size_t size() const;
    void push_back(const T&);
    void push_front(const T&);
    void pop_back();
    void pop_front();

    template<bool IsConst>
    iterator insert(const common_iterator<IsConst>&, const T&);
    template<bool IsConst>
    iterator erase(const common_iterator<IsConst>&);
};



/////////////   ITER DEFINITIONS   /////////////
template<typename T, typename Alloc>
template<bool IsConst>
List<T, Alloc>::common_iterator<IsConst>::operator common_iterator<true> () {
    common_iterator<true> it(ptr_node);
    return it;
}
template<typename T, typename Alloc>
template<bool IsConst>
List<T, Alloc>::common_iterator<IsConst>::common_iterator() {
    ptr_node = nullptr;
}
template<typename T, typename Alloc>
template<bool IsConst>
List<T, Alloc>::common_iterator<IsConst>::common_iterator(std::conditional_t<IsConst, const Node*, Node*> node) {
    ptr_node = node;
}
template<typename T, typename Alloc>
template<bool IsConst>
List<T, Alloc>::common_iterator<IsConst>::common_iterator(const List<T, Alloc>::common_iterator<IsConst>& it) {
    ptr_node = it.ptr_node;
}
template<typename T, typename Alloc>
template<bool IsConst>
std::conditional_t<IsConst, const T&, T&> List<T, Alloc>::common_iterator<IsConst>::operator*() {
    return ptr_node->val;
}
template<typename T, typename Alloc>
template<bool IsConst>
std::conditional_t<IsConst, const T*, T*> List<T, Alloc>::common_iterator<IsConst>::operator->() {
    return &(ptr_node->val);
}
template<typename T, typename Alloc>
template<bool IsConst>
typename List<T, Alloc>::template common_iterator<IsConst> &List<T, Alloc>::common_iterator<IsConst>::operator++() {
    ptr_node = ptr_node->next;
    return *this;
}
template<typename T, typename  Alloc>
template<bool IsConst>
typename List<T, Alloc>::template common_iterator<IsConst> &List<T, Alloc>::common_iterator<IsConst>::operator--() {
    ptr_node = ptr_node->prev;
    return *this;
}
template<typename T, typename  Alloc>
template<bool IsConst>
typename List<T, Alloc>::template common_iterator<IsConst> List<T, Alloc>::common_iterator<IsConst>::operator++(int) {
    common_iterator<IsConst> copy_it = *this;
    ++(*this);
    return copy_it;
}
template<typename T, typename  Alloc>
template<bool IsConst>
typename List<T, Alloc>::template common_iterator<IsConst> List<T, Alloc>::common_iterator<IsConst>::operator--(int) {
    common_iterator<IsConst> copy_it = *this;
    --(*this);
    return copy_it;
}


template<typename T, typename  Alloc>
template<bool IsConst>
bool List<T, Alloc>::common_iterator<IsConst>::operator==(const common_iterator& it) const {
    return ptr_node == it.ptr_node;
}
template<typename T, typename  Alloc>
template<bool IsConst>
bool List<T, Alloc>::common_iterator<IsConst>::operator!=(const common_iterator& it) const {
    return !(*this == it);
}


/////////////   LIST ITER DEFINITIONS   /////////////
template<typename T, typename  Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::begin() {
    return List::iterator(head->next);
}
template<typename T, typename  Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::end() {
    return List::iterator(head);
}
template<typename T, typename  Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::begin() const {
    return List::const_iterator(head->next);
}
template<typename T, typename  Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::end() const {
    return List::const_iterator(head);
}
template<typename T, typename  Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::cbegin() const {
    return List::const_iterator(head->next);
}
template<typename T, typename  Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::cend() const {
    return List::iterator(head);
}
template<typename T, typename  Alloc>
typename List<T, Alloc>::reverse_iterator List<T, Alloc>::rbegin() {
    return List::reverse_iterator(end());
}
template<typename T, typename  Alloc>
typename List<T, Alloc>::reverse_iterator List<T, Alloc>::rend() {
    return List::reverse_iterator(begin());
}
template<typename T, typename  Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::rbegin() const {
    return List::const_reverse_iterator(end());
}
template<typename T, typename  Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::rend() const {
    return List::const_reverse_iterator(begin());
}
template<typename T, typename  Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::crbegin() const {
    return List::const_reverse_iterator(end());
}
template<typename T, typename  Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::crend() const {
    return List::const_reverse_iterator(begin());
}



/////////////   LIST DEFINITIONS   /////////////
template<typename T, typename Alloc>
List<T, Alloc>::List(const Alloc &alloc) : sz(0), t_allocator(alloc) {
    head = node_alloc::allocate(allocator, 1);
    head->next = head;
    head->prev = head;
}
template<typename T, typename Alloc>
List<T, Alloc>::List(size_t n, const T &val, const Alloc &alloc) : List(alloc) {
    if (n < 0) throw std::bad_alloc();
    size_t i = 0;
    try {
        for (; i < n; ++i) {
            Node *new_node = node_alloc::allocate(allocator, 1);
            node_alloc::construct(allocator, new_node, val);
            link_nodes(head->next, new_node);
        }
    } catch (...) {
        for (size_t j = 0; j < i; ++j) {
            Node* copy_node = head->prev;
            del_node(head->prev);
            node_alloc::destroy(allocator, copy_node);
            node_alloc::deallocate(allocator, copy_node, 1);
        }
        node_alloc::deallocate(allocator, head, 1);
        throw;
    }
    sz = n;
}
template<typename T, typename Alloc>
List<T, Alloc>::List(size_t n, const Alloc &alloc): List(alloc) {
    if (n < 0) throw std::bad_alloc();
    size_t i = 0;
    try {
        for (; i < n; ++i) {
            Node *new_node = node_alloc::allocate(allocator, 1);
            node_alloc::construct(allocator, new_node);
            link_nodes(head->next, new_node);
        }
    } catch (...) {
        for (size_t j = 0; j < i; ++j) {
            Node* copy_node = head->prev;
            del_node(head->prev);
            node_alloc::destroy(allocator, copy_node);
            node_alloc::deallocate(allocator, copy_node, 1);
        }
        node_alloc::deallocate(allocator, head, 1);
        throw;
    }
    sz = n;
}
template<typename T, typename Alloc>
List<T, Alloc>::List(const List &l) : sz(l.sz),\
                                    t_allocator(t_node_alloc::select_on_container_copy_construction(l.t_allocator)),\
                                    allocator(node_alloc::select_on_container_copy_construction(l.allocator)) {
    Node *new_head = node_alloc::allocate(allocator, 1);
    new_head->next = new_head;
    new_head->prev = new_head;

    Node* l_begin = l.head->next;
    size_t i = 0;
    try {
        for (; i < l.sz; ++i) {
            Node* new_node = node_alloc::allocate(allocator, 1);
            node_alloc::construct(allocator, new_node, l_begin->val);
            link_nodes(new_head->prev, new_node);
            l_begin = l_begin->next;
        }
    } catch (...) {
        for (size_t j = 0; j < i; ++j) {
            Node* copy_node = new_head->prev;
            del_node(new_head->prev);
            node_alloc::destroy(allocator, copy_node);
            node_alloc::deallocate(allocator, copy_node, 1);
        }
        node_alloc::deallocate(allocator, new_head, 1);
        throw;
    }
    sz = l.sz;
    head = node_alloc::allocate(allocator, 1);
    head = new_head;
}
template<typename T, typename Alloc>
List<T, Alloc>::~List() {
    while (sz != 0)
        pop_back();
    node_alloc::deallocate(allocator, head, 1);
}
template<typename T, typename Alloc>
List<T, Alloc> &List<T, Alloc>::operator=(const List &l) {
    while (sz != 0)
        pop_back();
    if (node_alloc::propagate_on_container_copy_assignment::value) {
        node_alloc::deallocate(allocator, head, 1);
        allocator = l.allocator;
        t_allocator = l.t_allocator;
        head = node_alloc::allocate(allocator, 1);
        head->next = head;
        head->prev = head;
    }
    Node* l_begin = l.head->next;
    for (size_t i = 0; i < l.sz; ++i) {
        push_back(l_begin->val);
        l_begin = l_begin->next;
    }
    return *this;
}
template<typename T, typename Alloc>
Alloc List<T, Alloc>::get_allocator() const {
    return t_allocator;
}
template<typename T, typename Alloc>
size_t List<T, Alloc>::size() const {
    return sz;
}

template<typename T, typename Alloc>
void List<T, Alloc>::push_back(const T &val) {
    insert(end(), val);
}
template<typename T, typename Alloc>
void List<T, Alloc>::push_front(const T &val) {
    insert(begin(), val);
}
template<typename T, typename Alloc>
void List<T, Alloc>::pop_back() {
    erase(--end());
}
template<typename T, typename Alloc>
void List<T, Alloc>::pop_front() {
    erase(begin());
}
template<typename T, typename Alloc>
template<bool IsConst>
typename List<T, Alloc>::iterator  List<T, Alloc>::insert(const List::common_iterator<IsConst> &it, const T &val) {
    Node* new_node = node_alloc::allocate(allocator, 1);
    try {
        node_alloc::construct(allocator, new_node, val);
        link_nodes((it.ptr_node)->prev, new_node);
    } catch (...) {
        Node* copy_node = new_node;
        del_node(new_node);
        node_alloc::destroy(allocator, copy_node);
        node_alloc::deallocate(allocator, copy_node, 1);
        throw;
    }
    ++sz;
    return List::iterator(new_node);
}
template<typename T, typename Alloc>
template<bool IsConst>
typename List<T, Alloc>::iterator List<T, Alloc>::erase(const List::common_iterator<IsConst> &it) {
    Node* copy_node = const_cast<Node*>(it.ptr_node);
    iterator copy_it(copy_node);
    del_node(it.ptr_node);
    node_alloc::destroy(allocator, it.ptr_node);
    node_alloc::deallocate(allocator, copy_node, 1);
    --sz;
    return ++copy_it;
}
