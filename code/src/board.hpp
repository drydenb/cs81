#ifndef __BOARD_H__
#define __BOARD_H__

#include <iostream>
#include <vector> 

#define EMPTY 0 
#define BLACK 1 
#define WHITE 2

using namespace std;

struct Board {
	int size;
	vector<vector<int> > grid;
	int captured_blk;
	int captured_wht;
	vector<string> move_history; 
	float komi;
	// time settings
	Board(); 
	Board(int size);
	~Board();
};

#endif 
