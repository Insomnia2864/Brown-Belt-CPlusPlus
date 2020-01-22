#include "test_runner.h"
#include <limits>
#include <random>
#include <unordered_set>

struct Address 
{
    Address() = default;

    Address(const std::string& ct, const std::string& strt, int bld) :
        city(ct),
        street(strt),
        building(bld)
    {}

    Address(const Address& address) :
        city(address.city),
        street(address.street),
        building(address.building)
    {}

    std::string city, street;
    int building;

    bool operator==(const Address& other) const 
    {
        return other.building == building && other.city == city && other.street == street;
    }
};

struct Person
{
    Person() = default;

    Person(const std::string& nm, int hght, double wght, const Address& adr) :
        name(nm),
        height(hght),
        weight(wght),
        address(adr)
    {}

    std::string name;
    int height;
    double weight;
    Address address;

    bool operator==(const Person& other) const 
    {
        return name == other.name && height == other.height && weight == other.weight && address == other.address;
    }
};

struct AddressHasher 
{
    AddressHasher() = default;

    const std::size_t cnst = 9'369'319;
    const std::hash<std::string> hash_string = std::hash<std::string>();
    const std::hash<int> hash_int = std::hash<int>();

    std::size_t operator()(const Address& address) const
    {
        return cnst * cnst * hash_string(address.city) + cnst * hash_string(address.street) + hash_int(address.building);
    }
};

struct PersonHasher 
{
    PersonHasher() = default;

    const std::size_t cnst = 9'369'319;
    const std::hash<std::string> hash_string = std::hash<std::string>();
    const std::hash<int> hash_int = std::hash<int>();
    const std::hash<double> hash_double = std::hash<double>();
    const AddressHasher hash_address = AddressHasher();

    std::size_t operator()(const Person& person) const
    {
        return cnst * cnst * cnst * hash_string(person.name) + cnst * cnst * hash_int(person.height) + cnst * hash_double(person.weight) + hash_address(person.address);
    }
};



//testing

const std::vector<std::string> WORDS =
{
  "Kieran", "Jong", "Jisheng", "Vickie", "Adam", "Simon", "Lance",
  "Everett", "Bryan", "Timothy", "Daren", "Emmett", "Edwin", "List",
  "Sharon", "Trying", "Dan", "Saad", "Kamiya", "Nikolai", "Del",
  "Casper", "Arthur", "Mac", "Rajesh", "Belinda", "Robin", "Lenora",
  "Carisa", "Penny", "Sabrina", "Ofer", "Suzanne", "Pria", "Magnus",
  "Ralph", "Cathrin", "Phill", "Alex", "Reinhard", "Marsh", "Tandy",
  "Mongo", "Matthieu", "Sundaresan", "Piotr", "Ramneek", "Lynne", "Erwin",
  "Edgar", "Srikanth", "Kimberly", "Jingbai", "Lui", "Jussi", "Wilmer",
  "Stuart", "Grant", "Hotta", "Stan", "Samir", "Ramadoss", "Narendra",
  "Gill", "Jeff", "Raul", "Ken", "Rahul", "Max", "Agatha",
  "Elizabeth", "Tai", "Ellen", "Matt", "Ian", "Toerless", "Naomi",
  "Rodent", "Terrance", "Ethan", "Florian", "Rik", "Stanislaw", "Mott",
  "Charlie", "Marguerite", "Hitoshi", "Panacea", "Dieter", "Randell", "Earle",
  "Rajiv", "Ted", "Mann", "Bobbie", "Pat", "Olivier", "Harmon",
  "Raman", "Justin"
};

void TestSmoke() 
{
    std::vector<Person> points = 
    {
      {"John", 180, 82.5, {"London", "Baker St", 221}},
      {"Sherlock", 190, 75.3, {"London", "Baker St", 221}},
    };

    std::unordered_set<Person, PersonHasher> point_set;
    for (const auto& point : points) {
        point_set.insert(point);
    }

    ASSERT_EQUAL(points.size(), point_set.size());
    for (const auto& point : points) {
        ASSERT_EQUAL(point_set.count(point), static_cast<size_t>(1));
    }
}

void TestPurity() 
{
    Person person = { "John", 180, 82.5, {"London", "Baker St", 221} };
    PersonHasher hasher;

    auto hash = hasher(person);
    for (size_t t = 0; t < 100; ++t)
    {
        ASSERT_EQUAL(hasher(person), hash);
    }
};

void TestDistribution() 
{
    auto seed = 42;
    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> height_dist(150, 200);
    std::uniform_int_distribution<int> weight_dist(100, 240);  // [50, 120]
    std::uniform_int_distribution<int> building_dist(1, 300);
    std::uniform_int_distribution<int> word_dist(0, WORDS.size() - 1);

    PersonHasher hasher;

    const std::size_t num_buckets = 2053;

    const std::size_t perfect_bucket_size = 50;
    const std::size_t num_points = num_buckets * perfect_bucket_size;
    std::vector<std::size_t> buckets(num_buckets);
    for (std::size_t t = 0; t < num_points; ++t)
    {
        Person person;
        person.name = WORDS[word_dist(gen)];
        person.height = height_dist(gen);
        person.weight = weight_dist(gen) * 0.5;
        person.address.city = WORDS[word_dist(gen)];
        person.address.street = WORDS[word_dist(gen)];
        person.address.building = building_dist(gen);
        ++buckets[hasher(person) % num_buckets];
    }

    double pearson_stat = 0;
    for (auto bucket_size : buckets) 
    {
        std::size_t size_diff = bucket_size - perfect_bucket_size;
        pearson_stat +=
            size_diff * size_diff / static_cast<double>(perfect_bucket_size);
    }

    const double critical_value = 2158.4981036918693;
    ASSERT(pearson_stat < critical_value);
}

int main() 
{
    TestRunner tr;
    RUN_TEST(tr, TestSmoke);
    RUN_TEST(tr, TestPurity);
    RUN_TEST(tr, TestDistribution);

    return 0;
}