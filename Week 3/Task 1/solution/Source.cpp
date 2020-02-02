#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <optional>

using namespace std;

template <typename Iterator>
class IteratorRange 
{
public:
    IteratorRange() = default;

    IteratorRange(Iterator begin, Iterator end): 
        first(begin),
        last(end)
    {}

    Iterator begin() const 
    {
        return first;
    }

    Iterator end() const 
    { 
        return last;
    }

    void setFirst(Iterator it)
    {
        first = it;
    }

    void setLast(Iterator it)
    {
        last = it;
    }

private:
    Iterator first, last;
};

template <typename Collection>
auto Head(Collection& v, size_t top) 
{
    return IteratorRange{ v.begin(), next(v.begin(), min(top, v.size())) };
}

struct Person 
{
    string name;
    int age, income;
    bool is_male;
};

vector<Person> ReadPeople(istream& input) 
{
    int count;
    input >> count;

    vector<Person> result(count);
    for (Person& p : result) 
    {
        char gender;
        input >> p.name >> p.age >> p.income >> gender;
        p.is_male = gender == 'M';
    }

    return result;
}

class Storage
{
public:
    Storage() = delete;
    Storage(const Storage&) = delete;
    Storage(const vector<Person>&) = delete;

    Storage(vector<Person>&& vec);

    size_t NumberOfPeople(size_t adultAge) const;
    unsigned TotalIncome(size_t amount) const;
    optional<string> MostPopularName(char sex);

private:
    vector<Person> sorted_by_age;
    vector<Person> sorted_by_income;
    vector<Person> sorted_by_name;
    IteratorRange<vector<Person>::iterator> men, women;
    optional<string> most_popular_male_name, most_popular_female_name;
};

Storage::Storage(vector<Person>&& vec) :
    sorted_by_age(move(vec))
{
    sorted_by_income = sorted_by_age;
    sorted_by_name = sorted_by_age;


    sort(sorted_by_age.begin(), sorted_by_age.end(), [](const Person& l, const Person& r)
        {
            return l.age < r.age;
        });

    sort(sorted_by_income.begin(), sorted_by_income.end(), [](const Person& l, const Person& r)
        {
            return l.income > r.income;
        });

    sort(sorted_by_name.begin(), sorted_by_name.end(), [](const Person& l, const Person& r)
        {
            return make_pair(l.is_male, l.name) < make_pair(r.is_male, r.name);
        });

    auto it = find_if(sorted_by_name.begin(), sorted_by_name.end(), [](const Person& person)
        {
            return person.is_male;
        });
    
    men.setFirst(it);
    men.setLast(sorted_by_name.end());

    women.setFirst(sorted_by_name.begin());
    women.setLast(it);
}

size_t Storage::NumberOfPeople(size_t adultAge) const
{
    auto it = find_if(sorted_by_age.begin(), sorted_by_age.end(), [adultAge](const Person& person)
        {
            return person.age >= adultAge;
        });

    return static_cast<size_t>(distance(it, sorted_by_age.end()));
}

unsigned Storage::TotalIncome(size_t amount) const
{
    auto head = Head(sorted_by_income, amount);
    return accumulate(head.begin(), head.end(), 0u, [](int sum, const Person& person)
        {
            return sum += person.income;
        });
}

optional<string> Storage::MostPopularName(char sex)
{
    size_t max_ = 0;
    string result;

    if (sex == 'M')
    {
        if (most_popular_male_name.has_value())
        {
            return most_popular_male_name.value();
        }

        if (men.begin() == men.end())
        {
            return optional<string>();
        }

        for (auto it = men.begin(); it != men.end();)
        {
            string_view tmp(it->name);
            auto it2 = find_if(it, men.end(), [tmp](const Person& person)
                {
                    return string_view(person.name) != tmp;
                });
            
            size_t current = static_cast<size_t>(distance(it, it2));

            if (current > max_)
            {
                max_ = current;
                result = it->name;
            }
            it = it2;
        }
        most_popular_male_name = optional<string>(result);
        return most_popular_female_name;
    }
    else
    {
        if (most_popular_female_name.has_value())
        {
            return most_popular_female_name;
        }

        if (women.begin() == women.end())
        {
            return optional<string>();
        }

        for (auto it = women.begin(); it != women.end();)
        {
            string_view tmp(it->name);
            auto it2 = find_if(it, women.end(), [tmp](const Person& person)
                {
                    return string_view(person.name) != tmp;
                });

            size_t current = static_cast<size_t>(distance(it, it2));

            if (current > max_)
            {
                max_ = current;
                result = it->name;
            }
            it = it2;
        }

        most_popular_female_name = optional<string>(result);
        return most_popular_female_name;
    }
}

int main() 
{
    vector<Person> people = ReadPeople(cin);

    Storage storage(move(people));

    for (string command; cin >> command; ) 
    {
        if (command == "AGE") 
        {
            int adult_age;
            cin >> adult_age;

            cout << "There are " << storage.NumberOfPeople(adult_age)
                << " adult people for maturity age " << adult_age << '\n';
        }
        else if (command == "WEALTHY") 
        {
            int count;
            cin >> count;

            cout << "Top-" << count << " people have total income " << storage.TotalIncome(count) << '\n';
        }
        else if (command == "POPULAR_NAME") {
            char sex;
            cin >> sex;

            auto opt = storage.MostPopularName(sex);

            if (opt.has_value())
            {
                cout << "Most popular name among people of gender " << sex << " is " << opt.value() << '\n';

            }
            else
            {
                cout << "No people of gender " << sex << '\n';
            }
        
        }
    }
}