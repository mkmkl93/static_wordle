//
// Created by michal on 16.03.23.
//

#ifndef WORDLE_UTILS_H
#define WORDLE_UTILS_H

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <chrono>
#include "response.h"
using namespace std;

// For every word in `solutions` get its id in `guess`
vector<int> getSolutionsId(vector<string> &solutions, vector<string> &guess);

// Given a guess and the solution, return the response
// 0 == no match, 1 == right letter, wrong place, 2 == right letter in right place
// Using Wordle rules for duplicate letters, which are not treated the same
Response getResponse(string guess, string solution);

double timeDiff(const chrono::time_point<chrono::system_clock> &t0, const chrono::time_point<chrono::system_clock> &t1);

double since(const chrono::time_point<chrono::system_clock> &t0);

#endif //WORDLE_UTILS_H
