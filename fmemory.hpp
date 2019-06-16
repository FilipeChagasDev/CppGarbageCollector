/* DO NOT REMOVE THIS HEADER. If yout Team has problems with this code, it's me that you can turn to.
 *
 * Author: Filipe Chagas Ferraz
 * Author Email: filipe.ferraz0@gmail.com
 * Author GitHub: github.com/FilipeChagasDev
 * License: MIT
 *
 * Description:
 *      This is a C++ garbage collectior.
 *      Objects derived from the FObject class will be under the tutelage
 *      of this mechanism. For this to work, Ref<type> static objects must
 *      be used instead of pointeiros, and the CREATE<type>(args) function
 *      must be used instead of the new operator.
 */

#ifndef FMEMORY_HPP
#define FMEMORY_HPP

#include <functional>
#include <type_traits>
#include <stdexcept>
#include <cmath>

#define FDEBUG

#ifdef FDEBUG
    #include <iostream>
    #define FLOG(msg) std::cout << msg << std::endl;
#else
    #define FLOG(msg)
#endif

// ===========================================================================
// ============================= RefTree class ===============================
// ===========================================================================

class RefTree
{
protected:

    class Node
    {
    public:
        char branch; //'r', 'l' or ' '. Branch of the previous node on which it is inserted
        void *addr; //data
        Node *prev;
        Node *left;
        Node *right;

        Node(void *addr, Node *prev, char branch)
        {
            this->addr = addr;
            this->prev = prev;
            this->branch = branch;
            this->left = nullptr;
            this->right = nullptr;
        }
    };

    Node *root;

    void insertNode(Node *node);
    void insertNodeAfter(Node *node, Node *from); //Inserts the node below the node specified by 'from' argument

public:
    RefTree();
    bool empty();
    void insert(void *addr);
    Node *find(void *addr);
    void remove(Node *node);
};

RefTree::RefTree()
{
    this->root = nullptr;
}

bool RefTree::empty()
{
    return this->root == nullptr;
}

void RefTree::insert(void *addr)
{
    if(addr == nullptr) return;

    if(this->root == nullptr)
    {
        this->root = new Node(addr, nullptr,' ');
    }
    else
    {
        for(Node *i = this->root; i != nullptr; /*nothing*/ )
        {
            if(i->addr == addr) return;
            else if(i->addr < addr)
            {
                if(i->right != nullptr)
                {
                    i = i->right;
                    continue;
                }
                else
                {
                    i->right = new Node(addr, i, 'r');
                    return;
                }
            }
            else if(i->addr > addr)
            {
                if(i->left != nullptr)
                {
                    i = i->left;
                    continue;
                }
                else
                {
                    i->left = new Node(addr, i, 'l');
                    return;
                }
            }
        }
    }
}

void RefTree::insertNode(Node *node)
{
    if(node == nullptr) return;

    if(this->root == nullptr)
    {
        node->prev = nullptr;
        node->branch = ' ';
        this->root = node;
    }
    else
    {
        for(Node *i = this->root; i != nullptr; /*nothing*/ )
        {
            if(i->addr == node->addr) return;
            else if(i->addr < node->addr)
            {
                if(i->right != nullptr)
                {
                    i = i->right;
                    continue;
                }
                else
                {
                    node->prev = i;
                    node->branch = 'r';
                    i->right = node;
                    return;
                }
            }
            else if(i->addr > node->addr)
            {
                if(i->left != nullptr)
                {
                    i = i->left;
                    continue;
                }
                else
                {
                    node->prev = i;
                    node->branch = 'l';
                    i->left = node;
                    return;
                }
            }
        }
    }
}

void RefTree::insertNodeAfter(Node *node, Node *from)
{
    if(node == nullptr) return;

    if(from == nullptr)
    {
        this->insertNode(node);
    }
    else
    {
        for(Node *i = from; i != nullptr; /*nothing*/ )
        {
            if(i->addr == node->addr) return;
            else if(i->addr < node->addr)
            {
                if(i->right != nullptr)
                {
                    i = i->right;
                    continue;
                }
                else
                {
                    node->prev = i;
                    node->branch = 'r';
                    i->right = node;
                    return;
                }
            }
            else if(i->addr > node->addr)
            {
                if(i->left != nullptr)
                {
                    i = i->left;
                    continue;
                }
                else
                {
                    node->prev = i;
                    node->branch = 'l';
                    i->left = node;
                    return;
                }
            }
        }
    }
}

RefTree::Node* RefTree::find(void *addr)
{
    for(Node *i = this->root; i != nullptr; i = (i->addr > addr ? i->left : i->right))
    {
        if(i->addr == addr) return i;
    }
    return nullptr;
}

