#ifndef __BOARD_H__
#define __BOARD_H__

#include <iostream>
#include <vector> 
#include <deque> 

#define EMPTY 0 
#define BLACK 1 
#define WHITE 2

using namespace std;

struct Board {

	// members 
	int size;
	vector<vector<int> > grid;
	int captured_blk;
	int captured_wht;
	vector<vector<vector<int>> > move_history;  
	float komi;
	int just_moved;

	/* include time settings? */ 

	// constructors, destructors, mutators, etc. 
	Board(); 
	Board(int size);
	~Board();
};

#endif 
