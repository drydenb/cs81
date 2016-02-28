#ifndef __GTP_H__ 
#define __GTP_H__

#include <string> 
#include "board.hpp"
using namespace std; 

/// 
/// \brief Contains all information relevant to a GTP style command.
///
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

// ////////////////////////////////////////////////////////////////////////////////
// // GLOBALS
// ////////////////////////////////////////////////////////////////////////////////

// // an unordered map containing all supported GTP commands as keys and the number
// // of arguments expected for each command as values 
// unordered_map<string, int> supported_commands {
// 	// { command_name, number of arguments } 
// 	{"protocol_version", 0},
// 	{"name", 0},
// 	{"version", 0},
// 	{"known_command", 1},
// 	{"list_commands", 0},
// 	{"quit", 0},
// 	{"boardsize", 1},
// 	{"clear_board", 0},
// 	{"komi", 1},
// 	{"play", 2},
// 	{"genmove", 1}
// };

// // define a mapping from each letter in the modified alphabet (doesn't include
// // the letter 'i') to an index in the board 
// string alphabet("abcdefghjklmnopqrstuvwxyz");   
// // letter from GUI --> index for internal grid representation 
// unordered_map<char, int> alphabet_to_index;
// // index from internal grid representation --> letter to GUI 
// unordered_map<int, char> index_to_alphabet; 

// // a Board struct instance 
// Board board;

// initializes global maps
void gtp_init();

////////////////////////////////////////////////////////////////////////////////
// DEBUGGING 
////////////////////////////////////////////////////////////////////////////////

// pretty prints a GTP_Command  
void debug_Print_GTP_Command(GTP_Command const &cmd); 

void debug_Print_Board(Board const &board); 

////////////////////////////////////////////////////////////////////////////////
// INPUT / OUTPUT PROCESSING 
////////////////////////////////////////////////////////////////////////////////

// this function preprocesses an input command to the engine  
string gtp_preprocess(string input);
// valid forms are:
// 		id command_name arguments\n
// 		id command_name\n
// 		command_name arguments\n
// 		command_name\n
// we only need to check if the input is in the form:
// id command_name
// or 
// command_name
GTP_Command gtp_parse_command(string input);

// wraps a response.
// this function wraps using '=' GTP syntax is the command is successful,
// and wraps with '?' syntax otherwise 
void gtp_process_command(GTP_Command &cmd);

// this sends the response from the engine to stdout 
void gtp_respond(GTP_Command &cmd);

// this code implements GTP Protocol Version 2 
void gtp_protocol_version(GTP_Command &cmd); 

// prints out the name of this engine. this doesn't matter -- just pick "Izumi"
void gtp_name(GTP_Command &cmd); 

// prints out the version number for this engine. since no sense of version 
// return the empty string (compliant with GTP) 
void gtp_version(GTP_Command &cmd); 

// checks if the command given is supported 
void gtp_known_command(GTP_Command &cmd); 

// lists all the commands supported by the engine 
void gtp_list_commands(GTP_Command &cmd); 

// exits with success 
void gtp_quit(GTP_Command &cmd); 

void gtp_boardsize(GTP_Command &cmd); 

void gtp_clear_board(GTP_Command &cmd);

void gtp_komi(GTP_Command &cmd); 

tuple<int, int> parse_move(string move, bool &flag); 

void gtp_play(GTP_Command &cmd);

void gtp_genmove(GTP_Command &cmd);

// this function calls the appropriate GTP command for a given instance 
void gtp_dispatch(GTP_Command &cmd);


#endif 