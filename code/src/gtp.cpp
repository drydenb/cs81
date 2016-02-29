#include <iostream>
#include <sstream> 
#include <fstream> 

#include <algorithm>
#include <tuple> 
#include <unordered_map>

#include <random> 
#include <cstdlib> 

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp> 

#include "capture.hpp" 
#include "gtp.hpp" 
#include "board.hpp"

#define SEED 10 

using namespace std;




////////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////////

// an unordered map containing all supported GTP commands as keys and the number
// of arguments expected for each command as values 
unordered_map<string, int> supported_commands {
	// { command_name, number of arguments } 
	{"protocol_version", 0},
	{"name", 0},
	{"version", 0},
	{"known_command", 1},
	{"list_commands", 0},
	{"quit", 0},
	{"boardsize", 1},
	{"clear_board", 0},
	{"komi", 1},
	{"play", 2},
	{"genmove", 1}
};

// define a mapping from each letter in the modified alphabet (doesn't include
// the letter 'i') to an index in the board 
string alphabet("abcdefghjklmnopqrstuvwxyz");   

// letter from GUI --> index for internal grid representation 
unordered_map<char, int> alphabet_to_index;

// index from internal grid representation --> letter to GUI 
unordered_map<int, char> index_to_alphabet; 

// this is the global board instance
Board board;

// the logfile. this is used for debugging purposes 
ofstream logfile;   




////////////////////////////////////////////////////////////////////////////////
// DEBUGGING FUNCTIONS 
////////////////////////////////////////////////////////////////////////////////

/// 
/// \brief Prints out the contents of an unordered_map<S, T> to the logfile.  
///
template<typename S, typename T> 
void debug_Print_Map(unordered_map<S, T> const &map) {
	for (auto it = map.begin(); it != map.end(); ++it) {
		logfile << it->first << "," << it->second << ";" << endl; 
	}
	return; 
}

///
/// \brief Prints out the contents of a GTP_Command struct to the logfile.
///    
void debug_Print_GTP_Command(GTP_Command const &cmd) {
	cerr << "ID: " << cmd.id << endl; 
	cerr << "HAS ID: " << cmd.has_id << endl;
	cerr << "COMMAND NAME: \'" << cmd.command_name << "\'" << endl; 
	for (unsigned i = 0; i < cmd.args.size(); ++i) 
		cerr << "ARG " << i << ": \'" << cmd.args[i] << "\'" << endl; 
	cerr << "FLAG: " << cmd.error_flag << endl; 
	cerr << "RESPONSE: \'" << cmd.response << "\'" << endl;  
	return; 
}

///
/// \brief Prints out a ASCII representation of the board to the logfile. 
/// In the print out, 0 represents empty, 1 represents black, and 2 represents
/// white. The board is also labeled with its board number, which is the number
/// of boards printed out to the logfile so far. 
/// 
void debug_Print_Board(Board const &board) {
	// use static int to keep track of how many times we print out a board 
	static int board_number = 0; 
	++board_number;
	logfile << "Board Number: " << board_number << endl;  
	for (unsigned i = 0; i < board.grid.size(); ++i) {
		for (unsigned j = 0; j < board.grid.size(); ++j) {
			logfile << board.grid[i][j] << " ";
		}
		logfile << endl; 
	}
	logfile << endl; 

	return; 
}




////////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS 
////////////////////////////////////////////////////////////////////////////////

/// 
/// \brief Intializes global maps etc. necessary for the GTP communication.
///
void init() {

	// create a logfile for recording moves made this game 
	remove("logfile.txt"); 
	logfile.open("logfile.txt");
	if (!logfile.is_open()) {
		cout << "Could not generate logfile!" << endl; 
		exit(EXIT_FAILURE);
	}

	// initialize both unordered maps 
	int idx = 0;
	BOOST_FOREACH (char c, alphabet) {
		pair<char ,int> letter_coord (c, idx);
		alphabet_to_index.insert(letter_coord);
		++idx; 
	}
	idx = 0;
	BOOST_FOREACH (char c, alphabet) {
		pair<int, char> coord_letter (idx, c);
		index_to_alphabet.insert(coord_letter);
		++idx; 
	}
	return; 
}

