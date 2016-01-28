#include <iostream>
#include <string>

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
				} 
			}
			// remove the comment. be defensive by not assuming the last comment 
			// is terminated by '\n'
			if (found_newline) 
				input.erase(it_hash, it_newline);
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


	// print out ascii 
	for (string::iterator it = input.begin(); it != input.end(); ++it) { 
		int ascii_code = static_cast<int>(*it);    // get the ascii code
		cout << ascii_code << " "; 
	}
	cout << endl; 
	cout << input;  
	string result("thing");
	return result;
}



int main() {
	string s("3 tabs: 			done. # this is a comment \n # another comment");
	gtp_preprocess(s);
	// print_ascii(s); 
	return 0; 
}