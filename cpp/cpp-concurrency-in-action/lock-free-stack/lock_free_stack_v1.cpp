// Example 7.10-7.11 from "C++ concurrency in action" by Anthony Williams (2012)
//
// for platforms where sizeof(counted_node_ptr) allows it to fit in non-blocking atomic variable,
// i.e. std::atomic_is_lock_free( &counted_node_ptr ) == true
// (otherwise atomic operations internally use mutexes and are not lock-free, so it's easier to use
//  lock_free_stack_v0 based on (also mutex-based) std::atomic<std::shared_ptr<node>>              )
#include <memory>
#include <atomic>

template <typename T>
class lock_free_stack_v1
{
private:
  struct node;

  struct counted_node_ptr   // external counter is stored together with the ptr to the data node
  {
    int external_count;
    node* ptr;
  };

  struct node
  {
    std::shared_ptr<T> data;
    std::atomic<int>   internal_count;  // atomic internal_count
    counted_node_ptr   next;            // regular counted_node_ptr used to make the signle-linked list

    node(T const& data_): data( std::make_shared<T>(data_) ), internal_count(0) { }
  };

  std::atomic<counted_node_ptr> head;  // stack head is an atomic counted_node_ptr (NB: should fit in non-blocking atomics)

  void increase_head_count(counted_node_ptr& old_counter)
  {
    counted_node_ptr new_counter;
    do {
      new_counter = old_counter;
      ++ new_counter.external_count;
    }
    while( !head.compare_exchange_strong(old_counter, new_counter) );  // atomically update external count in the head

    old_counter.external_count = new_counter.external_count;
  }

public:
  ~lock_free_stack_v1()
  {
    while( pop() );
  }

  void push(T const& data)
  {
    counted_node_ptr new_node;

    new_node.ptr            = new node( data );
    new_node.external_count = 1;   // +1 for being inserted in the list/referred to by the list head
    new_node.ptr->next      = head.load();

    while( !head.compare_exchange_weak(new_node.ptr->next, new_node) );
  }

  std::shared_ptr<T> pop()
  {
    counted_node_ptr old_head = head.load();
    for ( ; ; )
    {
      increase_head_count( old_head );

      node* const ptr = old_head.ptr;
      if ( !ptr )
        return std::shared_ptr<T>();


      if ( head.compare_exchange_strong(old_head, ptr->next) ) // atomically try to take posession of the head
      {
        std::shared_ptr<T> res;
        res.swap( ptr->data );

        // check if node can be safely deleted: release the node from ourselves and remove from the the head of the list => -2
        // + external_count value which we saw when we loaded the old_head from head the last time
        // (at the beginning of the loop, while increasing head count, or when taking posessin of the head counted_node_ptr)
        int const count_increase = old_head.external_count - 2;
        if ( ptr->internal_count.fetch_add(count_increase) == -count_increase )  // can delete
          delete ptr;

        return res;
      }
      else if ( ptr->internal_count.fetch_sub(1) == 1 ){ // taking posession failed - release atomic internal_count from ourselves
        delete ptr;
      }
    }  // while( true )
  }
};
