#include <iostream>
#include <vector> 

#include "../gtp/board.hpp"

// for each i, j, perform the check

// board must be by reference so that we get the captures 
void check(Board &board, int grid_i, int grid_j) {
	// perform floodfill to get all tuples that are connected 
	// check all tuples for liberties 
	return; 
}

// this function checks for a valid boardsize 
bool valid_index(int boardsize, int idx) {
	// valid indices to the grid is from 0 to (boardsize - 1)
	if ( (idx < boardsize) && (idx >= 0) ) 
		return true; 
	return false; 
}

// implements the floodfill algorithm 
void floodfill(Board &board, int i, int j, int target_color, int replacement_color) {
	// if i,j is out of bounds for the board, then return 
	if ( !(valid_index(board.size, i) && valid_index(board.size, j)) ) 
		return; 
	// if the square is already filled, we don't need to fill it 
	if (target_color == replacement_color)
		return; 
	// if the square is some other color, then don't fill it  
	if (board.grid[i][j] != target_color)
		return; 
	board.grid[i][j] = replacement_color;

	// recursive calls
	floodfill(board, i, j - 1, target_color, replacement_color);
	floodfill(board, i, j + 1, target_color, replacement_color);
	floodfill(board, i - 1, j, target_color, replacement_color);
	floodfill(board, i + 1, j, target_color, replacement_color);
	return; 
}

int main() {
	// Board b; 
	// b.grid[5][5] = WHITE;
	return 0;
}
