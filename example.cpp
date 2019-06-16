#include <iostream>
#include "fmemory.hpp"

class Person FOBJECT
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

class Mem FOBJECT {};

void show_name( Ref<Person> p )
{
    std::cout << p->name << std::endl;
}

void test()
{
    Ref<Person> a( CREATE<Person>("Filipe") );
    Ref<Programmer> b;

    // -------- memory leakage test ---------
    for(int i = 0; i < 1000000; i++)
    {
        a = CREATE<Programmer>("Filipe");

        //This line of code causes a run-time error. Uncomment it to see.
        //a = CREATE<Mem>();

        b = a.cast<Programmer>();
        show_name(a);
    }
}

int main()
{
    test();
    return 0;
}
