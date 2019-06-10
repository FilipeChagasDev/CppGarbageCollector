#include <iostream>
#include "fmemory.hpp"

class Person FCOMPATIBLE
{
public:
    std::string name;
    Person(std::string name)
    {
        this->name = name;
    }
};

class Programmer : public Person
{
public:
    Programmer(std::string name) : Person(name) {}
};


void show_name( BYREF(Person) p )
{
    std::cout << p->name << std::endl;
}

void test()
{
    Ref<Person> a( CREATE<Person>("Filipe") );
    Ref<Programmer> b;

    // -------- memory leakage test ---------
    for(int i = 0; i < 500000; i++)
    {
        a = CREATE<Programmer>("Filipe");
        b = a.cast<Programmer>();
        show_name(a);
    }
}

int main()
{
    test();
    return 0;
}
