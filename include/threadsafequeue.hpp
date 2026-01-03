#pragma once
#include <memory>
#include <mutex>

/* Implementation of threadsafe queue taken from: 
   C++ Concurrency In Action 2nd edition by Anthony Williams 
   Listings 6.7,6.8,6.9,6.10
*/

template<typename T>
class threadsafe_queue
{
private:
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node* tail;
    std::condition_variable data_cond;

    node* get_tail(void);
    std::unique_ptr<node> pop_head(void);
    std::unique_lock<std::mutex> wait_for_data(void);
    std::unique_ptr<node> wait_pop_head(void);
    std::unique_ptr<node> wait_pop_head(T& value);
    std::unique_ptr<node> try_pop_head(void);
    std::unique_ptr<node> try_pop_head(T& value);

public:
    threadsafe_queue() :
    head(new node),
    tail(head.get())
    {}

    threadsafe_queue(const threadsafe_queue& other) = delete;
    threadsafe_queue& operator=(const threadsafe_queue& other) = delete;
    
    std::shared_ptr<T> try_pop(void);
    bool try_pop(T& value);

    std::shared_ptr<T> wait_and_pop(void);
    void wait_and_pop(T& value);

    void push(T new_value);
    bool empty(void);
};