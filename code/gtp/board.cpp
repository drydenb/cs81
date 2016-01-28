#include <iostream>

struct Board {
	int size;
	int **grid;
	int captured_blk;
	int captured_wht;
	float komi;
	// time settings
	Board(int size) {
		this->size = size;
		grid = new int[size][size];
		captured_blk = 0;
		captured_wht = 0;
		komi = 0.0;
	}
	~Board() {
		delete[] this->grid;
	}
};

int main() {
	Board b(19);
	for (int i = 0; i < b.size; ++i) {
		std::cout << b.grid[i][i] << std::endl;
	}
	return 0; 
}