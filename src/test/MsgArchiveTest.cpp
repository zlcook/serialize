#include <glog/logging.h>
#include <gtest/gtest.h>

#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <iostream>
#include <tuple>

#include "MsgArchive.hpp"

namespace zlcook {

/*trivial class
 *no standard layout
 * */
class Person {
 public:
  Person(){};
  Person(int a, bool g, std::string n, float s)
      : age(a), gender(g), name(n), salary(s) {}

  void toString() {
    std::cout << "name: " << name << ", age: " << age << ", gender: " << gender
              << ", salary: " << salary << "\n";
  }

  friend bool operator==(const Person& l, const Person& r) {
    return l.age == r.age && l.gender == r.gender && l.name == r.name &&
           l.salary == r.salary;
  }

  friend bool operator!=(const Person& l, const Person& r) {
    return !(l.age == r.age && l.gender == r.gender && l.name == r.name &&
             l.salary == r.salary);
  }

 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& age;
    ar& gender;
    ar& name;
    ar& salary;
  }

  int age;
  bool gender;
  std::string name;
  float salary;
};

struct Simple {
  int age;
  bool gender;

  friend bool operator==(const Simple& l, const Simple& r) {
    return l.age == r.age && l.gender == r.gender;
  }
};

/* No Trivial class */
class Company {
 public:
  Company() {}
  Company(std::string n, int y) : name(n), years(y) {}

  void addPerson(Person p) { persons.push_back(p); }

  void addSalary(int key, float money) { salarys.emplace(key, money); }

  friend bool operator==(const Company& l, const Company& r) {
    if (l.name != r.name) return false;
    if (l.years != r.years) return false;
    if (l.persons.size() != r.persons.size()) return false;

    for (size_t i = 0; i < l.persons.size(); i++) {
      if (l.persons[i] != r.persons[i]) return false;
    }
    if (l.salarys.size() != r.salarys.size()) return false;
    for (const auto& iter : l.salarys) {
      int key = iter.first;
      auto val = iter.second;
      if (r.salarys.find(key) == r.salarys.end()) return false;
      const float& r_val = r.salarys.at(key);
      if (val != r_val) return false;
    }
    return true;
  }

  void toString() {
    std::cout << "name: " << name << ", years: " << years;
    std::cout << "persons:\n ";
    for (auto& p : persons) {
      p.toString();
    }
    std::cout << "salarys:\n ";
    for (auto& iter : salarys) {
      std::cout << "key: " << iter.first << ", value: " << iter.second << "\n";
    }
  }

 private:
  friend class boost::serialization::access;

  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& name;
    ar& years;
    ar& persons;
    ar& salarys;
  }

  std::string name;
  int years;
  std::vector<Person> persons;
  std::map<int, float> salarys;
};

TEST(MsgArchiveTest, MsgArchive_Trivial_NoStandardLayout) {
  const int COUNT = 100;
  OutMsgArchive<Person, MemSink> oa(200);
  for (int i = 0; i < COUNT; i++) {
    Person ep(i, true, std::to_string(i), i * 100.0);
    oa.Emit(ep);
  }
  oa.Flush();
  ASSERT_EQ(COUNT, oa.Count());

  InMsgArchive<Person, MemSource> ia(oa.GetBuffer(), oa.Count());
  for (int i = 0; i < COUNT; i++) {
    Person ep(i, true, std::to_string(i), i * 100.0);
    auto person_ptr = ia.Absorb();
    ASSERT_EQ(ep, *person_ptr);
  }
  ASSERT_EQ(0, ia.Count());
  ASSERT_EQ(0, ia.Size());
}

TEST(MsgArchiveTest, MsgArchive_Trivial_StandardLayout) {
  OutMsgArchive<Simple> oa(100);
  for (int i = 0; i < 10; i++) {
    Simple sp{i, true};
    oa.Emit(sp);
  }
  oa.Flush();
  ASSERT_EQ(10, oa.Count());
  ASSERT_EQ(10 * sizeof(Simple), oa.Size());

  InMsgArchive<Simple> ia(oa.GetBuffer(), oa.Count());
  for (int i = 0; i < 10; i++) {
    Simple sp{i, true};
    auto sp_ptr = ia.Absorb();
    ASSERT_EQ(sp, *sp_ptr);
  }
  ASSERT_EQ(0, ia.Count());
  ASSERT_EQ(0, ia.Size());
}

TEST(MsgArchiveTest, MsgArchive_NoTrivial) {
  OutMsgArchive<Company> oa(100);
  for (int i = 0; i < 10; i++) {
    Company cp(std::to_string(i), i);
    for (int j = 0; j < 3; j++) {
      Person p(i, true, std::to_string(i), i * 100.0);
      cp.addPerson(p);
      cp.addSalary(j, i * 100.0);
    }
    oa.Emit(cp);
  }
  oa.Flush();
  ASSERT_EQ(10, oa.Count());

  InMsgArchive<Company> ia(oa.GetBuffer(), oa.Count());
  for (int i = 0; i < 10; i++) {
    Company expect_cp(std::to_string(i), i);
    for (int j = 0; j < 3; j++) {
      Person p(i, true, std::to_string(i), i * 100.0);
      expect_cp.addPerson(p);
      expect_cp.addSalary(j, i * 100.0);
    }
    auto cp_ptr = ia.Absorb();
    ASSERT_EQ(expect_cp, *cp_ptr);
    // cp_ptr->toString();
  }
  ASSERT_EQ(0, ia.Count());
  ASSERT_EQ(0, ia.Size());
}

}  // namespace zlcook

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
