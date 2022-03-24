#include <iostream>
#include <vector>
#include <type_traits>


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
int FastAllocator<T>::nearest_power2(int x) {
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

        void swap(Node& node) {
            std::swap(val, node.val);
            std::swap(prev, node.prev);
            std::swap(next, node.next);
        }

        Node() = default;
        Node(const T& val) : val(val) {}
        Node(T&& val) : val(std::move(val)) {}

        template<typename... Args>
        Node(Args &&... args) : val(std::forward<Args&&>(args)...) {}

        Node(Node&& node) noexcept : val(std::move(node.val)) {
            prev = node.prev;
            next = node.next;
            node.prev = node.next = nullptr;
        }
        Node& operator=(Node&& node) noexcept {
            Node copy = std::move(node);
            swap(copy);
            return *this;
        }
        ~Node() = default;
    };

    size_t sz = 0;
    Alloc t_allocator;
    typename Alloc::template rebind<Node>::other allocator;
    typename Alloc::template rebind<T>::other type_allocator;
    Node* head;

    void link_nodes(Node* v1, Node* v2) const {
        v2->next = v1->next;
        v2->prev = v1;
        (v1->next)->prev = v2;
        v1->next = v2;
    }
    void del_node(const Node* v) const {
        (v->prev)->next = v->next;
        (v->next)->prev = v->prev;
    }

