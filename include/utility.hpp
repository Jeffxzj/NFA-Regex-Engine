#ifndef REGEX_UTILITY
#define REGEX_UTILITY


#define CASE_NUMERIC \
  '0':      case '1': case '2': case '3': case '4': \
  case '5': case '6': case '7': case '8': case '9'

#define CASE_LOWER_CASE \
  'a':      case 'b': case 'c': case 'd': case 'e': \
  case 'f': case 'g': case 'h': case 'i': case 'j': \
  case 'k': case 'l': case 'm': case 'n': case 'o': \
  case 'p': case 'q': case 'r': case 's': case 't': \
  case 'u': case 'v': case 'w': case 'x': case 'y': \
  case 'z'

#define CASE_UPPER_CASE \
  'A':      case 'B': case 'C': case 'D': case 'E': \
  case 'F': case 'G': case 'H': case 'I': case 'J': \
  case 'K': case 'L': case 'M': case 'N': case 'O': \
  case 'P': case 'Q': case 'R': case 'S': case 'T': \
  case 'U': case 'V': case 'W': case 'X': case 'Y': \
  case 'Z'


struct CharacterRange {
  char lower_bound;
  char upper_bound;
};

struct RepeatRange {
  size_t lower_bound; // >=1
  size_t upper_bound; // if 0 means no upperbound
};


class ListImpl {
public:
  struct ListNode {
    ListNode *prev;
    ListNode *next;
  };

  ListNode * first;
  ListNode * last;

  void drop() {
    delete first;
    delete last;
  }

  void emplace(ListImpl &&other) {
    first = other.first;
    last = other.last;
    other.first = nullptr;
    other.last = nullptr;
  }

  ListImpl() : first{new ListNode{}}, last{new ListNode{}} {
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
    first->next = nullptr;
    last->prev = nullptr;
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
class List {
private:
  struct ListNode : ListImpl::ListNode {
    T elem;

    template<class ...Args>
    ListNode(Args &&...args) :
        ListImpl::ListNode{}, elem{std::forward(args)...} {}
  };

  ListImpl inner;

  void drop() {
    if (inner.first == nullptr) { return; }

    auto *ptr = inner.first->next;
    while (ptr != inner.last) {
      auto *next = ptr->next;
      delete reinterpret_cast<ListNode *>(ptr);
      ptr = next;
    }
    inner.first->next = nullptr;
    inner.last->prev = nullptr;
  }

  void emplace(List &&other) {
    inner = std::move(other.inner);
  }

public:
  class Iter {
  private:
    friend class List;

    ListNode *ptr;

  public:
    Iter(ListNode *ptr) : ptr{ptr} {}

    T &operator*() { return ptr->elem; }

    T *operator->() { return &ptr->elem; }
  };

  List() : inner{} {}

  Iter begin() { return Iter{inner.first->next}; }

  Iter end() { return Iter{inner.last}; }

  template<class ...Args>
  Iter emplace_back(Args &&...args) {
    ListNode new_node{std::forward(args)...};
    inner.insert_back(&new_node);
    return Iter{&new_node};
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


#endif // REGEX_UTILITY
