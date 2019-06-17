#### If you want to use this in your project, just download the <i>'fmemory.hpp'</i> file and include it, all the declarations and implementations are contained in it. This file only depends on the standard C++17 libraries.

# C++ Garbage Collector
The 'fmemory.hpp' file contains the implementation of a simple garbage collection mechanism in C++ (to be used in C++ programs, and not necessarily in interpreters of other languages made in C++). Here's a brief description of how to use it...

## the FObject class
This should be the base class of all classes whose objects should be managed by the garbage collector. For a class to be compatible with the garbage collector, it must have a public inheritance for FObject, as shown in the following example:

``` cpp
    #include <iostream>
    #include "fmemory.hpp"
    
    using namespace std;
    
    class Person : public FObject
    {
    protected:
          string name;
    public:
          Person(string name)
          {
                this->name = name;
          }

          string getName()
          {
                return this->name;
          }
    };
    
    
    class Programmer : public Person
    {
    protected:
          string language;
    public:
          Programmer(string name, string language) : Person(name)
          {
                this->lang = language;
          }
    };
```
In the example above, the Person and Programmer classes are compatible with the garbage collector.
You can also use the FOBJECT macro, if you prefer, as shown in the following example...

``` cpp
    #include <iostream>
    #include "fmemory.hpp"
    
    using namespace std;
    
    class Person FOBJECT
    {
    protected:
          string name;
    public:
          Person(string name)
          {
                this->name = name;
          }
    };
```
This macro is defined as <i><b>#define FOBJECT : public FObject</b></i>

## the CREATE function and the Ref class
FObject family classes can not be instantiated by the 'new' operator and can not be deleted by the 'delete' operator. Instead, they should be instantiated by the <b>FObject::create</b> function (or simply <b>CREATE</b>) and should be referenced by static objects of the Ref class instead of pointers. You can not explicitly delete objects from the FObject family.

The following examples show how to instantiate a Person object and reference it by a Ref:

``` cpp
    void test()
    {
          Ref<Person> p;
          p = CREATE<Person>("Filipe");
    }
```

``` cpp
    void test()
    {
          Ref<Person> p ( CREATE<Person>("Filipe") );
    }
```
Notice that the arguments that are passed to the <i>CREATE</i> function are the constructor arguments of the Person class.
The Ref can also be used as a parameter by reference, as shown in the following example:

```cpp
   void show_name( Ref<Person> arg )
   {
          cout << arg->getName() << endl;
   }
```

The '*' and '->' operators can be used with the Ref class in the same way as they are used in pointers. The operators '[]' and '()' do not work in Ref in the same way that it works on a pointer. If Ref refers to an object of a class that overloads these operators, they must be used as follows:

``` cpp
    #include <iostream>
    #include "fmemory.hpp"
    
    using namespace std;
    
    class Vec3d FOBJECT
    {
    protected:
          double buff[3];
    public:
          Vec3d(double x, double y, double z)
          {
                this->buff[0] = x;
                this->buff[1] = y;
                this->buff[2] = z;
          }

          double& operator [] (unsigned int i)
          {
                return this->buff[ i % 3 ];
          }

          double& operator () (unsigned int i)
          {
                return this->buff[ i % 3 ];
          }
    };
    
    void test()
    {
          Ref<Vec3d> myvector ( CREATE<Vec3d>(5,2,6) );

          // ---- USING THE [] OPERATOR ----  
          double x = (*myvector)[0];
          double y = (*myvector)[1];
          double z = (*myvector)[2];

          cout << "vector = [" << x << ", " << y << ", " << z << "]" << endl;
    }
    
    
    void test2()
    {
          Ref<Vec3d> myvector ( CREATE<Vec3d>(5,2,6) );

          // ---- USING THE () OPERATOR ----
          double x = (*myvector)(0);
          double y = (*myvector)(1);
          double z = (*myvector)(2);

          cout << "vector = [" << x << ", " << y << ", " << z << "]" << endl;
    }
```

## Assignments between Refs and Casting

Refs can receive references from other Refs as follows:
(Note that in this example all Refs and Objects created are of the Person class).

```cpp
   void test()
   {
          Ref<Person> a ( CREATE<Person>("Filipe") );
          Ref<Person> b ( CREATE<Person>("Chagas") );
          a = b; // 'a' receives the reference from 'b'
          // The Person object with name "Filipe" is collected.
          b = a; // 'b' receives the reference from 'a'
          // 'a' and 'b' had the same reference. Nothing happens.
          a = CREATE<Person>("Ferraz"); // 'a' receives a new reference
   }
```
A Ref for a given class can receive subclass objects of this class, as shown in the following example:

```cpp
   void test()
   {
          Ref<Person> a;
          Ref<Programmer> b ( CREATE<Programmer>("Filipe", "C++") );
          // Programmer is subclass of Person
          a = b; // 'a' receives the reference from 'b'
   }
```

However, a reference to a particular class can NOT receive an object that is not of this same class or a subclass of that class, even if that object has a common superclass.
The following example shows a case similar to this, which causes a <i>runtime_error</i> exception:

