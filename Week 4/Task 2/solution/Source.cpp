#include "Common.h"

#include <unordered_set>
#include <map>
#include <stdexcept>
#include <mutex>

using namespace std;

class LruCache : public ICache 
{
public:
    LruCache(shared_ptr<IBooksUnpacker> books_unpacker, const Settings& settings) :
        unpacker(move(books_unpacker)),
        max_memory(settings.max_memory),
        memory_taken(0u),
        biggest_index(0u)
    {}

    BookPtr GetBook(const string& book_name) override 
    {
        lock_guard<mutex> lg(m_);
        if (books_in_cache.count(book_name) == 0)
        {
            unique_ptr<IBook> unique_book = unpacker->UnpackBook(book_name);
            if (unique_book.get() == nullptr)
            {
                return shared_ptr<IBook>();
            }

            shared_ptr<IBook> book(unique_book.get());
            unique_book.release();
            PutABookInCache(book);
            return book;
        }
        else
        {
            for (auto it = cache_itself.begin(); it != cache_itself.end(); it++)
            {
                if (it->second->GetName() == book_name)
                {
                    if (it->first == biggest_index)
                    {
                        return it->second;
                    }
                    auto node = cache_itself.extract(it);
                    node.key() = biggest_index + 1;
                    biggest_index = node.key();
                    cache_itself.insert(move(node));
                    return prev(cache_itself.end())->second;
                }
            }
        }

        throw runtime_error("");
    }

private:
    shared_ptr<IBooksUnpacker> unpacker;
    size_t max_memory, memory_taken;
    size_t biggest_index;
    unordered_set<string> books_in_cache;
    map<size_t, shared_ptr<IBook>> cache_itself;
    mutex m_;

    void PutABookInCache(shared_ptr<IBook> book)
    {
        while ((max_memory < (memory_taken + book->GetContent().size())) && (!cache_itself.empty()))
        {
            auto it = cache_itself.begin();
            books_in_cache.erase(it->second->GetName());
            memory_taken -= it->second->GetContent().size();
            cache_itself.erase(it);
        }

        if (max_memory >= (memory_taken + book->GetContent().size()))
        {
            books_in_cache.insert(book->GetName());
            memory_taken += book->GetContent().size();

            if (cache_itself.empty())
            {
                cache_itself[0] = book;
                biggest_index = 0u;
            }
            else
            {
                size_t index = prev(cache_itself.end())->first + 1;
                cache_itself[index] = book;
                biggest_index = index;
            }
        }
    }
};


unique_ptr<ICache> MakeCache(shared_ptr<IBooksUnpacker> books_unpacker, const ICache::Settings& settings)
{
    return make_unique<LruCache>(books_unpacker, settings);
}