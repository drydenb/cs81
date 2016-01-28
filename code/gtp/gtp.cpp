#include <iostream>
#include <string>
#include <sstream> 

using namespace std;

// void print_ascii(string s) {
// 	for (string::iterator it = s.begin(); it != s.end(); ++it) { 
// 		int ascii_code = static_cast<int>(*it);    // get the ascii code
// 		cout << "Original string:" << s;
// 		cout << "ASCII, space delim:" << ascii_code << " "; 
// 	}
// }

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
void gtp_parse_command(string input) {

	istringstream iss(input);
	string token;
	iss >> token; 

	// while we can get valid tokens, see if we can parse the command
	bool found_id = false;
	bool found_command = false;
	bool valid_args = false; 

	while (iss >> token) {
		// cout << "Token is: \'" << token << "\'" << endl; 
		// first token is either an id or a command_name
		try {
			int id = stoi(token);
			found_id = true; 
		} 
		catch (const std::invalid_argument& ia) {
			;
		}
	}	 
}

int main() {
	string test("3 tabs:			. # this is a comment \n # another comment \n   		   \n\n\n more stuff # final comment \n");
	string processed = gtp_preprocess(test);
	gtp_parse_command(processed); 
	return 0; 
}