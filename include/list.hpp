#ifndef REGEX_LIST
#define REGEX_LIST


#include <utility>
#include <functional>


class ListImpl {
public:
  struct ListNode {
    ListNode *prev;
    ListNode *next;
  };

  ListNode *first;
  ListNode *last;

  void drop() {
    delete first;
    delete last;
    first = nullptr;
    last = nullptr;
  }

  void emplace(ListImpl &&other) {
    first->next = other.first->next;
    last->prev = other.last->prev;
    other.first->next = other.last;
    other.last->prev = other.first;
  }

  ListImpl() :
      first{new ListNode{nullptr, nullptr}},
      last{new ListNode{nullptr, nullptr}}
  {
    first->next = last;
    last->prev = first;
  }

  void insert_back(ListNode *node) {
    last->prev->next = node;
    node->prev = last->prev;
    node->next = last;
    last->prev = node;
  }

  void give_up_node(ListNode *node, ListImpl &other) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    other.insert_back(node);
  }

  void give_up_nodes(ListImpl &other) {
    other.last->prev->next = first->next;
    first->next->prev = other.last->prev;
    last->prev->next = other.last;
    other.last->prev = last->prev;
    first->next = last;
    last->prev = first;
  }

  ListImpl(const ListImpl &other) = delete;

  ListImpl(ListImpl &&other) { emplace(std::move(other)); }

  ListImpl &operator=(const ListImpl &other) = delete;

  ListImpl &operator=(ListImpl &&other) {
    drop();
    emplace(std::move(other));
    return *this;
  }

  ~ListImpl() { drop(); }
};

template<class T>
class ListIter;

template<class T>
class List {
public:
  using Iter = ListIter<T>;

private:
  friend class ListIter<T>;

  struct ListNode : ListImpl::ListNode {
    T elem;

    template<class ...Args>
    ListNode(Args &&...args) :
        ListImpl::ListNode{}, elem{std::forward<Args>(args)...} {}
  };

  ListImpl inner;

  void drop() {
    auto *ptr = inner.first->next;
    while (ptr != inner.last) {
      auto *next = ptr->next;
      delete reinterpret_cast<ListNode *>(ptr);
      ptr = next;
    }
  }

  void emplace(List &&other) {
    inner = std::move(other.inner);
  }

public:
  List() : inner{} {}

  Iter begin() { return Iter{inner.first->next}; }

  Iter end() { return Iter{inner.last}; }

  template<class ...Args>
  Iter emplace_back(Args &&...args) {
    auto new_node = new ListNode{std::forward<Args>(args)...};
    inner.insert_back(new_node);
    return Iter{new_node};
  }

  Iter give_up_node(Iter node, List &other) {
    inner.give_up_node(node.ptr, other.inner);
    return node;
  }

  void give_up_nodes(List &other) {
    inner.give_up_nodes(other.inner);
  }

  List(const List &other) = delete;

  List(List &&other) { emplace(std::move(other)); }

  List &operator=(const List &other) = delete;

  List &operator=(List &&other) {
    drop();
    emplace(std::move(other));
    return *this;
  }

  ~List() { drop(); }
};


template<class T>
class ListIter {
private:
  friend class List<T>;
  friend class std::hash<ListIter>;

  ListImpl::ListNode *ptr;

public:
  ListIter(ListImpl::ListNode *ptr) : ptr{ptr} {}

  const T &operator*() const {
    return reinterpret_cast<const typename List<T>::ListNode *>(ptr)->elem;
  }

  T &operator*() {
    return reinterpret_cast<typename List<T>::ListNode *>(ptr)->elem;
  }

  const T *operator->() const {
    return &reinterpret_cast<const typename List<T>::ListNode *>(ptr)->elem;
  }

  T *operator->() {
    return &reinterpret_cast<typename List<T>::ListNode *>(ptr)->elem;
  }

  bool operator==(const ListIter &other) const { return ptr == other.ptr; }

  bool operator!=(const ListIter &other) const { return ptr != other.ptr; }

  ListIter &operator++() {
    ptr = ptr->next;
    return *this;
  }

  ListIter operator++(int) {
    ListIter ret = *this;
    ptr = ptr->next;
    return ret;
  }

  ListIter &operator--() {
    ptr = ptr->prev;
    return *this;
  }

  ListIter operator--(int) {
    ListIter ret = *this;
    ptr = ptr->prev;
    return ret;
  }

  void delete_node() {
    ptr->prev->next = ptr->next;
    ptr->next->prev = ptr->prev;
    delete reinterpret_cast<typename List<T>::ListNode *>(ptr);
  }
};


namespace std {
  template<class T>
  struct hash<ListIter<T>> {
    size_t operator()(const ListIter<T> &other) const {
      return hash<ListImpl::ListNode *>{}(other.ptr);
    }
  };
}


#endif // REGEX_LIST