void RefTree::remove(RefTree::Node *node)
{
    if(node == nullptr) return;

    if(node->prev == nullptr)
    {
        this->root = nullptr;
        this->insertNode(node->left);
        this->insertNode(node->right);
        delete node;
    }
    else
    {
        Node *prev = node->prev;
        if(node->branch == 'l') prev->left = nullptr;
        else if(node->branch == 'r') prev->right = nullptr;
        this->insertNodeAfter(node->left, prev);
        this->insertNodeAfter(node->right, prev);
        delete node;
    }
}

// ===========================================================================
// =========================== Referable class ===============================
// ===========================================================================

class Referable
{
public:
    RefTree refs;

    void gcCheck() //garbage collector check
    {
        if(refs.empty() == true)
        {
            delete this;
        }
    }

    virtual ~Referable()
    {
    }

};

// ===========================================================================
// ============================= Ref     class ===============================
// ===========================================================================

template<class  T>
class Ref
{
static_assert(std::is_base_of<Referable, T>::value, "the Ref handler only refers to the subclasses of FObject");
private:
    void *operator new (size_t s) = delete;

protected:
    Referable *ptr;
    void unRef();
    void setRef(Referable &obj);

public:
    Ref();
    Ref(Ref &reffer);
    Ref(Referable &obj);
    ~Ref();
    void operator = (T& obj);
    void operator = (Ref reffer);
    T& operator * ();
    T* operator -> ();
    void setNull();
    bool isNull();
    bool sameReference(Ref &refer);

    template<class T2>
    T2& cast()
    {
        static_assert(std::is_base_of<T, T2>::value, "Invalid reference type conversion. Conversions with the fref::to<type>() method need to be from superclass to subclass." );
        return *static_cast<T2*>(this->ptr);
    }
};

template<class  T>
Ref<T>::Ref()
{
    this->ptr = nullptr;
}
template<class  T>
Ref<T>::Ref(Ref &reffer)
{
    this->ptr = nullptr;
    this->setRef(*static_cast<Referable*>(reffer.ptr));
}

template<class  T>
Ref<T>::Ref(Referable &obj)
{
    this->ptr = nullptr;
    try {
        dynamic_cast<T&>(obj);
    } catch (std::bad_cast &ex) {
        throw std::runtime_error( std::string("Ref<") + (typeid(T).name()+1) +"> cannot refer an " + (typeid(obj).name()+1) + " object");
    }

    this->setRef(obj);
}

template<class  T>
Ref<T>::~Ref()
{
    this->unRef();
}

template<class  T>
void Ref<T>::unRef()
{
    if(this->ptr != nullptr)
    {
        this->ptr->refs.remove( this->ptr->refs.find(this) );
        this->ptr->gcCheck();
    }
}

template<class  T>
void Ref<T>::setRef(Referable &obj)
{
    if(this->ptr != &obj)
    {
        this->unRef();
        this->ptr = &obj;
        obj.refs.insert(this);
    }
}

template<class  T>
void Ref<T>::operator = (T& obj)
{
    this->setRef(static_cast<Referable&>(obj));
}

template<class  T>
void Ref<T>::operator = (Ref reffer)
{
    this->setRef(*static_cast<Referable*>(reffer.ptr));
}

template<class  T>
T& Ref<T>::operator * ()
{
    if(this->ptr == nullptr) throw std::runtime_error("\'*\' operator used with a Null reference");
    return  *(static_cast<T*>(this->ptr));
}

template<class  T>
T* Ref<T>::operator -> ()
{
    if(this->ptr == nullptr) throw std::runtime_error("\'->\' operator used with a Null reference");
    return  static_cast<T*>(this->ptr);
}

template<class  T>
void Ref<T>::setNull()
{
    this->unRef();
    this->ptr = nullptr;
}

template<class  T>
bool Ref<T>::isNull()
{
    return this->ptr == nullptr;
}

template<class  T>
bool Ref<T>::sameReference(Ref &refer)
{
    return this->ptr == refer.ptr;
}

// ===========================================================================
// ============================= FObject class ===============================
// ===========================================================================

class FObject : public Referable
{
private:
    void *operator new(size_t sz)
    {
        return malloc(sz);
    }

public:
    template<class T, class ...Args>
    static T& create(Args... args)
    {
        static_assert(std::is_base_of<FObject, T>::value, "the \'create\' function only construct subclasses of FObject");
        T* ptr = new T(args...);
        //RefPack pack(static_cast<Referable*>(ptr));
        return *ptr;
    }

};

#define FOBJECT : public FObject
#define CREATE FObject::create

// ===========================================================================
// ===========================================================================
// ===========================================================================


#endif //FMEMORY_HPP
