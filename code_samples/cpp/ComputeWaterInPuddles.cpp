#include <iostream>
#include <iterator>
#include <array>
#include <cstdint>
#include <vector>
#include <list>
/*
 * Consider 1D dimensional puddles determined by array of integer heights
 * Compute maximal number of water which these puddles can hold
 *
 * Example: The single puddle on the relief below
 *          can hold 4 units of water marked 'W'
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
template <class T_container>
uintmax_t computeWaterIn(const T_container& heights)
{
    if (size(heights) < 3)
    {
        return 0;
    }
    auto left = begin(heights);
    auto right = rbegin(heights);
    auto lmax = *left;
    auto rmax = *right;
    uintmax_t water = 0;
    auto stepInto = [&water](auto& it, int& sideMax)
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
    vector<int> heightsV = { 1, 2, 5, 0, 4, 0 };
    cout << computeWaterIn(heightsV) << endl; // 4
    list<int> heightsL = { 0, 1, 3, -2, -3, 5 };
    cout << computeWaterIn(heightsL) << endl; // 11
    array<int, 15> heightsA = { 0, 1, 3, -2, -3, 5, 4, 2, 1, 0, 2, -1, -2, 0, -1 };
    cout << computeWaterIn(heightsA) << endl; // 17
    return 0;
}
