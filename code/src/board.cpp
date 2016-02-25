#include <iostream>
#include "board.hpp"

#define DEFAULT_BOARDSIZE 19

// default no argument constructor 
Board::Board() {
	size = DEFAULT_BOARDSIZE; 
	grid = vector<vector<int> >(size, vector<int>(size, 0));
	captured_blk = 0;
	captured_wht = 0;
	komi = 0.0;
	just_moved = 0; // initially, no one has moved 
}

// constructor with boardsize argument 
Board::Board(int s) {
	size = s; 
	grid = vector<vector<int> >(size, vector<int>(size, 0));
	captured_blk = 0;
	captured_wht = 0;
	komi = 0.0;
}

Board::~Board() {
	; 
}

// int main() {
// 	Board b(19);
// 	for (int i = 0; i < b.size; ++i) {
// 		for (int j = 0; j < b.size; ++j) {
// 			std::cout << b.grid[i][j] << " ";
// 		}
// 		std::cout << endl; 
// 	}
// 	cout << b.size << endl; 
// 	cout << b.captured_wht << endl;
// 	cout << b.captured_blk << endl; 
// 	return 0; 
// }