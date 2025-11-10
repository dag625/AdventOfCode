//
// Created by Daniel Garcia on 10/31/2025.
//

#include "circular_list.h"

#include <list>
#include <string>

#include "doctest/doctest.h"

namespace aoc {

    void list_test() {
        int i = 0;
        std::list<int>::const_iterator::reference x = i;
        std::list<int>::const_iterator::pointer y = &i;
        std::list<int>::const_iterator::value_type z = i;

        std::list<int> l1;
        auto e1 = l1.end();
        l1.assign_range(std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
        l1.insert(l1.begin(), {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});_STL_VERIFY(true, "");
    }


    inline void test() {
        circular_list<int> t1;
        circular_list<const int> t2;
        const circular_list<int> t3;
        circular_list_iterator<int> i0{};
        auto i1 = t1.begin();
        auto i2 = t2.begin();
        auto i3 = t3.begin();
        auto i4 = t2.cbegin();
        auto& v1 = *i1;
        auto& v2 = *i2;
        auto& v3 = *i3;
        auto& v5 = *i4;
        swap(i0, i1);

        circular_list<std::string> sl;
        auto si1 = sl.begin();
        auto si2 = sl.cbegin();
        auto svs1 = si1->size();
        auto svs2 = si2->size();
        si1->push_back('a');
        //si2->push_back('a');//No bueno, as we want
    }

} /* namespace aoc */