```cpp
   #include "fmemory.hpp"
   
   class Mammal FOBJECT { /*nothing*/ };
   
   class Human : public Mammal { /*nothing*/ };
   
   class Dog : public Mammal { /*nothing*/ };
   
   int main()
   {
          Ref<Human> a;
          Ref<Dog> b ( CREATE<Dog>() );
          a = b; // issue here
          return 0;
   }
   
   /* PROGRAM OUTPUT:
    terminate called after throwing an instance of 'std::runtime_error'
      what():  Ref<Person> cannot refer an Mem object
   */
```

See another example, still using the classes in the previous example:
The code in this other example causes a compile error. In the case of GCC, the error message is <i><b> "error: not viable overloaded = '" </b></i>.

```cpp
   int main()
   {
          Ref<Human> a;
          Ref<Mammal> b ( CREATE<Human>() );
          a = b; // issue here
          return 0;
   }
```

This is because, even though <i>Ref<Mammal></i> references a Human object, <i>Ref<Human></i> can not receive an assignment of <i>Ref<Mammal></i> without a cast being made.
This assignment can be done if the <b>cast</b> method is used, as follows:

```cpp
   int main()
   {
          Ref<Human> a;
          Ref<Mammal> b ( CREATE<Human>() );
          a = b.cast<Human>();
          return 0;
   }
```

If the cast is made between incompatible types, a <i>runtime_error</i> will be throwed. The following example shows one such case:

```cpp
   int main()
   {
          Ref<Human> a;
          Ref<Mammal> b ( CREATE<Dog>() );
          a = b.cast<Human>(); //issue here
          return 0;
   }
   
   /* PROGRAM OUTPUT:
   terminate called after throwing an instance of 'std::runtime_error'
      what():  Impossible cast from Dog to Human
   */
```
## Static FObjects
I do not recommend that you create static objects (without using the CREATE function) from the FObject Family. There's nothing to stop you from doing this, but the fmemory library was not meant to work this way.

```cpp
   #include "fmemory.hpp"
   class MyClass FOBJECT { /* nothing */ };
   
   int main()
   {
          // --- NEVER DO THAT ---
          MyClass obj; //static FObject
          Ref<MyClass> a; //Ref programmed to work ONLY WITH DYNAMICALLY CREATED FOBJECTS
          a = obj; //Ref referencing a static FObject
          // ---------------------
          return 0;
   }
```

The behavior of the program, in this case, is undefined.

## NULL Refs
Refs can not receive NULL or nullptr as assignment. Instead, you should use the <b>setNull</b> method, as shown in the following example:

```cpp
   #include "fmemory.hpp"
   class MyClass FOBJECT { /* nothing */ };
   
   int main()
   {
          Ref<MyClass> a ( CREATE<MyClass>() );
          a.setNull(); //Ref becomes null
          //The MyClass object is collected
          return 0;
   }
```

to verify that a Ref is null, use the <b>isNull</b> method...


```cpp
   #include <iostream>
   #include "fmemory.hpp"
   using namespace std;
   
   class MyClass FOBJECT { /* nothing */ };
   
   int main()
   {
          Ref<MyClass> a ( CREATE<MyClass>() );
          a.setNull(); //Ref becomes null
          //The MyClass object is collected
          
          if(a.isNull() == true) cout << "a is null" << endl;
          else cout << "a is not null" << endl;
          
          return 0;
   }
   
   /*   PROGRAM OUTPUT:
            a is null
   */
```

## Comparing references
The relational operators '==', '>', '<', '<=', '>=' and '!=' do not works with Refs. To verify that two Refs refer to the same object, use the <b>sameReference</b> method, as shown in the following example:


```cpp
   #include <iostream>
   #include "fmemory.hpp"
   using namespace std;
   
   class MyClass FOBJECT { /* nothing */ };
   
   int main()
   {
          Ref<MyClass> a ( CREATE<MyClass>() );
          Ref<MyClass> b;
          b = a;
          
          if(a.sameReference(b) == true) cout << "a = b" << endl;
          else cout << "a != b" << endl;
          
          
          if(b.sameReference(a) == true) cout << "b = a" << endl;
          else cout << "b != a" << endl;
          
          return 0;
   }
   
   /*   PROGRAM OUTPUT:
            a = b
            b = a
   */
```

If both Refs are null, the sameReference method also returns true:

```cpp
   #include <iostream>
   #include "fmemory.hpp"
   using namespace std;
   
   class MyClass FOBJECT { /* nothing */ };
   
   int main()
   {
          Ref<MyClass> a;
          Ref<MyClass> b;
          b = a;
          
          if(a.sameReference(b) == true) cout << "a = b" << endl;
          else cout << "a != b" << endl;
          
          
          if(b.sameReference(a) == true) cout << "b = a" << endl;
          else cout << "b != a" << endl;
          
          return 0;
   }
   
   /*   PROGRAM OUTPUT:
            a = b
            b = a
   */
```
