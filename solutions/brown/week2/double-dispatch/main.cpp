#include "game_object.h"
#include "geo2d.h"

#include "test_runner.h"

#include <memory>
#include <vector>

using namespace std;

// Определите классы Unit, Building, Tower и Fence так, чтобы они наследовались от
// GameObject и реализовывали его интерфейс.

class Unit : public GameObject {
public:
  explicit Unit(geo2d::Point position) : point(position) {}

  bool Collide(const GameObject &that) const override { return that.CollideWith(*this); }

  bool CollideWith(const Unit &that) const override { return geo2d::Collide(this->point, that.point); }
  bool CollideWith(const Building &that) const override;

  bool CollideWith(const Tower &that) const override;

  bool CollideWith(const Fence &that) const override;

  geo2d::Point point;
};

class Building : public GameObject {
public:
  explicit Building(geo2d::Rectangle geometry) : geometry(geometry) {}

  bool Collide(const GameObject &that) const override { return that.CollideWith(*this); }

  bool CollideWith(const Unit &that) const override { return geo2d::Collide(this->geometry, that.point); }
  bool CollideWith(const Building &that) const override { return geo2d::Collide(this->geometry, that.geometry); }
  bool CollideWith(const Tower &that) const override;

  bool CollideWith(const Fence &that) const override;

  geo2d::Rectangle geometry;
};

class Tower : public GameObject {
public:
  explicit Tower(geo2d::Circle geometry) : circle(geometry) {}

  bool Collide(const GameObject &that) const override { return that.CollideWith(*this); }
  bool CollideWith(const Unit &that) const override { return geo2d::Collide(this->circle, that.point); }
  bool CollideWith(const Building &that) const override { return geo2d::Collide(this->circle, that.geometry); }
  bool CollideWith(const Tower &that) const override { return geo2d::Collide(this->circle, that.circle); }
  bool CollideWith(const Fence &that) const override;

  geo2d::Circle circle;
};

class Fence : public GameObject {
public:
  explicit Fence(geo2d::Segment geometry) : segment(geometry) {}

  bool Collide(const GameObject &that) const override { return that.CollideWith(*this); }
  bool CollideWith(const Unit &that) const override { return geo2d::Collide(this->segment, that.point); }
  bool CollideWith(const Building &that) const override { return geo2d::Collide(this->segment, that.geometry); }
  bool CollideWith(const Tower &that) const override { return geo2d::Collide(this->segment, that.circle); }
  bool CollideWith(const Fence &that) const override { return geo2d::Collide(this->segment, that.segment); }

  geo2d::Segment segment;
};

bool Unit::CollideWith(const Building &that) const { return geo2d::Collide(this->point, that.geometry); }

bool Unit::CollideWith(const Tower &that) const { return geo2d::Collide(this->point, that.circle); }

bool Unit::CollideWith(const Fence &that) const { return geo2d::Collide(this->point, that.segment); }

bool Building::CollideWith(const Tower &that) const { return geo2d::Collide(this->geometry, that.circle); }

bool Building::CollideWith(const Fence &that) const { return geo2d::Collide(this->geometry, that.segment); }

bool Tower::CollideWith(const Fence &that) const { return geo2d::Collide(this->circle, that.segment); }

// Реализуйте функцию Collide из файла GameObject.h

bool Collide(const GameObject &first, const GameObject &second) { return first.Collide(second); }

void TestAddingNewObjectOnMap() {
  // Юнит-тест моделирует ситуацию, когда на игровой карте уже есть какие-то объекты,
  // и мы хотим добавить на неё новый, например, построить новое здание или башню.
  // Мы можем его добавить, только если он не пересекается ни с одним из существующих.
  using namespace geo2d;

  const vector<shared_ptr<GameObject>> game_map = {make_shared<Unit>(Point{3, 3}),
                                                   make_shared<Unit>(Point{5, 5}),
                                                   make_shared<Unit>(Point{3, 7}),
                                                   make_shared<Fence>(Segment{{7, 3}, {9, 8}}),
                                                   make_shared<Tower>(Circle{Point{9, 4}, 1}),
                                                   make_shared<Tower>(Circle{Point{10, 7}, 1}),
                                                   make_shared<Building>(Rectangle{{11, 4}, {14, 6}})};

  for (size_t i = 0; i < game_map.size(); ++i) {
    Assert(Collide(*game_map[i], *game_map[i]), "An object doesn't collide with itself: " + to_string(i));

    for (size_t j = 0; j < i; ++j) {
      Assert(!Collide(*game_map[i], *game_map[j]), "Unexpected collision found " + to_string(i) + ' ' + to_string(j));
    }
  }

  auto new_warehouse = make_shared<Building>(Rectangle{{4, 3}, {9, 6}});
  ASSERT(!Collide(*new_warehouse, *game_map[0]));
  ASSERT(Collide(*new_warehouse, *game_map[1]));
  ASSERT(!Collide(*new_warehouse, *game_map[2]));
  ASSERT(Collide(*new_warehouse, *game_map[3]));
  ASSERT(Collide(*new_warehouse, *game_map[4]));
  ASSERT(!Collide(*new_warehouse, *game_map[5]));
  ASSERT(!Collide(*new_warehouse, *game_map[6]));

  auto new_defense_tower = make_shared<Tower>(Circle{{8, 2}, 2});
  ASSERT(!Collide(*new_defense_tower, *game_map[0]));
  ASSERT(!Collide(*new_defense_tower, *game_map[1]));
  ASSERT(!Collide(*new_defense_tower, *game_map[2]));
  ASSERT(Collide(*new_defense_tower, *game_map[3]));
  ASSERT(Collide(*new_defense_tower, *game_map[4]));
  ASSERT(!Collide(*new_defense_tower, *game_map[5]));
  ASSERT(!Collide(*new_defense_tower, *game_map[6]));
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestAddingNewObjectOnMap);
  return 0;
}