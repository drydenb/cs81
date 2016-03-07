#ifndef __CAPTURE_H__ 
#define __CAPTURE_H__

#include "gtp.hpp" 
#include "board.hpp"

#define REPLACE       5  
#define DEBUG         0 

using namespace std; 

void valid_board_representation(Board const &board);
bool valid_index(int boardsize, int idx);
int check_stone(Board const &board, tuple<int, int> const &position);
int check_group(Board const &board, vector<tuple<int, int> > const &check_vec);
void floodfill(Board board,
			   vector<tuple<int, int> > &to_check, 
			   int i, 
			   int j, 
			   int target_color, 
			   int replacement_color);
void remove_stones(Board &board, vector<tuple<int, int> > const &to_capture);
bool check_capture(Board const &board, int capture_color, int i, int j);
bool check_captures(Board const &board, int capture_color);
void perform_capture(Board &board, int capture_color, int i, int j);
void perform_captures(Board &board);

#endif 