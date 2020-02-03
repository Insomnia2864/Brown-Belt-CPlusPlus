#include "Common.h"
#include "test_runner.h"

#include <sstream>
#include <memory>

using namespace std;

class ValueExpression : public Expression
{
public:
    ValueExpression(int value) :
        value_(value)
    {}

    int Evaluate() const override
    {
        return value_;
    }

    std::string ToString() const override
    {
        return std::to_string(value_);
    }

private:
    int value_;
};

class ComplexExpression : public Expression
{
public:

    using ExpressionPtr = std::unique_ptr<Expression>;

    ComplexExpression(ExpressionPtr left, ExpressionPtr right, char operation) :
        left_(move(left)),
        right_(move(right)),
        operation_(operation)
    {}

    int Evaluate() const override
    {
        if (operation_ == '*')
        {
            return left_->Evaluate() * right_->Evaluate();
        }
        else if (operation_ == '/')
        {
            return left_->Evaluate() / right_->Evaluate();
        }
        else if (operation_ == '+')
        {
            return left_->Evaluate() + right_->Evaluate();
        }
        else if (operation_ == '-')
        {
            return left_->Evaluate() - right_->Evaluate();
        }

        throw std::logic_error("unknown operation");
    }

    std::string ToString() const override
    {
        return "(" + left_->ToString() + ")" + operation_ + "(" + right_->ToString() + ")";
    }


private:
    ExpressionPtr left_, right_;
    char operation_;
};

ExpressionPtr Value(int value)
{
    return make_unique<ValueExpression>(value);
}

ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right)
{
    return make_unique<ComplexExpression>(move(left), move(right), '+');
}

ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right)
{
    return make_unique<ComplexExpression>(move(left), move(right), '*');
}

string Print(const Expression* e) 
{
    if (!e) 
    {
        return "Null expression provided";
    }
    stringstream output;
    output << e->ToString() << " = " << e->Evaluate();
    return output.str();
}

void Test() 
{
    ExpressionPtr e1 = Product(Value(2), Sum(Value(3), Value(4)));
    ASSERT_EQUAL(Print(e1.get()), "(2)*((3)+(4)) = 14");

    ExpressionPtr e2 = Sum(move(e1), Value(5));
    ASSERT_EQUAL(Print(e2.get()), "((2)*((3)+(4)))+(5) = 19");

    ASSERT_EQUAL(Print(e1.get()), "Null expression provided");
}

int main() 
{
    TestRunner tr;
    RUN_TEST(tr, Test);
    return 0;
}