///
/// \brief Converts from the internal representation's coordinates to GTP's 
///        external coordinates. 
/// The internal representation is flipped on the vertical axis with respect 
/// to the external representation. This is mainly because GTP defines its 
/// boord coordinates' origin A1 in the lower-left, whereas the internal 
/// representation's origin 0,0 is in the upper-left. Optimal future moves are
/// invariant upon rotations or reflections of the board, so this seems okay. 
/// Thus (0,0) --> ('A', 1), (0, 18) --> ('A', 19), (18, 0) --> ('T', 1), 
/// and (18, 18) --> ('T', 19), with natural extensions for larger boards. 
///   
tuple<char, int> internal_to_external(tuple<int, int> const &coord) {
	// we want to map the first element of the tuple to its corresponding 
	// character using the index_to_alphabet map. 
	unordered_map<int, char>::const_iterator it = 
		index_to_alphabet.find(get<0>(coord));
	assert (it != index_to_alphabet.end()); 
	char letter = it->second;         // this is lowercase, OK by protocol 
	int index = get<1>(coord) + 1;    // external rep. is 1-indexed
	return tuple<char, int> (letter, index);
}

///
/// \brief Converts from GTP's external coordinates to the internal 
///        representation's coordinates.
/// See internal_to_external for a more detailed description. This function 
/// performs the reverse mapping.
/// 
tuple<int, int> external_to_internal(tuple<char, int> const &coord) {
	// map the character to its index in the internal rep. 
	unordered_map<char, int>::const_iterator it = 
		alphabet_to_index.find(get<0>(coord));
	assert (it != alphabet_to_index.end());
	int x_idx = it->second; 
	int y_idx = get<1>(coord) - 1;    // internal rep. is 0-indexed
	assert (y_idx >= 0);
	return tuple<int, int> (x_idx, y_idx); 
}




////////////////////////////////////////////////////////////////////////////////
// INPUT / OUTPUT PROCESSING 
////////////////////////////////////////////////////////////////////////////////

/// 
/// gtp_prepross function preprocesses an input command to the engine  
///
string gtp_preprocess(string input) {
	
	// REMOVE CONTROL CHARACTERS // 
	// remove all control characters ASCII 0-31 and 127, except for HT (dec 9)
	// and LF (dec 10)
	for (string::iterator it = input.begin(); it != input.end(); ++it) { 
		int ascii_code = static_cast<int>(*it);    // get the ascii code
		if ( (ascii_code <= 31) || (ascii_code == 127) ) {
			if ( (ascii_code != 9) && (ascii_code != 10) ) 
				input.erase(it); 
		}
	}
	
	// REMOVE COMMENTS // 
	// remove all comments of the form: "# this is a comment \n" by removing 
	// the substring in ['#' \n). 
	bool found_hash = true; 
	while (found_hash) {

		found_hash = false;
		string::iterator it_hash, it_newline;

		// search for a hashtag 
		for (string::iterator it = input.begin(); it != input.end(); ++it) { 
			if ( *it == '#' ) {
				found_hash = true;    // we found a hashtag  
				it_hash = it;         // record location of the hashtag
				break;                // one comment at a time 
			} 
		}

		// if there is a comment, remove it 
		if (found_hash) {
			bool found_newline = false;
			// search for a newline after the hashtag 
			for (string::iterator it = it_hash; it != input.end(); ++it) {
				if ( *it == '\n' ) {
					found_newline = true;    // we found a newline   
					it_newline = it;         // record location of the newline
					break;                   // one comment at a time 
				} 
			}
			// remove the comment. be defensive by not assuming the last comment 
			// is terminated by '\n'
			if (found_newline) {
				// string to_delete(it_hash, it_newline);
				// cout << "Deleting:" << to_delete << endl;
				// cout << "done." << endl; 
				input.erase(it_hash, it_newline);
			}
			else 
				input.erase(it_hash, input.end());
		}
	}

	// CONVERT HORIZONTAL TABS TO SPACES // 
	// now convert all occurances of HT (dec 9) to SPACE (dec 32)
	bool found_ht = true;
	while (found_ht) {

		found_ht = false;

		for (string::iterator it = input.begin(); it != input.end(); ++it) { 
			int ascii_code = static_cast<int>(*it);    // get the ascii code
			if (ascii_code == 9) {
				found_ht = true; 
				input.replace(it, it + 1, " ");
			} 
		}
	}

	// DISCARD EMPTY OR WHITESPACE ONLY LINES // 
	string output;
	string line;
	istringstream iss(input); 
	while (getline(iss, line)) {
		// cout << line << endl;
		if(line.find_first_not_of(" \t\n\v\f\r") != string::npos) {
		    // cout << "The following line has non-whitespace:" << endl; 
		    // cout << line << endl; 
		    output = output + line + '\n'; 
		} 
		// else {
		// 	cout << "The following line is whitespace:" << endl;
		// 	cout << line << endl; 
		// }
	}

	// cout << "The resulting output is:" << endl;
	// cout << output; 

	// print out ascii 
	// for (string::iterator it = input.begin(); it != input.end(); ++it) { 
	// 	int ascii_code = static_cast<int>(*it);    // get the ascii code
	// 	cout << ascii_code << " "; 
	// }
	// cout << endl; 
	// cout << input;  

	return output;
}

