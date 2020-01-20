#include "test_runner.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <utility>
#include <vector>

using Id = std::size_t;

template <typename T>
class PriorityCollection
{
public:
    PriorityCollection() = default;
    ~PriorityCollection() = default;

    using Id = std::size_t;

    Id Add(T object);

    template <typename ObjInputIt, typename IdOutputIt>
    void Add(ObjInputIt range_begin, ObjInputIt range_end, IdOutputIt ids_begin);

    bool IsValid(Id id) const;

    const T& Get(Id id) const;

    void Promote(Id id);

    std::pair<const T&, int> GetMax() const;

    std::pair<T, int> PopMax();
    
private:
    struct Object
    {
        T object;
        int priority;
    };

    std::vector<Object> objects;
    std::set<std::pair<int, Id>> priority_order;
};

template <typename T>
Id PriorityCollection<T>::Add(T object)
{
    Id id = objects.size();
    objects.push_back({ std::move(object), 0 });
    priority_order.insert({ 0, id });
    return id;
}

template <typename T>
template <typename ObjInputIt, typename IdOutputIt>
void PriorityCollection<T>::Add(ObjInputIt range_begin, ObjInputIt range_end, IdOutputIt ids_begin)
{
    while (range_begin != range_end)
    {
        *(ids_begin++) = this->Add(std::move(*(range_begin++)));
    }
}

template <typename T>
bool PriorityCollection<T>::IsValid(Id id) const
{
    return ((id < objects.size()) && (objects[id].priority != -1));
}

template <typename T>
const T& PriorityCollection<T>::Get(Id id) const
{
    return objects[id].object;
}

template <typename T>
void  PriorityCollection<T>::Promote(Id id)
{
    Object& obj = objects[id];
    priority_order.erase({ obj.priority++, id });
    priority_order.insert({ obj.priority, id });
}

template <typename T>
std::pair<const T&, int> PriorityCollection<T>::GetMax() const
{
    std::set<std::pair<int, Id>>::const_iterator it = std::prev(priority_order.cend());

    return { objects[it->second].object, it->first };
}

template <typename T>
std::pair<T, int> PriorityCollection<T>::PopMax()
{
    std::set<std::pair<int, Id>>::iterator it = std::prev(priority_order.end());
    Object& pr = objects[it->second];

    priority_order.erase(it);
    const int priority = pr.priority;
    pr.priority = -1;
    return { std::move(pr.object), priority };
}







// testing

class StringNonCopyable : public std::string 
{
public:
    using std::string::string;
    StringNonCopyable(const StringNonCopyable&) = delete;
    StringNonCopyable(StringNonCopyable&&) = default;
    StringNonCopyable& operator=(const StringNonCopyable&) = delete;
    StringNonCopyable& operator=(StringNonCopyable&&) = default;
};

void TestNoCopy() 
{
    PriorityCollection<StringNonCopyable> strings;
    const auto white_id = strings.Add("white");
    const auto yellow_id = strings.Add("yellow");
    const auto red_id = strings.Add("red");

    strings.Promote(yellow_id);

    for (int i = 0; i < 2; ++i) 
    {
        strings.Promote(red_id);
    }

    strings.Promote(yellow_id);

    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "red");
        ASSERT_EQUAL(item.second, 2);
    }

    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "yellow");
        ASSERT_EQUAL(item.second, 2);
    }

    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "white");
        ASSERT_EQUAL(item.second, 0);
    }
}

int main() 
{
    TestRunner tr;
    RUN_TEST(tr, TestNoCopy);

    return 0;
}