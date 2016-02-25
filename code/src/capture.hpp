#ifndef __CAPTURE_H__ 
#define __CAPTURE_H__

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
bool valid_index(int const boardsize, int const idx);

// returns the number of liberties found for the piece passed in.
int check_stone(Board const &board, tuple<int, int> const position);

// returns the total number of liberties for a given connected group  
int check_group(Board const &board, vector<tuple<int, int> > const &check_vec);

// implements the floodfill algorithm 
//
// this should NOT be by reference. this algorithm modifies the board by performing a 
// floodfill. we only want to obtain the list of stones in a particular group 
void floodfill(Board board,
			   vector<tuple<int, int> > &to_check, 
			   int i, 
			   int j, 
			   int target_color, 
			   int replacement_color);

// fills the board grid as EMPTY for every i,j tuple in the to_capture vector 
void remove_stones(Board &board, vector<tuple<int, int> > const &to_capture);

// this function checks a particular i, j position on the board and 
// removes the group at i, j if the group doesn't have at least 1 
// liberty. if board.grid[i][j] == EMPTY then this function returns 
// since we only check groups of stones for captures (obviously). 
void perform_capture(Board &board, int i, int j);

void perform_captures(Board &board);

/* 
original board 

goes through capture cycle for i, j
board is now modified

original board is set again
*/

// int main() {
// 	Board b; 

// 	// create a group of white stones 
// 	b.grid[5][5] = WHITE;
// 	b.grid[5][6] = WHITE;
// 	b.grid[5][7] = WHITE;
// 	b.grid[5][8] = WHITE;
// 	b.grid[5][9] = WHITE;
// 	b.grid[4][9] = WHITE;
// 	b.grid[3][9] = WHITE;
// 	b.grid[2][9] = WHITE;
// 	b.grid[1][9] = WHITE;
// 	b.grid[0][9] = WHITE;
// 	b.grid[4][5] = WHITE;
// 	b.grid[3][5] = WHITE;
// 	b.grid[2][5] = WHITE;
// 	b.grid[1][5] = WHITE;
// 	b.grid[0][5] = WHITE;

// 	// these black stones will be surrounded by white stones 
// 	b.grid[0][6] = BLACK;
// 	b.grid[0][7] = BLACK;
// 	b.grid[0][8] = BLACK;
// 	b.grid[1][6] = BLACK;
// 	b.grid[1][7] = BLACK;
// 	b.grid[1][8] = BLACK;
// 	b.grid[2][6] = BLACK;
// 	b.grid[2][7] = BLACK;
// 	b.grid[2][8] = BLACK;
// 	b.grid[3][6] = BLACK;
// 	b.grid[3][7] = BLACK;
// 	b.grid[3][8] = BLACK;
// 	b.grid[4][6] = BLACK;
// 	b.grid[4][7] = BLACK;
// 	b.grid[4][8] = BLACK;

// 	// floodfill(b, 0, 7, EMPTY, BLACK);
// 	// for (unsigned i = 0; i < to_check.size(); ++i) {
// 	// 	cout << get<0>(to_check[i]) << " " << get<1> (to_check[i]) << endl; 
// 	// }
// 	print_Board(b); 
// 	perform_captures(b);
// 	cout << endl; 
// 	print_Board(b); 
// 	// cout << "ayy lmao" << endl; 
// 	return 0;
// }

#endif 