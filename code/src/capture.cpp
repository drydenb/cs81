#include <iostream>
#include <vector> 
#include <tuple> 

#include "gtp.hpp" 
#include "board.hpp"

// these were defined in board.hpp:  
// EMPTY 0 
// BLACK 1 
// WHITE 2

#define BLACK_REPLACE 3
#define WHITE_REPLACE 4 

using namespace std; 

// vector<tuple<int, int> > to_check;
// vector<tuple<int, int> > to_capture; 
 
// valid indices on the grid are from 0 to (boardsize - 1)
bool valid_index(int const boardsize, int const idx) { 
	if ( (idx < boardsize) && (idx >= 0) )  
		return true; 
	return false; 
}

// returns the number of liberties found for the piece passed in.
int check_stone(Board const &board, tuple<int, int> const position) {
	
	// the number of liberties found for this piece 
	int stone_liberties = 0; 

	// we need to check the surrounding 4 positions for liberties (EMPTY)
	int x = get<0>(position);
	int y = get<1>(position); 

	// check up : (x, y + 1) 
	if (valid_index(board.size, y + 1)) {
		if (board.grid[x][y + 1] == EMPTY)
			++stone_liberties;
	}

	// check down : (x, y - 1) 
	if (valid_index(board.size, y - 1)) {
		if (board.grid[x][y - 1] == EMPTY) 
			++stone_liberties;
	}

	// check left : (x - 1, y) 
	if (valid_index(board.size, x - 1)) {
		if (board.grid[x - 1][y] == EMPTY) 
			++stone_liberties;
	}

	// check right : (x + 1, y)
	if (valid_index(board.size, x + 1)) {
		if (board.grid[x + 1][y] == EMPTY) 
			++stone_liberties;
	}

	// at this point, we have that the piece has liberties > 0 iff some EMPTY 
	// space was found in a surrounding position.

	return stone_liberties;  
}

// returns the total number of liberties for a given connected group  
int check_group(Board const &board, vector<tuple<int, int> > const &check_vec) {
	int group_liberties = 0; 
	for (unsigned i = 0; i < check_vec.size(); ++i) 
		group_liberties += check_stone(board, check_vec[i]);
	return group_liberties; 
}

// implements the floodfill algorithm 
//
// this should NOT be by reference. this algorithm modifies the board by performing a 
// floodfill. we only want to obtain the list of stones in a particular group 
void floodfill(Board board,
			   vector<tuple<int, int> > &to_check, 
			   int i, 
			   int j, 
			   int target_color, 
			   int replacement_color) { 
	// if i,j is out of bounds for the board, then return 
	if ( !(valid_index(board.size, i) && valid_index(board.size, j)) ) 
		return; 
	// this is a base case. if the target_color is the same as the replacement_color,
	// then we don't need to do anything  
	if (target_color == replacement_color)
		return; 
	// if the square is some other color, then don't fill it  
	if (board.grid[i][j] != target_color)
		return; 

	// fill in the grid and then add the i, j position to the return vector 
	board.grid[i][j] = replacement_color;
	to_check.push_back(tuple<int, int> (i, j)); 

	// recursive calls. the vector should be mutated across all recursive calls 
	// to produce the total list of stones in one group 
	floodfill(board, to_check, i, j - 1, target_color, replacement_color);
	floodfill(board, to_check, i, j + 1, target_color, replacement_color);
	floodfill(board, to_check, i - 1, j, target_color, replacement_color);
	floodfill(board, to_check, i + 1, j, target_color, replacement_color);
	return; 
}

// fills the board grid as EMPTY for every i,j tuple in the to_capture vector 
void remove_stones(Board &board, vector<tuple<int, int> > const &to_capture) {
	// go through the list of positions and fill in the grid as empty
	for (unsigned i = 0; i < to_capture.size(); ++i) {
		int x = get<0>(to_capture[i]);
		int y = get<1>(to_capture[i]);
		board.grid[x][y] = EMPTY; 
	}
	return; 
}

// this function checks a particular i, j position on the board and 
// removes the group at i, j if the group doesn't have at least 1 
// liberty. if board.grid[i][j] == EMPTY then this function returns 
// since we only check groups of stones for captures (obviously). 
void perform_capture(Board &board, int i, int j) {
	
	// used for floodfill  
	int target_color;  
	int replacement_color; 

	// check this group for liberties 
	vector<tuple<int, int> > to_check;

	// base case : the grid is empty at this position 
	if (board.grid[i][j] == EMPTY) {
		return; 
	} else {
		// set the target color 
		target_color = board.grid[i][j]; 
	}

	// set the replacement color 
	if (target_color == BLACK) 
		replacement_color = BLACK_REPLACE;
	if (target_color == WHITE)
		replacement_color = WHITE_REPLACE; 

	// now that we have the color of the grid at i, j, we perform a floodfill 
	// and retrieve the group of stones using to_check
	floodfill(board, to_check, i, j, target_color, replacement_color);

	// at this point, to_check should contain the vector of tuples of stones we need to 
	// check for liberties > 0 
	int group_liberties = check_group(board, to_check);

	if (group_liberties == 0) {
		remove_stones(board, to_check); 
	}

	return; 
}

void perform_captures(Board &board) {
	// this may not be the most efficient implementation, but this 
	// should work for now. loop through all positions of the board, 
	// detect a group of stones and check those stones for liberties. 
	// if no liberties are found for the group, remove them from the 
	// board. otherwise, do nothing. 
	for (int i = 0; i < board.size; ++i) {
		for (int j = 0; j < board.size; ++j) {
			perform_capture(board, i, j); 
		}
	}

	return; 
}

/* 
original board 

goes through capture cycle for i, j
board is now modified

original board is set again
*/

int main() {
	Board b; 

	// create a group of white stones 
	b.grid[5][5] = WHITE;
	b.grid[5][6] = WHITE;
	b.grid[5][7] = WHITE;
	b.grid[5][8] = WHITE;
	b.grid[5][9] = WHITE;
	b.grid[4][9] = WHITE;
	b.grid[3][9] = WHITE;
	b.grid[2][9] = WHITE;
	b.grid[1][9] = WHITE;
	b.grid[0][9] = WHITE;
	b.grid[4][5] = WHITE;
	b.grid[3][5] = WHITE;
	b.grid[2][5] = WHITE;
	b.grid[1][5] = WHITE;
	b.grid[0][5] = WHITE;

	// these black stones will be surrounded by white stones 
	b.grid[0][6] = BLACK;
	b.grid[0][7] = BLACK;
	b.grid[0][8] = BLACK;
	b.grid[1][6] = BLACK;
	b.grid[1][7] = BLACK;
	b.grid[1][8] = BLACK;
	b.grid[2][6] = BLACK;
	b.grid[2][7] = BLACK;
	b.grid[2][8] = BLACK;
	b.grid[3][6] = BLACK;
	b.grid[3][7] = BLACK;
	b.grid[3][8] = BLACK;
	b.grid[4][6] = BLACK;
	b.grid[4][7] = BLACK;
	b.grid[4][8] = BLACK;

	// floodfill(b, 0, 7, EMPTY, BLACK);
	// for (unsigned i = 0; i < to_check.size(); ++i) {
	// 	cout << get<0>(to_check[i]) << " " << get<1> (to_check[i]) << endl; 
	// }
	print_Board(b); 
	perform_captures(b);
	cout << endl; 
	print_Board(b); 
	// cout << "ayy lmao" << endl; 
	return 0;
}