// valid forms are:
// 		id command_name arguments\n
// 		id command_name\n
// 		command_name arguments\n
// 		command_name\n
// we only need to check if the input is in the form:
// id command_name
// or 
// command_name
GTP_Command gtp_parse_command(string input) {

	// prepare for parsing
	istringstream iss(input);
	string token; 
	GTP_Command gtp_cmd; 

    iss >> token;    // get the first token 
    try { 
    	int id = boost::lexical_cast<int>(token);
    	gtp_cmd.id = id;
    	gtp_cmd.has_id = true;

    	// if we got here, the next token should be the command_name 
    	iss >> token; 
    	string command_name = token;
    	gtp_cmd.command_name = command_name; 
    }
    catch (const boost::bad_lexical_cast &) { 
    	// the token we have should be the command name 
    	string command_name = token;
    	gtp_cmd.command_name = command_name; 
    }

    // if we made it here, we should have tokenized as far as command_name 

	// if the command is supported, parse its arguments 
	unordered_map<string, int>::const_iterator cmd =
					supported_commands.find(gtp_cmd.command_name);

	if ( cmd == supported_commands.end() ) {
		// didn't find the command_name
		gtp_cmd.error_flag = true; 
		return gtp_cmd; 
	} else {
		// look up the number of arguments for the command 
		int num_args = cmd->second;
		// read the rest of the arguments 
		for (int i = 0; i < num_args; ++i) {
			iss >> token;
			gtp_cmd.args.push_back(token); 
		}
	}
	return gtp_cmd; 
}

// wraps a response.
// this function wraps using '=' GTP syntax is the command is successful,
// and wraps with '?' syntax otherwise 
void gtp_process_command(GTP_Command &cmd) {
	// if there is no error, return a successful response with '=' 
	if (!cmd.error_flag) {    
		// prepend the id if there is one 
		if (cmd.has_id) {
			string id = to_string(cmd.id);
			cmd.response = "=" + id + " " + cmd.response + "\n\n"; 
		} else 
			cmd.response = "= " + cmd.response + "\n\n"; 
	 // otherwise, return the failure response with '?'
	} else {   
		// prepend the id if there is one 
		if (cmd.has_id) {
			string id = to_string(cmd.id);
			cmd.response = "?" + id + " " + cmd.response + "\n\n"; 
		} else 
			cmd.response = "? " + cmd.response + "\n\n"; 
	}
}

// this sends the response from the engine to stdout 
void gtp_respond(GTP_Command &cmd) {
	cout << cmd.response << endl; 
}

////////////////////////////////////////////////////////////////////////////////
// GTP COMMAND IMPLEMENTATIONS 
////////////////////////////////////////////////////////////////////////////////

// a successful response is of the form 
// =id response\n\n
// =id\n\n
// = response\n\n
// =\n\n

