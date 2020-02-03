#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <queue>
#include <stdexcept>
#include <set>
using namespace std;

template <typename T>
class ObjectPool
{
public:
    T* Allocate();
    T* TryAllocate();

    void Deallocate(T* object);

    ~ObjectPool();

private:
    std::set<T*> in_use_;
    std::queue<T*> freed_;
};

template <typename T>
T* ObjectPool<T>::Allocate()
{
    T* tmp;
    if (freed_.empty())
    {
        tmp = new T;
        in_use_.insert(tmp);
    }
    else
    {
        tmp = freed_.front();
        freed_.pop();
        in_use_.insert(tmp);
    }

    return tmp;
}

template <typename T>
T* ObjectPool<T>::TryAllocate()
{
    if (freed_.empty())
    {
        return nullptr;
    }

    T* tmp = freed_.front();
    freed_.pop();
    in_use_.insert(tmp);

    return tmp;
}

template <typename T>
void ObjectPool<T>::Deallocate(T* object)
{
    auto it = in_use_.find(object);

    if (it == in_use_.end())
    {
        throw std::invalid_argument("no such pointer");
    }

    freed_.push(*it);
    in_use_.erase(it);
}

template <typename T>
ObjectPool<T>::~ObjectPool()
{
    for (T* i : in_use_)
    {
        delete i;
    }

    while (!freed_.empty())
    {
        delete freed_.front();
        freed_.pop();
    }
}

void TestObjectPool()
{
    ObjectPool<string> pool;

    auto p1 = pool.Allocate();
    auto p2 = pool.Allocate();
    auto p3 = pool.Allocate();

    *p1 = "first";
    *p2 = "second";
    *p3 = "third";

    pool.Deallocate(p2);
    ASSERT_EQUAL(*pool.Allocate(), "second");

    pool.Deallocate(p3);
    pool.Deallocate(p1);
    ASSERT_EQUAL(*pool.Allocate(), "third");
    ASSERT_EQUAL(*pool.Allocate(), "first");

    pool.Deallocate(p1);
}

int main() 
{
    TestRunner tr;
    RUN_TEST(tr, TestObjectPool);

    return 0;
}