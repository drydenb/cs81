// string::begin/end
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main ()
{
  vector<vector<int> > grid = vector<vector<int> >(10, vector<int>(10, 0));
  grid[5][5] = 10; 
  for (int i = 0; i < 10; ++i) {
  	for (int j = 0; j < 10; ++j) {
  		cout << grid[i][j] << " "; 
  	}
  	cout << endl; 
  }

  vector<vector<int> > tentative = grid;
  for (int i = 0; i < 10; ++i) {
  	for (int j = 0; j < 10; ++j) {
  		cout << tentative[i][j] << " "; 
  	}
  	cout << endl; 
  }
  return 0;
}