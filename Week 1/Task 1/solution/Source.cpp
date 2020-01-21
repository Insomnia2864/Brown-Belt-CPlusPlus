#include "test_runner.h"

#include <forward_list>
#include <iterator>
#include <vector>
#include <algorithm>

using namespace std;

template <typename Type, typename Hasher>
class HashSet 
{
public:
    using BucketList = std::forward_list<Type>;

public:
    explicit HashSet(std::size_t num_buckets, const Hasher& hasher = {}) :
        buckets_count_(num_buckets),
        hasher_(hasher),
        storage_(std::vector<BucketList>(buckets_count_))
    {}

    void Add(const Type& value)
    {
        if (!this->Has(value))
        {
            storage_[hasher_(value) % buckets_count_].push_front(value);
        }
    }

    bool Has(const Type& value) const 
    {
        const BucketList& tmp = storage_[hasher_(value) % buckets_count_];
        auto it = std::find(tmp.begin(), tmp.end(), value);
        if (it == tmp.end())
        {
            return false;
        }

        return true;
    }

    void Erase(const Type& value)
    {
        BucketList& tmp = storage_[hasher_(value) % buckets_count_];
        BucketList new_bucket;

        for (const auto& i : tmp)
        {
            if (i != value)
            {
                new_bucket.push_front(std::move(i));
            }
        }

        storage_[hasher_(value) % buckets_count_] = std::move(new_bucket);
    }
    const BucketList& GetBucket(const Type& value) const
    {
        return storage_[hasher_(value) % buckets_count_];
    }
private:
    std::size_t buckets_count_;
    Hasher hasher_;
    std::vector<BucketList> storage_;
};


struct IntHasher 
{
    std::size_t operator()(const int value) const
    {
        return static_cast<std::size_t>(value);
    }
};

struct TestValue
{
    int value;

    bool operator==(TestValue other) const 
    {
        return static_cast<std::size_t>(value) / 2 == static_cast<std::size_t>(other.value) / 2;
    }
};

struct TestValueHasher 
{
    std::size_t operator()(const TestValue value) const
    {
        return static_cast<std::size_t>(value.value) / 2;
    }
};

void TestSmoke() 
{
    HashSet<int, IntHasher> hash_set(2);
    hash_set.Add(3);
    hash_set.Add(4);

    ASSERT(hash_set.Has(3));
    ASSERT(hash_set.Has(4));
    ASSERT(!hash_set.Has(5));

    hash_set.Erase(3);

    ASSERT(!hash_set.Has(3));
    ASSERT(hash_set.Has(4));
    ASSERT(!hash_set.Has(5));

    hash_set.Add(3);
    hash_set.Add(5);

    ASSERT(hash_set.Has(3));
    ASSERT(hash_set.Has(4));
    ASSERT(hash_set.Has(5));
}

void TestEmpty() 
{
    HashSet<int, IntHasher> hash_set(10);
    for (int value = 0; value < 10000; ++value) 
    {
        ASSERT(!hash_set.Has(value));
    }
}

void TestIdempotency() 
{
    HashSet<int, IntHasher> hash_set(10);
    hash_set.Add(5);
    ASSERT(hash_set.Has(5));
    hash_set.Add(5);
    ASSERT(hash_set.Has(5));
    hash_set.Erase(5);
    ASSERT(!hash_set.Has(5));
    hash_set.Erase(5);
    ASSERT(!hash_set.Has(5));
}

void TestEquivalence() 
{
    HashSet<TestValue, TestValueHasher> hash_set(10);
    hash_set.Add(TestValue{ 2 });
    hash_set.Add(TestValue{ 3 });

    ASSERT(hash_set.Has(TestValue{ 2 }));
    ASSERT(hash_set.Has(TestValue{ 3 }));

    const auto& bucket = hash_set.GetBucket(TestValue{ 2 });
    const auto& three_bucket = hash_set.GetBucket(TestValue{ 3 });
    ASSERT_EQUAL(&bucket, &three_bucket);

    ASSERT_EQUAL(1, distance(begin(bucket), end(bucket)));
    ASSERT_EQUAL(2, bucket.front().value);
}

int main() 
{
    TestRunner tr;
    RUN_TEST(tr, TestSmoke);
    RUN_TEST(tr, TestEmpty);
    RUN_TEST(tr, TestIdempotency);
    RUN_TEST(tr, TestEquivalence);
    return 0;
}