#include <iostream>

#include <vector> 
#include <tuple> 
#include <cassert> 

#include "capture.hpp" 
#include "gtp.hpp" 
#include "board.hpp"

using namespace std; 




////////////////////////////////////////////////////////////////////////////////
// GLOBALS 
////////////////////////////////////////////////////////////////////////////////

// 

////////////////////////////////////////////////////////////////////////////////
// DEBUGGING FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

///
/// \brief Debugging function that checks if the board only contains EMPTY, 
///        BLACK, or WHITE
/// 
void valid_board_representation(Board const &board) {
	bool valid = true;
	for (int i = 0; i < board.size; ++i) {
		for (int j = 0; j < board.size; ++j) {
			if (    (board.grid[i][j] != BLACK) 
				 || (board.grid[i][j] != WHITE)
				 || (board.grid[i][j] != EMPTY) ) {
				valid = false;
			} 
		}
	}
	if (!valid) {
		cerr << "Board representation is not valid!" << endl; 
		exit(EXIT_FAILURE);
	} 
	return; 
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION DEFINITIONS   
////////////////////////////////////////////////////////////////////////////////

///
/// \brief Returns a boolean indicating whether the input idx is valid for the 
///        given boardsize. 
/// Valid indices on the grid are from 0 to (boardsize - 1)
///
bool valid_index(int boardsize, int idx) { 
	if ( (idx < boardsize) && (idx >= 0) )  
		return true; 
	return false; 
}

///
/// \brief Returns the number of liberties found for the piece passed in. 
/// This function only checks for EMPTY spaces in neighboring legal positions.
/// Whether or not the space is occupied by a BLACK or WHITE piece is not 
/// considered by this function.
/// 
int check_stone(Board const &board, tuple<int, int> const &position) {
	
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

///
/// \brief Returns the total number of liberties for a given connected group.
/// Once again, this function only considers how any EMPTY spaces occur for 
/// for the entire group (this is the number of liberties). 
///
int check_group(Board const &board, vector<tuple<int, int> > const &check_vec) {
	int group_liberties = 0; 
	for (unsigned i = 0; i < check_vec.size(); ++i) 
		group_liberties += check_stone(board, check_vec[i]);
	return group_liberties; 
}

///
/// \brief Implements the floodfill algorithm, pushing floodfilled elements into
///        the passed in to_check vector. 
/// Board should not be by reference. This algorithm modifies passed-by-value
/// board by  performing a floodfill. we only want to obtain the list of stones 
/// in a particular group so we should not modify the board here. 
///
void floodfill(Board board,
			   vector<tuple<int, int> > &to_check, 
			   int i, 
			   int j, 
			   int target_color, 
			   int replacement_color) { 
	// if i,j is out of bounds for the board, then return 
	if ( !(valid_index(board.size, i) && valid_index(board.size, j)) ) 
		return; 
	// this is a base case. if the target_color is the same as the 
	// replacement_color, then we don't need to do anything  
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

///
/// \brief Fills the board grid as EMPTY for every i,j tuple in the to_capture 
///        vector.
/// 
void remove_stones(Board &board, vector<tuple<int, int> > const &to_capture) {
	// go through the list of positions and fill in the grid as empty
	for (unsigned i = 0; i < to_capture.size(); ++i) {
		int x = get<0>(to_capture[i]);
		int y = get<1>(to_capture[i]);
		board.grid[x][y] = EMPTY; 
	}
	return; 
}

///
/// \brief Returns a boolean for whether or not captures can occur on the passed
///        in color on the group located at i,j.
///
bool check_capture(Board const &board, int capture_color, int i, int j) {

	bool can_capture = false;             // flag for captures 
	int replacement_color = REPLACE;      // used for floodfill
	vector<tuple<int, int> > to_check;    // check this group for liberties 

	// if board.grid[i][j] doesn't contain the target color, return  
	if ((board.grid[i][j] == EMPTY) || (board.grid[i][j] != capture_color)) {
		assert (can_capture == false); 
		return can_capture; 
	} 

	// board.grid[i][j] should now be the color we're trying to capture 
	assert (board.grid[i][j] == capture_color); 

	// now that we have the color of the grid at i, j, we perform a floodfill 
	// and retrieve the group of stones using to_check. note that this should
	// not modify board since it is passed by value (this may need to be 
	// optimized later). 
	floodfill(board, to_check, i, j, capture_color, replacement_color);

	// at this point, to_check should contain the vector of tuples of stones we 
	// need to check for liberties > 0 
	int group_liberties = check_group(board, to_check);

	if (group_liberties == 0) {
		can_capture = true; 		
	}
	return can_capture; 
}

///
/// \brief Returns a boolean if captures can occur anywhere on the board for the
///        passed in color.
/// 
bool check_captures(Board const &board, int capture_color) {
	bool can_capture = false; 
	for (int i = 0; i < board.size; ++i) {
		for (int j = 0; j < board.size; ++j) {
			// check_capture returns true if a capture can occur for the group 
			// at i,j, and false otherwise. 
			can_capture = check_capture(board, capture_color, i, j);
		}
	}
	return can_capture; 
}

///
/// \brief Removes the group at i,j. Assumes that this group has been checked 
///        for no liberties. This function will remove the group or have an 
///        assertion failure.  
/// 
void perform_capture(Board &board, int capture_color, int i, int j) {

	int replacement_color = REPLACE;       // used for floodfill
	vector<tuple<int, int> > to_remove;    // check this group for liberties 

	// board.grid[i][j] be the color we're trying to capture 
	assert (board.grid[i][j] == capture_color); 

	// perform a floodfill and retrieve the group of stones using to_remove. 
	// note that this should not modify board since it is passed by value (this 
	// may need to be optimized later). 
	floodfill(board, to_remove, i, j, capture_color, replacement_color);

	// at this point, to_remove should contain the vector of tuples of stones we 
	// need to check for liberties > 0 
	int group_liberties = check_group(board, to_remove);
	assert (group_liberties == 0);
	remove_stones(board, to_remove);
	return;
}

///
/// \brief Performs captures if they exist for the color opposite to the one 
///        just played (i.e., if black just moved, then we check for captures 
///        on white). 
///
void perform_captures(Board &board) {

	// this implementation will probably need to be optmized later. this is 
	// just too many pass-by-values to be fast enough i think. for the moment
	// i am taking care of the cases with goto. this will also need to be 
	// changed in the long run because it is poor style. 

	assert ( (board.just_moved == BLACK) || (board.just_moved == WHITE) ); 

	if (board.just_moved == BLACK) 
		goto black_moved;
	else 
		goto white_moved; 

	black_moved: 
	if (board.just_moved == BLACK) {
		for (int i = 0; i < board.size; ++i) {
			for (int j = 0; j < board.size; ++j) {
				if (check_capture(board, WHITE, i, j)) {
					perform_capture(board, WHITE, i, j); 
				}
			}
		}
	}
	return; 

	white_moved: 
	if (board.just_moved == WHITE) {
		for (int i = 0; i < board.size; ++i) {
			for (int j = 0; j < board.size; ++j) {
				if (check_capture(board, BLACK, i, j)) {
					perform_capture(board, BLACK, i, j); 
				}
			}
		}
	}
	return;
}

// for testing floodfill:

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
