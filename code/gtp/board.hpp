#ifndef __BOARD_H__
#define __BOARD_H__

#include <iostream>
#include <vector> 

using namespace std;

struct Board {
	int size;
	vector<vector<int> > grid;
	int captured_blk;
	int captured_wht;
	float komi;
	// time settings
	Board(int size);
	~Board();
};

#endif 
