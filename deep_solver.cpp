//
// Created by michal on 16.03.23.
//

#include "deep_solver.h"

void DeepSolver::solve(vector<string> &solutions, vector<string> guess) {
	wordSize = solutions[0].size();
	answerSize = 1;
	for (int i = 0; i < wordSize; i++)
		answerSize *= 3;

	vector<int> solution_ids = getSolutionsId(solutions, guess);

	int nGuess = int(guess.size());
	int nSolution = int(solution_ids.size());

	vector< vector< vector< int > > > partitions(nGuess);   // [nguess][243][nsol]
	vector< vector< Response > > solutionClue(nGuess);	// [nguess][nguess] -> clue

//		int nThread = clamp(int(thread::hardware_concurrency()), 2, 32);
	int nThread = clamp(int(thread::hardware_concurrency()), 1, 1); //TODO zwiększyć

	solve_clues(guess, solution_ids, nGuess, nSolution, partitions, solutionClue, nThread);

	vector<int> index(nGuess);
	iota(index.begin(), index.end(), 0);

	vector<int> favs = index;
	vector<Path> favResults(favs.size());
	explore(guess, solution_ids, solutionClue, nThread, favs, favResults);

	for (int ifav = 0; ifav < int(favs.size()); ++ifav) {
		cout << guess[favs[ifav]] << " " << right << setw(6) << favResults[ifav].maxDepth << endl;
	}

//		cout << mapa.size() << " " << map_id << "\wordSize";

	write_worst_paths_to_file(guess, favResults);

//		ofstream twoDeepFile;
//		twoDeepFile.open(std::filesystem::path(string("../results/summary_two-deep.csv")));
//		twoDeepFile << "first guess,avg guesses,max guesses,first clue,wordSize solutions,second guess,max guesses remaining,solutions" << endl;
//		for (auto & s : wordstreams) {
//			twoDeepFile << s.str();
//		}
//		twoDeepFile.close();

	cout << endl;
}

void DeepSolver::solve_clues(const vector<string> &guess, const vector<int> &solution_ids, int nGuess, int nSolution,
				 vector< vector< vector< int > > > &partitions, vector< vector< Response > > &solutionClue,
				 const int &nThread) {
	auto t0 = std::chrono::system_clock::now();
	vector<thread> threads;

	for (int it = 0; it < nThread; ++it) {
		threads.emplace_back([this, it, nGuess, nThread, nSolution, &guess, &solution_ids, &partitions, &solutionClue]() {
			for (int ig = it * nGuess / nThread; ig < (it + 1) * nGuess / nThread; ++ig) {
				partitions[ig].resize(answerSize);
				solutionClue[ig].resize(nGuess);

				for (int is = 0; is < nSolution; ++is) {
					Response response = getResponse(guess[ig], guess[solution_ids[is]]);
					partitions[ig][response].push_back(solution_ids[is]);
					solutionClue[ig][solution_ids[is]] = response;
				}
			}
		} );
	}
	for (auto &t : threads)
		t.join();
	cout << fixed << setprecision(3) << "solve_clues time " << " " << since(t0) << endl;
}

Path DeepSolver::exploreGuess(vector<int> &solution, int g, vector<vector<Response>> &solutionClue, vector<string> &guess,
							  DeadLetters &dl) {
	Path result;
	int nGuess = int(guess.size());

	// Jakie hasła by dostały konkretną odpowiedź gdybyśmy zapytali o słowo o indeksie g
	vector<vector<int>> parts = splitIntoParts(solution, solutionClue[g]);

	for (int ip = 0; ip < answerSize; ++ip) {
		const vector<int> &sols = parts[ip];
		// Dane zapytanie g w ogóle niczego nie zmienia bo po podziale dostajemy dokładnie taki samy zbiór możliwych haseł
		if (solution.size() == sols.size()) {
			result.maxDepth = UNSOLVABLE;
			return result;
		}
	}

	// Sprawdzamy każdą możliwą odpowiedź wyroczni
	for (int ip = 0; ip < answerSize; ++ip) {
		// Lista haseł pasujące do danej odpowiedzi
		vector<int> &sols = parts[ip];
		if (sols.empty())
			continue;

		// Liczba pasujących haseł, jeżeli dostaniemy odpowiedź `ip`
		int nipSol = int(sols.size());
		Path best;
		// Jest tylko jedna możliwa odpowiedź ale nie jest to słowo `g`, bo nie dostaliśmy odpowiedzi 242
		if (nipSol == 1) {
			best.maxDepth = 1;
			best.solution.emplace_back(sols[0]);
		} else if (nipSol == 2) {
			// Mamy dwie możliwość, więc no lepiej się nie da
			best.maxDepth = 2;
			best.solution.emplace_back(sols[1]);
			best.solution.emplace_back(sols[0]);
		} else {
			// TODO posortować po maksymalnym poddrzewie zapytania
			// Nowy stan martwych liter, jeżeli zgadujemy hasło `guess[g]`, dostaliśmy odpowiedź `ip` i początkowo mieliśmy stan `deadLetters`

			vector<pair<int, int>> maxPart;
			for (int ig = 0; ig < nGuess; ++ig) {
				maxPart.emplace_back(int(splitIntoPartsMax(sols, solutionClue[ig])), int(ig));
			}
			sort(maxPart.begin(), maxPart.end());

			DeadLetters dl2 = dl;
			dl2.addResponse(guess[g], Response(wordSize, ip));

			best.maxDepth = UNSOLVABLE;
//			best = exploreGuess(sols, maxPart[0].second, solutionClue, guess, dl2, best.maxDepth - 1, true);

			for (int i = 0; i < nGuess; ++i) {
				if (maxPart[i].second != maxPart[0].second)
					break;
				Path p = exploreGuess(sols, maxPart[i].second, solutionClue, guess, dl2);

//				Path p = exploreGuess(sols, i, solutionClue, guess, dl2, best.maxDepth - 1, true);

				if (p.maxDepth < best.maxDepth)
					best = p;

				break;
			}
		}

		if (best.maxDepth > result.maxDepth || result.maxDepth == UNSOLVABLE)
			result = best;
	}

	result.solution.emplace_back(g);
	result.maxDepth += 1;

	return result;
}

