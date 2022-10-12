#include <iostream>
#include <iterator>
#include <array>
#include <cstdint>
#include <vector>
#include <list>
/*
 * Consider 1D dimensional puddles determined by an array of integer heights.
 * Compute the maximal amount of water which these puddles can hold.
 *
 * Example: The single puddle on the relief below
 *          can hold 4 units of water marked 'W'.
 *     _
 *    | |  _
 *    | |W| |
 *   _| |W| |
 * _|   |W| |
 *      |W| |_
 * 1 2 5 0 4 0
 *
 */
using namespace std;
using Amount = uintmax_t;
/*
 * Using an iterative approach, we have to hold all descending steps. 
 * In the worst case, when all the sequence is descending, 
 * 2*N additional memory will be required. 
 * The requirement can be relaxed to 1*N at the cost of some extra code.
 */
template <typename Iterator>
Amount waterInPuddles(Iterator begin, Iterator end)
{
    if (begin == end)
    {
        return 0;
    }
    using Height = typename iterator_traits<Iterator>::value_type;
    using Length = typename iterator_traits<Iterator>::difference_type;
    struct Level
    {
        Height height;
        Length length;
        explicit Level(Height h)  : height(h), length(1) {}
        Level(Height h, Length l) : height(h), length(l) {}
    };
    using Levels = vector<Level>;
    Levels levels;
    levels.emplace_back(*begin++);
    uintmax_t water = 0;
    for (; begin != end; ++begin)
    {
        Height h = *begin;
        Level &last = levels.back();
        while (h > last.height && levels.size() > 1)
        {
            Level puddle = last;
            levels.pop_back();
            last = levels.back();
            Height puddleDepth = min(last.height, h) - puddle.height;
            Length puddleLength = puddle.length;
            water += puddleDepth * puddleLength;
            last.length += puddleLength;
        }
        if (h > last.height)
        {
            last.height = h;
        }
        else if (h < last.height)
        {
            levels.emplace_back(h);
        }
        else //if (h == last.height)
        {
            ++last.length;
        }
    }
    return water;
}
/*
 * The symmetric approach does not require additional memory for computations,
 * but all the sequence must be loaded into memory beforehand. 
 * It cannot be used to handle very long input sequences.
 */
template <class T_container>
Amount waterInPuddles(const T_container &heights)
{
    if (size(heights) < 3)
    {
        return 0;
    }
    auto left = begin(heights);
    auto right = rbegin(heights);
    auto lmax = *left;
    auto rmax = *right;
    Amount water = 0;
    auto stepInto = [&water](auto &it, int &sideMax)
    {
        ++it;
        water += max(sideMax - *it, 0);
        sideMax = max(sideMax, *it);
    };
    while (&*left != &*right)
    {
        if (lmax < rmax)
        {
            stepInto(left, lmax);
        }
        else
        {
            stepInto(right, rmax);
        }
    }
    return water;
}

int main()
{
    vector<int> heightsV = {1, 2, 5, 0, 4, 0};
    list<int> heightsL = {0, 1, 3, -2, -3, 5};
    array<int, 15> heightsA = {0, 1, 3, -2, -3, 5, 4, 2, 1, 0, 2, -1, -2, 0, -1};
    cout << waterInPuddles(begin(heightsV), end(heightsV)) << endl; // 4
    cout << waterInPuddles(begin(heightsL), end(heightsL)) << endl; // 11
    cout << waterInPuddles(begin(heightsA), end(heightsA)) << endl; // 17
    cout << waterInPuddles(heightsV) << endl;                       // 4
    cout << waterInPuddles(heightsL) << endl;                       // 11
    cout << waterInPuddles(heightsA) << endl;                       // 17
    return 0;
}
int main0()
{
    // vector<int> heightsV = { 1, 2, 5, 0, 4, 0 };
    // cout << waterInPuddles(heightsV) << endl; // 4
    // list<int> heightsL = { 0, 1, 3, -2, -3, 5 };
    // cout << waterInPuddles(heightsL) << endl; // 11
    // array<int, 15> heightsA = { 0, 1, 3, -2, -3, 5, 4, 2, 1, 0, 2, -1, -2, 0, -1 };
    // cout << waterInPuddles(heightsA) << endl; // 17
    return 0;
}
