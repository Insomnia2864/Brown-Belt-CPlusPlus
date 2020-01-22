#include "test_runner.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <list>

struct Record 
{
    std::string id;
    std::string title;
    std::string user;
    int timestamp;
    int karma;
};

class Database
{
public:
    bool Put(const Record& record);
    const Record* GetById(const std::string& id) const;
    bool Erase(const std::string& id);

    template <typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const;

    template <typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const;

    template <typename Callback>
    void AllByUser(const std::string& user, Callback callback) const;

private:
    struct Its
    {
        std::multimap<int, std::list<Record>::iterator>::iterator timestamp_to_record;
        std::multimap<int, std::list<Record>::iterator>::iterator karma_to_record;
        std::multimap<std::string, std::list<Record>::iterator>::iterator user_to_record;
    };

    std::list<Record> storage;
    std::unordered_map<std::string, std::list<Record>::iterator> id_to_record;
    std::multimap<int, std::list<Record>::iterator> timestamp_to_record;
    std::multimap<int, std::list<Record>::iterator> karma_to_record;
    std::multimap<std::string, std::list<Record>::iterator> user_to_record;
    std::unordered_map<std::string, Its> id_to_its;
};

bool Database::Put(const Record& record)
{
    if (id_to_record.count(record.id) == 0)
    {
        Its its;
        storage.push_back(record);
        id_to_record[record.id] = std::prev(storage.end());
        its.timestamp_to_record = timestamp_to_record.insert(std::make_pair(record.timestamp, std::prev(storage.end())));
        its.karma_to_record = karma_to_record.insert(std::make_pair(record.karma, std::prev(storage.end())));
        its.user_to_record = user_to_record.insert(std::make_pair(record.user, std::prev(storage.end())));

        id_to_its[record.id] = std::move(its);

        return true;
    }

    return false;
}

const Record* Database::GetById(const std::string& id) const
{
    if (id_to_record.count(id) == 0)
    {
        return nullptr;
    }

    return &(*id_to_record.at(id));
}

bool Database::Erase(const std::string& id)
{
    if (id_to_record.count(id) == 0)
    {
        return false;
    }

    Its& its = id_to_its[id];

    timestamp_to_record.erase(its.timestamp_to_record);
    karma_to_record.erase(its.karma_to_record);
    user_to_record.erase(its.user_to_record);
    id_to_its.erase(id);

    storage.erase(id_to_record[id]);
    id_to_record.erase(id);
    
    return true;
}

template <typename Callback>
void Database::RangeByTimestamp(int low, int high, Callback callback) const
{
    auto f1 = timestamp_to_record.lower_bound(low);
    auto f2 = timestamp_to_record.upper_bound(high);

    while (f1 != f2 && callback(*((f1++)->second)))
    {}
}

template <typename Callback>
void Database::RangeByKarma(int low, int high, Callback callback) const
{
    auto f1 = karma_to_record.lower_bound(low);
    auto f2 = karma_to_record.upper_bound(high);

    while (f1 != f2 && callback(*((f1++)->second)))
    {}
}

template <typename Callback>
void Database::AllByUser(const std::string& user, Callback callback) const
{
    auto f1 = user_to_record.lower_bound(user);
    auto f2 = user_to_record.upper_bound(user);

    while (f1 != f2 && callback(*((f1++)->second)))
    {}
}

void TestRangeBoundaries() 
{
    const int good_karma = 1000;
    const int bad_karma = -10;

    Database db;
    db.Put({ "id1", "Hello there", "master", 1536107260, good_karma });
    db.Put({ "id2", "O>>-<", "general2", 1536107260, bad_karma });

    int count = 0;
    db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) 
        {
            ++count;
            return true;
        });

    ASSERT_EQUAL(2, count);
}

void TestSameUser() 
{
    Database db;
    db.Put({ "id1", "Don't sell", "master", 1536107260, 1000 });
    db.Put({ "id2", "Rethink life", "master", 1536107260, 2000 });

    int count = 0;
    db.AllByUser("master", [&count](const Record&) 
        {
            ++count;
            return true;
        });

    ASSERT_EQUAL(2, count);
}

void TestReplacement() 
{
    const std::string final_body = "Feeling sad";

    Database db;
    db.Put({ "id", "Have a hand", "not-master", 1536107260, 10 });
    db.Erase("id");
    db.Put({ "id", final_body, "not-master", 1536107260, -10 });

    auto record = db.GetById("id");
    ASSERT(record != nullptr);
    ASSERT_EQUAL(final_body, record->title);
}

int main()  
{
    TestRunner tr;
    RUN_TEST(tr, TestRangeBoundaries);
    RUN_TEST(tr, TestSameUser);
    RUN_TEST(tr, TestReplacement);
    return 0;
}