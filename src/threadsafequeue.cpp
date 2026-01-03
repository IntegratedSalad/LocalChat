#include "threadsafequeue.hpp"

template<typename T>
void threadsafe_queue<T>::push(T new_value)
{
    std::shared_ptr<T> new_data(
        std::make_shared<T>(std::move(new_value)));
    std::unique_ptr<node> p(new node);
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
    data_cond.notify_one();
}

template<typename T>
typename threadsafe_queue<T>::node* threadsafe_queue<T>::get_tail(void)
{
    std::lock_guard<std::mutex> tail_lock(tail_mutex);
    return tail;
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::pop_head(void)
{
    std::unique_ptr<node> old_head = std::move(head);
    head = std::move(old_head->next);
    return old_head;
}

template<typename T>
std::unique_lock<std::mutex> threadsafe_queue<T>::wait_for_data(void)
{
    std::unique_lock<std::mutex> head_lock(head_mutex);
    data_cond.wait(head_lock, [&]{return head.get() != get_tail();});
    return std::move(head_lock);
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::wait_pop_head(void)
{
    std::unique_lock<std::mutex> head_lock(wait_for_data());
    return pop_head();
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::wait_pop_head(T& value)
{
    std::unique_lock<std::mutex> head_lock(wait_for_data());
    value = std::move(*head->data);
    return pop_head();
}

template<typename T>
std::shared_ptr<T> threadsafe_queue<T>::wait_and_pop(void)
{
    std::unique_ptr<node> const old_head = wait_pop_head();
    return old_head->data;
}

template<typename T>
void threadsafe_queue<T>::wait_and_pop(T& value)
{
    std::unique_ptr<node> const old_head = wait_pop_head(value);
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::try_pop_head(void)
{
    std::lock_guard<std::mutex> head_lock(head_mutex);
    if (head.get() == get_tail())
    {
        return std::unique_ptr<node>();
    }
    return pop_head();
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::try_pop_head(T& value)
{
    std::lock_guard<std::mutex> head_lock(head_mutex);
    if (head.get() == get_tail())
    {
        return std::unique_ptr<node>();
    }
    value = std::move(*head->data);
    return pop_head();
}
    
template<typename T>
std::shared_ptr<T> threadsafe_queue<T>::try_pop(void)
{
    std::unique_ptr<node> old_head = try_pop_head();
    return old_head ? old_head->data : std::shared_ptr<T>();
}

template<typename T>
bool threadsafe_queue<T>::try_pop(T& value)
{
    std::unique_ptr<node> const old_head = try_pop_head(value);
    return old_head;
}

template<typename T>
bool threadsafe_queue<T>::empty(void)
{
    std::lock_guard<std::mutex> head_lock(head_mutex);
    return (head.get() == get_tail());
}