public:

    ////////////   ITERATORS   ////////////
    template<bool IsConst>
    class common_iterator {
    public:
        std::conditional_t<IsConst, const Node*, Node*> ptr_node;

    public:
        operator common_iterator<true>();

        common_iterator();
        common_iterator(std::conditional_t<IsConst, const Node*, Node*>);
        common_iterator(const common_iterator<IsConst>&);
        std::conditional_t<IsConst, const T&, T&> operator*();
        std::conditional_t<IsConst, const T*, T*> operator->();

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
    using type_alloc = std::allocator_traits<typename Alloc::template rebind<T>::other>;
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
    List(List&&) noexcept;
    ~List();

    List& operator=(const List&);
    List& operator=(List&&) noexcept;

    Alloc get_allocator() const;

    size_t size() const;
    void push_back(const T&);
    void push_front(const T&);
    void pop_back();
    void pop_front();

    template<bool IsConst>
    iterator insert(const common_iterator<IsConst>&, const T&);
    template<bool IsConst>
    iterator insert(const common_iterator<IsConst>&, const T&&);
    template<bool IsConst>
    iterator erase(const common_iterator<IsConst>&);
    template<typename... Args>
    iterator emplace(const iterator&, Args&&...);
    template<bool IsConst>
    void splice(const common_iterator<IsConst>&, List&, const common_iterator<IsConst>&);
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
List<T, Alloc>::List(List &&l) noexcept : sz(l.sz), \
                                t_allocator(std::move(l.t_allocator)), allocator(std::move(l.allocator)) {
    head = node_alloc::allocate(allocator, 1);
    std::swap(head, l.head);
    l.head->next = l.head->prev = l.head;
    l.sz = 0;
}
template<typename T, typename Alloc>
List<T, Alloc>::~List() {
    while (sz != 0)
        pop_back();
    node_alloc::deallocate(allocator, head, 1);
}
template<typename T, typename Alloc>
List<T, Alloc> &List<T, Alloc>::operator=(const List &l) {
    if (this == &l)
        return *this;
    while (sz != 0)
        pop_back();
    if (node_alloc::propagate_on_container_copy_assignment::value) {
        if (t_allocator != l.t_allocator) {
            node_alloc::deallocate(allocator, head, 1);
            allocator = l.allocator;
            t_allocator = l.t_allocator;
            head = node_alloc::allocate(allocator, 1);
            head->next = head;
            head->prev = head;
        }
    }
    Node* l_begin = l.head->next;
    for (size_t i = 0; i < l.sz; ++i) {
        push_back(l_begin->val);
        l_begin = l_begin->next;
    }
    return *this;
}
template<typename T, typename Alloc>
List<T, Alloc> &List<T, Alloc>::operator=(List &&l) noexcept {
    if (this == &l)
        return *this;
    while (sz != 0)
        pop_back();
    if (node_alloc::propagate_on_container_move_assignment::value) {
        if (t_allocator != l.t_allocator) {
            node_alloc::deallocate(allocator, head, 1);
            allocator = l.allocator;
            t_allocator = l.t_allocator;
            head = node_alloc::allocate(allocator, 1);
            head->next = head;
            head->prev = head;
        }
    }
    std::swap(head, l.head);
    std::swap(sz, l.sz);
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
typename List<T, Alloc>::iterator List<T, Alloc>::insert(const List::common_iterator<IsConst> &it, const T &&val) {
    Node* new_node = node_alloc::allocate(allocator, 1);
    try {
        node_alloc::construct(allocator, new_node, std::move(val));
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
template<typename T, typename Alloc>
template<typename... Args>
typename List<T, Alloc>::iterator List<T, Alloc>::emplace(const List::iterator &it, Args &&... args) {
    Node* new_node = node_alloc::allocate(allocator, 1);
    try {
        type_alloc::construct(type_allocator, &(new_node->val), std::forward<Args>(args)...);
        link_nodes((it.ptr_node)->prev, new_node);
    } catch (...) {
        Node* copy_node = new_node;
        del_node(new_node);
        type_alloc::destroy(type_allocator, &(copy_node->val));
        node_alloc::deallocate(allocator, copy_node, 1);
        throw;
    }
    ++sz;
    return List::iterator(new_node);
}

template<typename T, typename Alloc>
template<bool IsConst>
void List<T, Alloc>::splice(const List::common_iterator<IsConst> &pos, List &l, const List::common_iterator<IsConst> &it) {
    l.del_node(it.ptr_node);
    link_nodes(pos.ptr_node, it.ptr_node);
    ++sz;
    --l.sz;
}





/*******************************************************/
///////////////////////   MAP   /////////////////////////
/*******************************************************/
template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>,
        typename Alloc = FastAllocator<std::pair<const Key, Value>>>
class UnorderedMap {
public:
    using NodeType = std::pair<const Key, Value>;
private:
    static const int def_sz = 1024;
    size_t max_sz = def_sz;

    std::vector<typename List<NodeType, Alloc>::iterator> buckets_its;
    List<NodeType, Alloc> hash_table; //all elements as a chain
    float l_factor = 0.f;
    float max_l_factor = 1.f;


    using node_alloc = std::allocator_traits<typename std::allocator_traits<Alloc>::template rebind_alloc<NodeType>>;
    typename std::allocator_traits<Alloc>::template rebind_alloc<NodeType> allocator;

    void rehash(size_t);
    void initialise();
    void reset_bucket_it();
    void recount_load_factor();
    size_t get_index(const Key&);

public:

    ////////////   ITERATORS   ////////////
    using iterator = typename List<NodeType, Alloc>::iterator;
    using const_iterator = typename List<NodeType, Alloc>::const_iterator;
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


    ////////////   MAP METHODS   ////////////
    UnorderedMap();
    UnorderedMap(const UnorderedMap&);
    UnorderedMap(UnorderedMap&&) noexcept;
    ~UnorderedMap() = default;

    UnorderedMap<Key, Value, Hash, Equal, Alloc>& operator=(const UnorderedMap&);
    UnorderedMap<Key, Value, Hash, Equal, Alloc>& operator=(UnorderedMap&&) noexcept;

    [[nodiscard]] size_t size() const;
    [[nodiscard]] float load_factor() const;
    [[nodiscard]] float max_load_factor() const;
    [[nodiscard]] size_t max_size() const;

    void reserve(size_t);

    Value& operator[](const Key&);
    const Value& operator[](const Key&) const;
    Value& at(const Key&);
    const Value& at(const Key&) const;

    template<typename NodeTypeT = std::pair<const Key, Value>>
    std::pair<iterator, bool> insert(NodeTypeT&&);

    template <typename InputIterator>
    void insert(InputIterator, InputIterator);

    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&...);

    UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator find(const Key&);
    UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_iterator find(const Key&) const;

    void erase(iterator);
    void erase(iterator, iterator);
};


/////////////   MAP ITER DEFINITIONS   /////////////
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::begin() {
    return hash_table.begin();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::end() {
    return hash_table.end();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::begin() const {
    return hash_table.cbegin();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::end() const {
    return hash_table.cend();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::cbegin() const {
    return hash_table.cbegin();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::cend() const {
    return hash_table.cend();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::reverse_iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::rbegin() {
    return hash_table.rbegin();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::reverse_iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::rend() {
    return hash_table.rend();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_reverse_iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::rbegin() const {
    return hash_table.crbegin();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_reverse_iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::rend() const {
    return hash_table.crend();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_reverse_iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::crbegin() const {
    return hash_table.crbegin();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_reverse_iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::crend() const {
    return hash_table.crend();
}


/////////////   MAP DEFINITIONS   /////////////
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::rehash(size_t new_max_sz) {
    if (new_max_sz > max_sz)
        reserve(new_max_sz);
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::initialise() {
    buckets_its.resize(def_sz, hash_table.end());
    max_sz = def_sz;
    l_factor = 0.0;
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::reset_bucket_it() {
    buckets_its.resize(max_sz, hash_table.end());
    for (iterator it = hash_table.begin(); it != hash_table.end(); ++it) {
        size_t ind = get_index(it->first);
        if (buckets_its[ind] == hash_table.end()) {
            buckets_its[ind] = it;
        }
    }
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::recount_load_factor() {
    l_factor = static_cast<float>(static_cast<float>(size()) / static_cast<float>(max_sz));
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::get_index(const Key& key) {
    return Hash{}(key) % max_sz;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap() {
    initialise();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap(const UnorderedMap &m) : max_sz(m.max_sz),\
                                    hash_table(m.hash_table), l_factor(m.l_factor) {
    reset_bucket_it();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc>::UnorderedMap(UnorderedMap &&m) noexcept : max_sz(m.max_sz), \
                                    hash_table(std::move(m.hash_table)), l_factor(m.l_factor) {
    m.initialise();
    reset_bucket_it();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc> &
UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator=(const UnorderedMap &m) {
    if (this == &m)
        return *this;
    hash_table = m.hash_table;
    max_sz = m.max_sz;
    l_factor = m.l_factor;
    reset_bucket_it();
    return *this;
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
UnorderedMap<Key, Value, Hash, Equal, Alloc> &
UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator=(UnorderedMap &&m) noexcept {
    if (this == &m)
        return *this;
    buckets_its = std::move(m.buckets_its);
    hash_table = std::move(m.hash_table);
    max_sz = m.max_sz;
    l_factor = m.l_factor;
    m.initialise();
    reset_bucket_it();
    return *this;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::size() const {
    return hash_table.size();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
float UnorderedMap<Key, Value, Hash, Equal, Alloc>::load_factor() const {
    return l_factor;
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
float UnorderedMap<Key, Value, Hash, Equal, Alloc>::max_load_factor() const {
    return max_l_factor;
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
size_t UnorderedMap<Key, Value, Hash, Equal, Alloc>::max_size() const {
    return max_sz;
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::reserve(size_t new_sz) {
    max_sz = new_sz;
    List<NodeType, Alloc> old = std::move(hash_table);
    buckets_its.clear();
    buckets_its.resize(max_sz, hash_table.end());
    while(old.begin() != old.end()) {
        auto it = old.begin();
        size_t ind = get_index(it->first);
        if (buckets_its[ind] == hash_table.end()) {
            hash_table.splice(hash_table.begin(), old, it);
            buckets_its[ind] = it;
        }
        else {
            hash_table.splice(buckets_its[ind], old, it);
        }
    }
    recount_load_factor();
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
Value &UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator[](const Key &key) {
    return insert(std::move(std::make_pair(key, Value()))).first->second;
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
const Value &UnorderedMap<Key, Value, Hash, Equal, Alloc>::operator[](const Key &key) const {
    return const_cast<const Value>((*this)[key]);
    //По идее const [] не существует, но раз он тут, то лучше к симметричной at реализации обратиться с out_of_range, так как поведение сильно отличается от [] простых
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
Value &UnorderedMap<Key, Value, Hash, Equal, Alloc>::at(const Key &key) {
    auto it = find(key);
    if (it == end())
        throw std::out_of_range("container does not have an element with the specified key");
    return it->second;
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
const Value &UnorderedMap<Key, Value, Hash, Equal, Alloc>::at(const Key &key) const {
    return const_cast<const Value>((*this).at(key));
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<typename NodeTypeT>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator, bool> UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(NodeTypeT &&node) {
    iterator it = find(node.first);
    if (it == end()) {
        if (l_factor >= max_l_factor)
            rehash(2 * max_sz);
        size_t ind = get_index(node.first);
        if (buckets_its[ind] == hash_table.end()) {
            hash_table.emplace(hash_table.begin(), std::forward<NodeTypeT>(node));
            buckets_its[ind] = hash_table.begin();
        }
        else {
            hash_table.emplace(buckets_its[ind], std::forward<NodeTypeT>(node));
            --buckets_its[ind];
        }
        recount_load_factor();
        return {buckets_its[ind], true};
    }
    return {it, false};
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<typename InputIterator>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::insert(InputIterator begin, InputIterator end) {
    for (;begin != end; ++begin) {
        insert(*begin);
    }
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
template<typename... Args>
std::pair<typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator, bool> UnorderedMap<Key, Value, Hash, Equal, Alloc>::emplace(Args &&... args) {
    if (l_factor >= max_l_factor)
        rehash(2 * max_sz);
    auto node = node_alloc::allocate(allocator, 1);
    node_alloc::construct(allocator, node, std::forward<Args>(args)...);
    size_t ind = get_index(node->first);

    auto it = buckets_its[ind];
    if (it != end()) {
        for (; it != end() && get_index(it->first) == ind; ++it) {
            if (Equal{}(it->first, node->first)) {
                node_alloc::destroy(allocator, node);
                node_alloc::deallocate(allocator, node, 1);
                return {it, false};
            }
        }
    }
    hash_table.emplace(hash_table.begin(), std::forward<Args>(args)...);
    it = begin();
    node_alloc::destroy(allocator, node);
    node_alloc::deallocate(allocator, node, 1);
    recount_load_factor();
    return std::make_pair(it, true);
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::find(const Key& key) {
    size_t ind = get_index(key);
    auto it = buckets_its[ind];
    for (; it != hash_table.end(); ++it) {
        if (Equal{}(it->first, key))
            return it;
    }
    return end();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
typename UnorderedMap<Key, Value, Hash, Equal, Alloc>::const_iterator UnorderedMap<Key, Value, Hash, Equal, Alloc>::find(const Key& key) const {
    size_t ind = get_index(key);
    auto it = buckets_its[ind];
    for (; it != hash_table.end(); ++it) {
        if (Equal{}(it->first, key))
            return UnorderedMap::const_iterator(it);
    }
    return cend();
}

template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::erase(UnorderedMap::iterator it) {
    size_t ind = get_index(it->first);
    auto el = buckets_its[ind];
    auto next = el;
    ++next;
    size_t next_ind = get_index(next->first);
    if (next == end() || next_ind != ind) {
        hash_table.erase(el);
        el = hash_table.end();
        return;
    }
    if (next_ind == ind && Equal{}(el->first, it->first)) {
        hash_table.erase(el);
        el = next;
        return;
    }
    for (; !Equal{}(el->first, it->first); ++el);
    hash_table.erase(el);
    recount_load_factor();
}
template<typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
void UnorderedMap<Key, Value, Hash, Equal, Alloc>::erase(UnorderedMap::iterator begin, UnorderedMap::iterator end) {
    for (auto it = begin; it != end;)
        erase(it++);
}
