#include <iostream>
// #include <string>
#include <sstream> 
// #include <vector> 
#include <unordered_map>
#include <cstdlib> 
#include <boost/lexical_cast.hpp>

#include "gtp.hpp" 
#include "board.hpp"

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
	{"play", 1},
	{"genmove", 1}
};

// a Board struct instance 
// Board board; 

////////////////////////////////////////////////////////////////////////////////
// DEBUGGING 
////////////////////////////////////////////////////////////////////////////////

// pretty prints a GTP_Command  
void gtp_debug_print_cmd(GTP_Command cmd) {
	cout << "ID: " << cmd.id << endl; 
	cout << "HAS ID: " << cmd.has_id << endl;
	cout << "COMMAND NAME: \'" << cmd.command_name << "\'" << endl; 
	for (unsigned i = 0; i < cmd.args.size(); ++i) 
		cout << "ARG " << i << ": \'" << cmd.args[i] << "\'" << endl; 
	cout << "FLAG: " << cmd.error_flag << endl; 
	cout << "RESPONSE: \'" << cmd.response << "\'" << endl;  
	return; 
}

////////////////////////////////////////////////////////////////////////////////
// INPUT / OUTPUT PROCESSING 
////////////////////////////////////////////////////////////////////////////////

// this function preprocesses an input command to the engine  
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

// prints out the version number for this engine. since no sense of version return
// the empty string (compliant with GTP) 
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

void gtp_quit(GTP_Command &cmd) {
	cmd.response = string(""); 
	assert (!cmd.error_flag); 
	gtp_process_command(cmd); 
	gtp_respond(cmd);
	exit (EXIT_SUCCESS); 
}

void gtp_boardsize(GTP_Command &cmd) {
	// if we're adjusting the board, just make a new board. 
	// this avoids any shady business with left over variables in an instance.
	return; 
}

void gtp_clear_board(GTP_Command &cmd) {
	return; 
}

void gtp_komi(GTP_Command &cmd) {
	return; 
}

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
		;
	else if (cmd.command_name == "clear_board")
		;
	else if (cmd.command_name == "komi") 
		;
	else if (cmd.command_name == "play")
		;
	else if (cmd.command_name == "genmove")
		;
	else {
		;
	}
}

int main() {
	// string test("3 tabs:			. # this is a comment \n # another comment \n   		   \n\n\n more stuff # final comment \n");
	// string test("# comment \n 28 komi 1.0 \n");
	
	string test("# giving protocol command: \n 30 protocol_version \n");
	string processed = gtp_preprocess(test);
	// cout << processed << endl; 
	GTP_Command command = gtp_parse_command(processed);
	gtp_list_commands(command);
	// cout << "Next line: " << endl; 
	// cout << command.response; 
	gtp_debug_print_cmd(command);
	// for (int i = 0; i < valid_commands.size(); ++i) {
	// 	cout << valid_commands[i] << endl; 
	// }

	// constantly respond to input from stdin 
	string input; 
	while (true) {
		getline(cin, input);
		cin.clear();



	}

	// exit (EXIT_SUCCESS); 
}