// string process_output(GTP_Command &cmd, string input) {
// 	string prepended; 
// 	if (cmd.has_id) {
// 		string id = to_string(cmd.id);
// 		prepended = "=" + id + " " + input ; 
// 	}
// 	return 
// }

// this code implements GTP Protocol Version 2 
void gtp_protocol_version(GTP_Command &cmd) {
	cmd.response = string("2");
	assert (!cmd.error_flag);    	// this command never fails  
	return; 
}

// prints out the name of this engine. this doesn't matter -- just pick "Izumi"
void gtp_name(GTP_Command &cmd) {
	cmd.response = string("Izumi");
	assert (!cmd.error_flag);    // this command never fails 
	return; 
}

// prints out the version number for this engine. since no sense of version 
// return the empty string (compliant with GTP) 
void gtp_version(GTP_Command &cmd) {
	cmd.response = string(""); 
	assert (!cmd.error_flag);    // this command never fails 
	return; 
}

// checks if the command given is supported 
void gtp_known_command(GTP_Command &cmd) {
	// if known return true 
	unordered_map<string, int>::const_iterator cmd_it =
					supported_commands.find(cmd.args[0]);

	if ( cmd_it == supported_commands.end() ) {
		cmd.response = string("false");    // this command is not supported 
	} else {
		cmd.response = string("true");    // this command is supported
	} 
	assert (!cmd.error_flag);    // this command never fails 
	return; 
}

// lists all the commands supported by the engine 
void gtp_list_commands(GTP_Command &cmd) {
	for (unordered_map<string, int>::iterator it = supported_commands.begin(); 
		it != supported_commands.end();
		++it) {
		cmd.response = cmd.response + it->first + "\n"; 
	}
	assert (!cmd.error_flag);    // this command never fails 
	return; 
}

// exits with success 
void gtp_quit(GTP_Command &cmd) {
	cmd.response = string(""); 
	assert (!cmd.error_flag); 
	gtp_process_command(cmd); 
	gtp_respond(cmd);
	exit (EXIT_SUCCESS); 
}

void gtp_boardsize(GTP_Command &cmd) {
	// retrive the boardsize from args 
	int size = stoi(cmd.args[0]);     

	// test if this boardsize is supported 
	if ( (size > 25) || (size < 1) ) {
		cmd.error_flag = true;
		cmd.response = string("unacceptable size"); 
		return; 
	}

	// otherwise, set the boardsize 
	board.size = size;               
	return; 
}

void gtp_clear_board(GTP_Command &cmd) {
	// retrieve the boardsize from instance, clear the board, clear the 
	// capture counts, and clear move history
	int size = board.size;    
	board.grid = vector<vector<int> >(size, vector<int>(size, EMPTY)); 
	board.captured_blk = 0;    
	board.captured_wht = 0;   
	board.move_history.clear();  

	// print_Board(board); 
	return; 
}

void gtp_komi(GTP_Command &cmd) {
	float new_komi = stof(cmd.args[0]);    // retrieve the komi from args
	board.komi = new_komi;                 // set the komi 
	return; 
}

// forces the board to play in a particular given position 
void gtp_play(GTP_Command &cmd) {

	// retrive the color and move from args  
	string color = cmd.args[0];
	string move = cmd.args[1];
	// cast color and move to lowercase 
	transform(color.begin(), color.end(), color.begin(), ::tolower);
	transform(move.begin(), move.end(), move.begin(), ::tolower);

	// parse the move and convert it to internal representation coordinates 
	char letter = move[0];               
	int index = stoi(move.substr(1));    
	tuple<char, int> external_coord (letter, index); 
	tuple<int, int> internal_coord = external_to_internal(external_coord); 

	// now internal_coord should index into our board appropriately 

	if ( (color == "black") || (color == "b") ) {
		// move black where move is
		board.grid[get<0>(internal_coord)][get<1>(internal_coord)] = BLACK;  
		// change the just_moved flag in the board 
		board.just_moved = BLACK; 

		// TODO: DEAL WITH SUPERKO USING HASHING
		// update the move history 
		// board.move_history.push_back(board.grid); 
	} else if ( (color == "white") || (color == "w" ) ) {
		// move white where move is
		board.grid[get<0>(internal_coord)][get<1>(internal_coord)] = WHITE;  
		// change just_moved
		board.just_moved = WHITE; 

		// TODO: DEAL WITH SUPERKO USING HASHING 
		// update the move history 
		// board.move_history.push_back(board.grid); 
	} else {
		cmd.error_flag = true;
		cmd.response = string("syntax error");  
	}

	// print_Board(board); 
	// ofstream logfile; 
	// logfile.open("logfile.txt");
	// logfile
	// check the board for captures 
	// logfile << "Before captures (play): ";
	// debug_Print_Board(board); 
	perform_captures(board); 
	logfile << "After captures (play): ";
	debug_Print_Board(board); 

	return; 
}

