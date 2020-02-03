#include "test_runner.h"

#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;


struct Email 
{
    string from;
    string to;
    string body;
};

istream& operator>>(istream& in, Email& email)
{
    getline(in, email.from);
    getline(in, email.to);
    getline(in, email.body);

    return in;
}

class Worker 
{
public:
    virtual ~Worker() = default;
    virtual void Process(unique_ptr<Email> email) = 0;
    virtual void Run() 
    {
        throw logic_error("Unimplemented");
    }

protected:
    void PassOn(unique_ptr<Email> email) const
    {
        if (next_.get() != nullptr)
        {
            next_->Process(move(email));
        }
    }

    unique_ptr<Worker> next_;

public:
    void SetNext(unique_ptr<Worker> next)
    {
        if (next_ != nullptr)
        {
            next_->SetNext(move(next));
        }
        else
        {
            next_ = (move(next));
        }
    }
};


class Reader : public Worker 
{
public:
    explicit Reader(istream& in)
    {
        Email tmp;
        while (in >> tmp)
        {
            emails.push_back(make_unique<Email>(tmp));
        }
    }

    void Process(unique_ptr<Email> email) override
    {
        PassOn(move(email));
    }

    void Run() override
    {
        for (auto& i : emails)
        {
            Process(move(i));
        }
    }

private:
    vector<unique_ptr<Email>> emails;
};


class Filter : public Worker 
{
public:
    using Function = function<bool(const Email&)>;

public:
    explicit Filter(Function func) :
        function_(func)
    {}

    void Process(unique_ptr<Email> email) override
    {
        if (function_(*email.get()))
        {
            PassOn(move(email));
        }
    }

private:
    Function function_;
};


class Copier : public Worker 
{
public:
    explicit Copier(const string& str) :
        recipient(str)
    {}

    void Process(unique_ptr<Email> email) override
    {
        string to_ = email->to;
        auto email_copy = make_unique<Email>();
        email_copy->from = email->from;
        email_copy->to = recipient;
        email_copy->body = email->body;
        PassOn(move(email));

        if (recipient != to_)
        {
            PassOn(move(email_copy));
        }
    }

private:
    string recipient;
};


class Sender : public Worker 
{
public:
    explicit Sender(ostream& out) :
        out_(out)
    {}

    void Process(unique_ptr<Email> email) override
    {
        out_ << email->from << '\n' << email->to << '\n' << email->body << '\n';
        PassOn(move(email));
    }

private:
    ostream& out_;
};


class PipelineBuilder 
{
public:
    explicit PipelineBuilder(istream& in) :
        in_(in)
    {
        start_ = make_unique<Reader>(in_);
    }

    PipelineBuilder& FilterBy(Filter::Function filter)
    {
        start_->SetNext(make_unique<Filter>(filter));
        return *this;
    }

    PipelineBuilder& CopyTo(string recipient)
    {
        start_->SetNext(make_unique<Copier>(recipient));
        return *this;
    }

    PipelineBuilder& Send(ostream& out)
    {
        start_->SetNext(make_unique<Sender>(out));
        return *this;
    }

    unique_ptr<Worker> Build()
    {
        return move(start_);
    }

private:
    istream& in_;
    unique_ptr<Worker> start_;
};



//testing


void TestSanity() 
{
    string input = 
        (
        "erich@example.com\n"
        "richard@example.com\n"
        "Hello there\n"

        "erich@example.com\n"
        "ralph@example.com\n"
        "Are you sure you pressed the right button?\n"

        "ralph@example.com\n"
        "erich@example.com\n"
        "I do not make mistakes of that kind\n"
        );
    istringstream inStream(input);
    ostringstream outStream;

    PipelineBuilder builder(inStream);
    builder.FilterBy([](const Email& email) 
        {
        return email.from == "erich@example.com";
        });
    builder.CopyTo("richard@example.com");
    builder.Send(outStream);
    auto pipeline = builder.Build();

    pipeline->Run();

    string expectedOutput = 
        (
        "erich@example.com\n"
        "richard@example.com\n"
        "Hello there\n"

        "erich@example.com\n"
        "ralph@example.com\n"
        "Are you sure you pressed the right button?\n"

        "erich@example.com\n"
        "richard@example.com\n"
        "Are you sure you pressed the right button?\n"
        );

    ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() 
{
    TestRunner tr;
    RUN_TEST(tr, TestSanity);
    
    return 0;
}