#include <glog/logging.h>
#include <gtest/gtest.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <iostream>
#include <tuple>

#include "MemDevice.hpp"

namespace zlcook {

/*trivial class*/
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
    std::cout << "name: " << name << ", years: " << years << "\n";
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

TEST(ArchiveTest, Basic) {
  boost::iostreams::stream<MemSink> os(500);
  // serize
  {
    boost::archive::binary_oarchive oa(os);
    for (int i = 0; i < 10; i++) {
      oa& i;
    }
  }

  // deserize
  boost::iostreams::stream<MemSource> is(os->GetBuffer());
  {
    boost::archive::binary_iarchive ia(is);
    int number;
    for (int i = 0; i < 10; i++) {
      ia& number;
      ASSERT_EQ(i, number);
    }
  }
}

TEST(ArchiveTest, Trivial) {
  boost::iostreams::stream<MemSink> os(500);
  // serize
  {
    boost::archive::binary_oarchive oa(os);
    for (int i = 0; i < 10; i++) {
      Person p(i, true, std::to_string(i), i * 100.0);
      oa& p;
    }
  }

  // deserize
  boost::iostreams::stream<MemSource> is((*os).GetBuffer());
  {
    boost::archive::binary_iarchive ia(is);
    for (int i = 0; i < 10; i++) {
      Person ep(i, true, std::to_string(i), i * 100.0);
      Person p;
      ia& p;
      ASSERT_EQ(ep, p);
    }
  }
}

TEST(ArchiveTest, NoTrivial) {
  boost::iostreams::stream<MemSink> os(500);
  // serize
  {
    boost::archive::binary_oarchive oa(os);
    for (int i = 0; i < 10; i++) {
      Company cp(std::to_string(i), i);
      for (int j = 0; j < 3; j++) {
        Person p(i, true, std::to_string(i), i * 100.0);
        cp.addPerson(p);
        cp.addSalary(j, i * 100.0);
      }
      oa& cp;
    }
  }

  // deserize
  boost::iostreams::stream<MemSource> is((*os).GetBuffer());
  {
    boost::archive::binary_iarchive ia(is);
    for (int i = 0; i < 10; i++) {
      Company expect_cp(std::to_string(i), i);
      for (int j = 0; j < 3; j++) {
        Person p(i, true, std::to_string(i), i * 100.0);
        expect_cp.addPerson(p);
        expect_cp.addSalary(j, i * 100.0);
      }
      Company cp;
      ia& cp;
      // cp.toString();
      ASSERT_EQ(expect_cp, cp);
    }
  }
}
}  // namespace zlcook

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