void DeepSolver::write_worst_paths_to_file(vector<string> &guess, vector<Path> &favResults) {
	ofstream summFile;

	summFile.open(filesystem::path(string("../results/summary_ev.csv")));
	summFile << "guess,max guesses,solution\n";
	for (int i = 0; i < (int)favResults.size(); i++) {
		summFile << guess[i] << "," << favResults[i].maxDepth << ",";

		for (int j = (int)favResults[i].solution.size() - 1; j >= 0; j--) {
			summFile << guess[ favResults[i].solution[j] ];

			if (j != 0)
				summFile << " ";
		}

		summFile << "\n";
	}

	summFile.close();
}

void DeepSolver::explore(vector<string> &guess, vector<int> &solution_ids, vector<vector<Response>> &solutionClue,
			 int nThread, vector<int> &favs, vector<Path> &favResults)  {
	vector<thread> ethreads;
	atomic<int> next = 0;
	atomic<int> nDone = 0;
	int nFav = int(favs.size());
	auto te0 = std::chrono::system_clock::now();

	cout << "  wątek  id  słowo      max\n";

	for (int it = 0; it < nThread; ++it) {
		ethreads.emplace_back([this, it, te0, &nFav, &nDone, &next, &favs, &guess, &solution_ids, &solutionClue, &favResults]() {
			for (int ifav = next++; ifav < nFav; ifav = next++) {
				auto tf0 = std::chrono::system_clock::now();

				DeadLetters dl(wordSize);
				Path path = exploreGuess(solution_ids, favs[ifav], solutionClue, guess, dl);

				favResults[ifav] = path;
				nDone++;

				double dt = since(te0);
				double done = double(nDone) / nFav;
				double rem = dt / done - dt;

				cout << setw(5) << it << " " << " " << setw(4) << ifav << "  " << guess[favs[ifav]] << "  "
					 << fixed << right << setw(6) << favResults[ifav].maxDepth << " "
					 << setw(7) << setprecision(2) << since(tf0) / 60 << " min     "
					 << setw(5) << setprecision(1) << done * 100 << "% done  "
					 << setw(5) << setprecision(0) << rem / 60 << " min to go" << endl;

				// *** "results" directory needs to already exist ***

				// the entire decision tree for this starting word
//					ofstream treefile;
//					treefile.open(std::filesystem::path(string("results/tree ") + guess[favs[ifav]] + ".txt"));
//					showPath(treefile, "", path, guess, solutionClue);
//					treefile.close();

//					showTable(wordstreams[ifav], path, guess);
//
//					summstreams[it] << guess[favs[ifav]] << "," << fixed << "," << path.maxDepth << "," << since(tf) << endl;
			}
		} );
	}
	for (auto & t : ethreads)
		t.join();
}

vector<int> DeepSolver::splitIntoPartsCount(vector<int> &solutions, vector<Response> &solutionClue) {
	vector<int> count(answerSize);

	for (int sol : solutions)
		count[solutionClue[sol]] += 1;

	return count;
}

vector<vector<int>> DeepSolver::splitIntoParts(vector<int> &solutions, vector<Response> &solutionClue) {
	vector<int> count = splitIntoPartsCount(solutions, solutionClue);
	vector<vector<int>> parts(answerSize);

	for (int i = 0; i < answerSize; ++i)
		parts[i].reserve(count[i]);

	for (int sol : solutions)
		parts[solutionClue[sol]].push_back(sol);

	return parts;
}

int DeepSolver::splitIntoPartsMax(vector<int> &solutions, vector<Response> &solutionClue) {
	vector<int> count = splitIntoPartsCount(solutions, solutionClue);

	return *max_element(count.begin(), count.end());
}