// this function determines if the grid passed in has been seen before 
// bool move_in_history(Board const &board, vector<vector<int> > const &tentative) {
// 	bool seen = false; 
// 	for (unsigned i = 0; i < board.move_history.size(); ++i) {
// 		if (tentative == board.move_history[i]) {
// 			seen = true;
// 			return seen;  
// 		}
// 	}
// 	return seen; 
// }

// we should never play into death. this function checks surrounding positions for
// stones of the appropriate color and returns true if this is a move into death 
// bool into_death(vector<vector<int> > const &grid, 
// 			    int boardsize, 
// 			    int enemy_color, 
// 			    int x_idx, 
// 			    int y_idx) {
// 	bool in_death = false; 
	
// 	// check 4 possible surrounding positions, provided their indices are valid 

// 	// check up 
// 	if (valid_index(boardsize, ++y_idx)) {

// 	}
// 	return; 
// }

void gtp_genmove(GTP_Command &cmd) {

	// retrieve the color for which we need to generate a move 
	string color = cmd.args[0];    
	transform(color.begin(), color.end(), color.begin(), ::tolower); 

	// toggle flag for white or black to move 
	bool black_flag = false;
	bool white_flag = false; 
	if ( (color == "b") || (color == "black") ) {
		black_flag = true;
	} else if ( (color == "w") || (color == "white") ) {
		white_flag = true;
	} else {
		cmd.error_flag = true;
		cmd.response = string("unknown command");
		return; 
	}

	// generates a move of the given color. for now, play random moves 
	// using a uniform random distribution. 
	int x_played;
	int y_played; 
	int color_played; 
	bool done = false; 

	// draw from a uniform random distribution on [0, board.grid.size() - 1] 
	// since the minimum index is 0 and the maximum index is the boardsize - 1. 
	default_random_engine generator; 
	generator.seed(SEED); 
	uniform_int_distribution<int> distribution(0, board.grid.size() - 1); 

	while (!done) {

		// draw samples from the distribution 
		int x_idx = distribution(generator); 
		int y_idx = distribution(generator); 

		logfile << "Generated x, y: " << x_idx << "," << y_idx << endl; 

		// copy the current board state into tentative
		vector<vector<int> > tentative = board.grid;
		
		// if the generated indices is for a filled square, then generate 
		// a new one  
		if (!(tentative[x_idx][y_idx] == EMPTY)) {
			continue; 
		}

		assert (black_flag || white_flag);

		// if we made it here, then we can at least perform the move 
		// TODO: THIS DOES NOT CHECK FOR PLAYING INTO DEATH 
		if (black_flag) {
			tentative[x_idx][y_idx] = BLACK;
			color_played = BLACK; 
		}
		if (white_flag) {
			tentative[x_idx][y_idx] = WHITE;
			color_played = WHITE;  
		}

		// TODO: DOES NOT DEAL WITH SUPERKO 
		board.grid = tentative; 
		x_played = x_idx;
		y_played = y_idx; 
		board.just_moved = color_played; 

		logfile << "x_played: " << x_idx << endl; 
		logfile << "y_played: " << y_idx << endl; 

		done = true; 

		// using the tentative board, we test if this move has occurred before 
		// if (!move_in_history(board, tentative)) {
		// 	// perform the move  
		// 	board.grid = tentative;

		// 	// record what move was performed, the color that moved, and
		// 	// record this move in the move history 
		// 	x_played = x_idx;
		// 	y_played = y_idx; 

		// 	logfile << "set xplayed: " << x_idx << endl; 
		// 	logfile << "set yplayed: " << y_idx << endl; 

		// 	board.just_moved = color_played; 
		// 	board.move_history.push_back(board.grid); 

		// 	// the move is complete 
		// 	done = true;  
		// }
	}

	// we now must relay the move that was played back to the controller 
	tuple<int, int> internal_coord (x_played, y_played);
	tuple<char, int> external_coord = internal_to_external(internal_coord); 

	// using the converted external coordinates, cast to strings and 
	// concatenate the response 
	string x_string(1, get<0>(external_coord));
	string y_string = to_string(get<1>(external_coord)); 
	string response = x_string + y_string; 

	logfile << "the response is: " << response << endl; 

	// set the response in the GTP_Command struct and then return 
	cmd.response = response; 

	// print_Board(board);
	logfile << "Choosen move: " << to_string(x_played) << " ";
	logfile << to_string(y_played) << endl; 
	// check the board for captures: 
	// logfile << "Before captures (genmove): ";
	// debug_Print_Board(board); 
	perform_captures(board); 
	logfile << "After captures (genmove): ";
	debug_Print_Board(board); 

	return; 
}

