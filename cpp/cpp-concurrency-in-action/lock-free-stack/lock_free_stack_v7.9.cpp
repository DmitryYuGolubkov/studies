// implementation of lock-free stack for platforms with lock-free std::shared_ptr, where 
// std::atomic_is_lock_free(&some_shared_ptr) == true
#include <atomic>
#include <memory>

template<typename T>
class lock_free_stack
{
private:
  struct node
  {
    std::shared_ptr<T> data;
    std::shared_ptr<node> next;
    node(T const& data_) :
      data( std::make_shared<T>( data_) )
    {}
  };
  
  std::shared_ptr<node> head;

public:
  void push(T const& data)
  {
    std::shared_ptr<node> const new_node = std::make_shared<node>( data );
    new_node->next = head.load();  // if std::shared_ptr is atomic, it might have load()
    while( !std::atomic_compare_exchange_weak(&head, &new_node->next, new_node) );
  }
  
  std::shared_ptr<T> pop()
  {
    std::shared_ptr<node> old_head = std::atomic_load( &head );
    while( old_head && !std::atomic_compare_exchange_weak(&head, &old_head, old_head->next) );
    return old_head ? old_head->data : std::shared_ptr<T>();
  }
};
