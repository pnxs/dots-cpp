#pragma once

#include "dots/cpp_config.h"

namespace dots
{

/**
 * This class makes it possible to create a list of types, that are using a template, accessible for
 * iteration before main().
 *
 * Usage:
 *
 * Let a class use Chained-Template
 * struct ListName: public public Chained<ListName>
 * {
 *    ListName(int id);
 * }
 *
 * Create a template, that constructs a
 * template<class T, class S>
 * class Existence
 * {
 * public:
 *    static S& get()
 *    {
 *        return m_obj;
 *    }
 * private:
 *    static S m_obj;
 * };
 *
 * template<class T, class S>
 * S Existence<T, S>::m_obj(T::some_static method_that_returns_what_the_contructor_needs());
 *
 * For all different types of this template a entry is made into the list:
 * Existence<T, ListName>::get();
 *
 * Get list of all different  of the specific type:
 * ListName::allChained()
 *
 */
template<class T>
class Chained
{
    static list<T*>& getChain()
    {
        static list<T*>* chain = new list<T*>;
        return *chain;
    }

public:
    Chained()
    {
        getChain().push_back(static_cast<T*>(this));
    }

    static const list<T*> &allChained()
    {
        return getChain();
    }
};

}