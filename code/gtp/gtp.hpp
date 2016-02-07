#ifndef __GTP_H__ 
#define __GTP_H__

#include <string> 
using namespace std; 

// a struct containing all necessary information 
// in a GTP command from a controller  
typedef struct GTP_Command {
	int id;
	bool has_id; 

	string command_name;
	vector<string> args;
	
	bool error_flag;
	string response;   
	
	GTP_Command () {
		id = 0; 
		has_id = false; 
		error_flag = false;
		response = string(""); 
	}

} GTP_Command;

#endif 