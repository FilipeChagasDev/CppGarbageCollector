#ifndef CPPFRAMEWORK_HPP
#define CPPFRAMEWORK_HPP

#include <functional>
#include <type_traits>
#include <stdexcept>
#include <cmath>

// ===========================================================================
// ============== OO BASE, REFERENCES AND GARBAGE COLLECTOR ==================
// ===========================================================================

class RefTree
{
protected:
    class Node
    {
    public:
        char branch;
        void *addr;
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

public:
    RefTree()
    {
        this->root = nullptr;
    }

    bool empty()
    {
        return this->root == nullptr;
    }

    void insert(void *addr)
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

    void insertNode(Node *node)
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

    Node *find(void *addr)
    {
        std::function<Node*(Node*)> depth_search = [&](Node *curr)->Node*
        {
            if(curr->addr == addr) return curr;
            if(curr->left != nullptr) return depth_search(curr->left);
            if(curr->right != nullptr) return depth_search(curr->right);
            return nullptr;
        };
        return (root != nullptr ? depth_search(this->root) : nullptr );
    }

    void remove(Node *node)
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
            if(node->branch == 'l') node->prev->left = nullptr;
            else if(node->branch == 'r') node->prev->right = nullptr;
            this->insertNode(node->left);
            this->insertNode(node->right);
            delete node;
        }
    }
};

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

class RefPack
{
public:
    Referable *ptr;

    RefPack(Referable *ptr)
    {
        this->ptr = ptr;
    }
};

template<class  T>
class Ref
{
static_assert(std::is_base_of<Referable, T>::value, "the Ref handler only refers to the subclasses of FObject");
private:
    void *operator new (size_t s) = delete;

protected:
    Referable *ptr;

    void unRef()
    {
        if(this->ptr != nullptr)
        {
            this->ptr->refs.remove( this->ptr->refs.find(this) );
            this->ptr->gcCheck();
        }
    }

    void setRef(Referable &obj)
    {
        if(this->ptr != &obj)
        {
            this->unRef();
            this->ptr = &obj;
            obj.refs.insert(this);
        }
    }


public:


    Ref()
    {
        this->ptr = nullptr;
    }

    Ref(Ref &reffer)
    {
        this->ptr = nullptr;
        this->setRef(*static_cast<Referable*>(reffer.ptr));
    }

    Ref(RefPack reference)
    {
        this->ptr = nullptr;
        if(dynamic_cast<T*>(reference.ptr) == nullptr) throw std::runtime_error("No matching type on reference set");
        this->setRef(*(reference.ptr));
    }

    ~Ref()
    {
        this->unRef();
    }

    void operator = (T& obj)
    {
        this->setRef(static_cast<Referable>(obj));
    }


    void operator = (Ref& reffer)
    {
        this->setRef(*static_cast<Referable*>(reffer.ptr));
    }

    void operator = (Ref reffer)
    {
        this->setRef(*static_cast<Referable*>(reffer.ptr));
    }

    void operator = (RefPack reference)
    {
        if(dynamic_cast<T*>(reference.ptr) == nullptr) throw std::runtime_error("No matching type on reference set");
        this->setRef(*(reference.ptr));
    }

    T& operator * ()
    {
        if(this->ptr == nullptr) throw std::runtime_error("\'*\' operator used with a Null reference");
        return  *(static_cast<T*>(this->ptr));
    }

    T* operator -> ()
    {
        if(this->ptr == nullptr) throw std::runtime_error("\'->\' operator used with a Null reference");
        return  static_cast<T*>(this->ptr);
    }

    void setNull()
    {
        this->unRef();
        this->ptr = nullptr;
    }

    bool isNull()
    {
        return this->ptr == nullptr;
    }

    bool sameReference(Ref &refer)
    {
        return this->ptr == refer.ptr;
    }

    template<class T2>
    RefPack cast()
    {
        static_assert(std::is_base_of<T, T2>::value, "Invalid reference type conversion. Conversions with the fref::to<type>() method need to be from superclass to subclass." );
        RefPack r(this->ptr);
        return r;
    }

};



class FObject : public Referable
{
private:
    void *operator new(size_t sz)
    {
        return malloc(sz);
    }

public:
    template<class T, class ...Args>
    static RefPack create(Args... args)
    {
        static_assert(std::is_base_of<FObject, T>::value, "the \'create\' function only construct subclasses of FObject");
        T* ptr = new T(args...);
        RefPack pack(static_cast<Referable*>(ptr));
        return pack;
    }

};

#define BYREF(type) Ref<type> &
#define FCOMPATIBLE : public FObject
#define CREATE FObject::create

// ===========================================================================
// ===========================================================================
// ===========================================================================


#endif // CPPFRAMEWORK_HPP
