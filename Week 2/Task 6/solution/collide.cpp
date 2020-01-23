#include "geo2d.h"
#include "game_object.h"

#include "test_runner.h"

#include <vector>
#include <memory>


class Unit : public GameObject
{
public:
    explicit Unit(geo2d::Point position) :
        position_(position)
    {}

    bool Collide(const GameObject& gameObject) const override;
        
    bool CollideWith(const Unit& unit) const override;
    bool CollideWith(const Building& building) const override;
    bool CollideWith(const Tower& tower) const override;
    bool CollideWith(const Fence& fence) const override;

    geo2d::Point GetFigure() const;

private:
    geo2d::Point position_;
};

class Building : public GameObject
{
public:
    explicit Building(geo2d::Rectangle geometry) :
        rectangle_(geometry)
    {}

    bool Collide(const GameObject& gameObject) const override;
    
    bool CollideWith(const Unit& unit) const override;
    bool CollideWith(const Building& building) const override;
    bool CollideWith(const Tower& tower) const override;
    bool CollideWith(const Fence& fence) const override;

    geo2d::Rectangle GetFigure() const;

private:
    geo2d::Rectangle rectangle_;
};

class Tower : public GameObject
{
public:
    explicit Tower(geo2d::Circle geometry) :
        circle_(geometry)
    {}

    bool Collide(const GameObject& gameObject) const override;

    bool CollideWith(const Unit& unit) const override;
    bool CollideWith(const Building& building) const override;
    bool CollideWith(const Tower& tower) const override;
    bool CollideWith(const Fence& fence) const override;

    geo2d::Circle GetFigure() const;

private:
    geo2d::Circle circle_;
};

class Fence : public GameObject
{
public:
    explicit Fence(geo2d::Segment geometry) :
        segment_(geometry)
    {}

    bool Collide(const GameObject& gameObject) const override;

    bool CollideWith(const Unit& unit) const override;
    bool CollideWith(const Building& building) const override;
    bool CollideWith(const Tower& tower) const override;
    bool CollideWith(const Fence& fence) const override;

    geo2d::Segment GetFigure() const;

private:
    geo2d::Segment segment_;
};

bool Unit::Collide(const GameObject& gameObject) const
{
    return gameObject.CollideWith(*this);
}

bool Unit::CollideWith(const Unit& unit) const
{
    return geo2d::Collide(position_, unit.GetFigure());
}
bool Unit::CollideWith(const Building& building) const
{
    return geo2d::Collide(position_, building.GetFigure());
}
bool Unit::CollideWith(const Tower& tower) const
{
    return geo2d::Collide(position_, tower.GetFigure());
}
bool Unit::CollideWith(const Fence& fence) const
{
    return geo2d::Collide(position_, fence.GetFigure());
}

geo2d::Point Unit::GetFigure() const
{
    return position_;
}


bool Building::Collide(const GameObject& gameObject) const
{
    return gameObject.CollideWith(*this);
}

bool Building::CollideWith(const Unit& unit) const 
{
    return geo2d::Collide(rectangle_, unit.GetFigure());
}
bool Building::CollideWith(const Building& building) const 
{
    return geo2d::Collide(rectangle_, building.GetFigure());
}
bool Building::CollideWith(const Tower& tower) const 
{
    return geo2d::Collide(rectangle_, tower.GetFigure());
}
bool Building::CollideWith(const Fence& fence) const 
{
    return geo2d::Collide(rectangle_, fence.GetFigure());
}

geo2d::Rectangle Building::GetFigure() const
{
    return rectangle_;
}


bool Tower::Collide(const GameObject& gameObject) const 
{
    return gameObject.CollideWith(*this);
}

bool Tower::CollideWith(const Unit& unit) const
{
    return geo2d::Collide(circle_, unit.GetFigure());
}
bool Tower::CollideWith(const Building& building) const 
{
    return geo2d::Collide(circle_, building.GetFigure());
}
bool Tower::CollideWith(const Tower& tower) const 
{
    return geo2d::Collide(circle_, tower.GetFigure());
}
bool Tower::CollideWith(const Fence& fence) const 
{
    return geo2d::Collide(circle_, fence.GetFigure());
}

geo2d::Circle Tower::GetFigure() const
{
    return circle_;
}


bool Fence::Collide(const GameObject& gameObject) const 
{
    return gameObject.CollideWith(*this);
}

bool Fence::CollideWith(const Unit& unit) const 
{
    return geo2d::Collide(segment_, unit.GetFigure());
}
bool Fence::CollideWith(const Building& building) const 
{
    return geo2d::Collide(segment_, building.GetFigure());
}
bool Fence::CollideWith(const Tower& tower) const 
{
    return geo2d::Collide(segment_, tower.GetFigure());
}
bool Fence::CollideWith(const Fence& fence) const 
{
    return geo2d::Collide(segment_, fence.GetFigure());
}

geo2d::Segment Fence::GetFigure() const
{
    return segment_;
}


bool Collide(const GameObject& first, const GameObject& second)
{
    return first.Collide(second);
}

void TestAddingNewObjectOnMap() 
{
    using namespace geo2d;

    const std::vector<std::shared_ptr<GameObject>> game_map = 
    {
      std::make_shared<Unit>(Point{3, 3}),
      std::make_shared<Unit>(Point{5, 5}),
      std::make_shared<Unit>(Point{3, 7}),
      std::make_shared<Fence>(Segment{{7, 3}, {9, 8}}),
      std::make_shared<Tower>(Circle{Point{9, 4}, 1}),
      std::make_shared<Tower>(Circle{Point{10, 7}, 1}),
      std::make_shared<Building>(Rectangle{{11, 4}, {14, 6}})
    };

    for (size_t i = 0; i < game_map.size(); ++i) 
    {
        Assert(
            Collide(*game_map[i], *game_map[i]),
            "An object doesn't collide with itself: " + std::to_string(i)
        );

        for (size_t j = 0; j < i; ++j) 
        {
            Assert(
                !Collide(*game_map[i], *game_map[j]),
                "Unexpected collision found " + std::to_string(i) + ' ' + std::to_string(j)
            );
        }
    }

    auto new_warehouse = std::make_shared<Building>(Rectangle{ {4, 3}, {9, 6} });
    ASSERT(!Collide(*new_warehouse, *game_map[0]));
    ASSERT(Collide(*new_warehouse, *game_map[1]));
    ASSERT(!Collide(*new_warehouse, *game_map[2]));
    ASSERT(Collide(*new_warehouse, *game_map[3]));
    ASSERT(Collide(*new_warehouse, *game_map[4]));
    ASSERT(!Collide(*new_warehouse, *game_map[5]));
    ASSERT(!Collide(*new_warehouse, *game_map[6]));

    auto new_defense_tower = std::make_shared<Tower>(Circle{ {8, 2}, 2 });
    ASSERT(!Collide(*new_defense_tower, *game_map[0]));
    ASSERT(!Collide(*new_defense_tower, *game_map[1]));
    ASSERT(!Collide(*new_defense_tower, *game_map[2]));
    ASSERT(Collide(*new_defense_tower, *game_map[3]));
    ASSERT(Collide(*new_defense_tower, *game_map[4]));
    ASSERT(!Collide(*new_defense_tower, *game_map[5]));
    ASSERT(!Collide(*new_defense_tower, *game_map[6]));
}

int main() 
{
    TestRunner tr;
    RUN_TEST(tr, TestAddingNewObjectOnMap);
    return 0;
}