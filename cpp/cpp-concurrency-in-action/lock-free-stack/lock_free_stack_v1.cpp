// Example 7.10-7.11 from "C++ concurrency in action" by Anthony Williams (2012)
//
// for platforms where sizeof(counted_node_ptr) allows it to fit in non-blocking atomic variable,
// i.e. std::atomic_is_lock_free( &counted_node_ptr ) == true
// (otherwise atomic operations internally use mutexes and are not lock-free, so it's easier to use
//  lock_free_stack_v0 based on (also mutex-based) std::atomic<std::shared_ptr<node>>              )
#include <memory>
#include <atomic>

template <typename T>
class lock_free_stack_v1 {
private:
  struct node;

  struct counted_node_ptr
  {
    int external_count;
    node* ptr;
  };

  struct node
  {
    std::shared_ptr<T> data;
    std::atomic<int>   internal_count;
    counted_node_ptr   next;
  };

  std::atomic<counted_node_ptr> head;

public:
  ~lock_free_stack_v1()
  {
    while( pop() )
      ;
  }

  void push(T const& data)
  {
    counted_node_ptr new_node;
    new_node.ptr = new node( data );
    new_node.external_count = 1;
    new_node.ptr->next = head.load();
    while( !head.compare_exchange_weak(new_node.ptr->next, new_node) )
      ;
  }

  std::shared_ptr<T> pop()
  {
    return std::shared_ptr<T>();
  }
};
