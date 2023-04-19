#include <string>
#include <vector>
#include "words_handler.h"
#include "deep_solver.h"
#include "xen_solver.h"
#include "greedy_solver.h"
#include "static_solver.h"

int main() {
	vector<string> solutions = WordsHandler::solutions();
//	solutions.resize(10);
	//vector<string> guess = readWordFile("guesses5.txt");
	vector<string> guess = WordsHandler::guesses();

//	solutions = {"aaaaa", "baaaa", "caaaa", "daaaa", "eaaaa", "faaaa", "gaaaa", "haaaa", "iaaaa", "jaaaa", "kaaaa"
				//"abbaa", "accaa", "acbaa", "abcaa", "addaa", "adcaa", "adbaa", "acdaa", "abdaa"
//	};

	sort(solutions.begin(), solutions.end());
	guess = solutions;


//	XenSolver xen_solver = XenSolver();
//	xen_solver.solve(guess, solutions);

	DeepSolver deepSolver = DeepSolver();
//	deepSolver.solve(guess, solutions);

//	GreedySolver greedySolver = GreedySolver();
//	greedySolver.solve(guess, solutions);

//	StaticSolver staticSolver = StaticSolver();
//	staticSolver.solve(guess, solutions);

	StaticSolver2 staticSolver2 = StaticSolver2();
	staticSolver2.solve(guess, solutions);


	return 0;
}

// TODO zobaczyć jakie słowa są rozdzielane przez względnie mało pytań

/*
 *
 *
 */