// void debug_print_board(GTP_Command &cmd) {
// 	print_Board(board); 
// }

// this function calls the appropriate GTP command for a given instance 
void gtp_dispatch(GTP_Command &cmd) {

	// depending on the command name, call the appropriate function 
	if (cmd.command_name == "protocol_version") 
		gtp_protocol_version(cmd);
	else if (cmd.command_name == "name")
		gtp_name(cmd);
	else if (cmd.command_name == "version") 
		gtp_version(cmd); 
	else if (cmd.command_name == "known_command")
		gtp_known_command(cmd);
	else if (cmd.command_name == "list_commands")
		gtp_list_commands(cmd);
	else if (cmd.command_name == "quit")
		gtp_quit(cmd);
	else if (cmd.command_name == "boardsize")
		gtp_boardsize(cmd);
	else if (cmd.command_name == "clear_board")
		gtp_clear_board(cmd);
	else if (cmd.command_name == "komi") 
		gtp_komi(cmd); 
	else if (cmd.command_name == "play")
		gtp_play(cmd);
	else if (cmd.command_name == "genmove")
		gtp_genmove(cmd);
	else {
		// cout << "made it here" << endl; 
		cmd.error_flag = true;
		cmd.response = string("unknown command"); 
	}

	return; 
}

int main() {

	// string test("3 tabs:			. # this is a comment \n # another comment \n   		   \n\n\n more stuff # final comment \n");
	// string test("# comment \n 28 komi 1.0 \n");

	init(); 

	// debug_Print_Board(board);
	// exit(EXIT_SUCCESS);

	// cout << "Before:" << endl; 
	// print_Board(board); 

	// string test("# giving protocol command: \n genmove b \n");
	// string processed = gtp_preprocess(test);

	// // cout << processed << endl; 

	// GTP_Command command = gtp_parse_command(processed);

	// // gtp_list_commands(command);
	// gtp_dispatch(command); 
	// gtp_process_command(command);
	// gtp_respond(command);

	// cout << "After:" << endl; 
	// print_Board(board); 

	// cout << "Next line: " << endl; 
	// cout << command.response; 
	// print_GTP_Command(command);
	// for (int i = 0; i < valid_commands.size(); ++i) {
	// 	cout << valid_commands[i] << endl; 
	// }

	// constantly respond to input from stdin 
	string input; 
	while (true) {
		getline(cin, input);
		cin.clear();

		if (input == "\n") 
			continue; 

		string processed = gtp_preprocess(input);
		GTP_Command cmd = gtp_parse_command(processed);
		gtp_dispatch(cmd);
		gtp_process_command(cmd);
		gtp_respond(cmd);
		// print_Board(board); 
	}

	// if we made it here, then the input was quit

	// exit (EXIT_SUCCESS); 
}