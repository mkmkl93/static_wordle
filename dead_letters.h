//
// Created by michal on 12.03.23.
//

#ifndef WORDLE_DEAD_LETTERS_H
#define WORDLE_DEAD_LETTERS_H

#include <bitset>
#include "response.h"
#include <cstring>

using namespace std;

struct DeadLetters {
	vector< bitset<26> > dead;
	vector<char> known;

	DeadLetters() { dead.resize(WORD_SIZE); };

	void killOne(int i, char letter);

	void killAllButOne(int i, char letter);

	void addResponse(string w, Response r);

	void unionKnown(vector<char> v_known);

	bool matches(string s);

	bool operator<(const DeadLetters& dl) const;
};

#endif //WORDLE_DEAD_LETTERS_H
