// assume the command is in one of the forms above 
	try {
		// first form: 'id command_name arguments\n'
		iss >> token;    // assume this is the id 
		// try {
			
		// 	int id = stoi(token);    
		// 	gtp_cmd.id = id; 

		// 	iss >> token;    // this must now be the command 

		// }
		// catch (const invalid_argument &ia) {
		// 	// must be another form 
		// }
		// second form: 'id command_name\n'
		
		// third form: 'command_name arguments\n'
		
		// fourth form: 'command_name\n'
	}
	// otherwise, something went wrong while parsing 
	catch (...) {
		cerr << "Unable to parse command: \'";
		cerr << input << "\'" << endl; 
	}

	// istringstream iss(input);
	// string token;
	// iss >> token; 

	// // while we can get valid tokens, see if we can parse the command
	// bool found_id = false;
	// bool found_command = false;
	// bool valid_args = false; 

	// while (iss >> token) {
	// 	// cout << "Token is: \'" << token << "\'" << endl; 
	// 	// first token is either an id or a command_name
	// 	try {
	// 		int id = stoi(token);
	// 		found_id = true; 
	// 	} 
	// 	catch (const std::invalid_argument& ia) {
	// 		;
	// 	}
	// }	 









	// attempt to parse 
	// iss >> token;
	// try {
	// 	// assume we get an id 
	// 	cout << "In try..." << endl; 
	// 	cout << token << endl; 
	// 	int id = stoi(token);
	// 	gtp_cmd.has_id = true;
	// 	gtp_cmd.id = id; 

	// 	cout << id << endl; 
	// 	// so far so good 
	// 	iss >> token;
	// 	string command_name = token; 
	// 	gtp_cmd.command_name = command_name; 
	// 	cout << gtp_cmd.command_name << endl; 
	// }
	// catch (const invalid_argument &ia) {
	// 	// we didn't get a valid id. assume we get a command_name 
	// 	cout << "In catch..." << endl; 
	// 	string command_name = token; 
	// 	gtp_cmd.command_name = command_name; 
	// 	cout << gtp_cmd.command_name << endl; 	
	// }

	// // if the command is supported, parse its arguments 
	// unordered_map<string, int>::const_iterator cmd =
	// 				supported_commands.find(gtp_cmd.command_name);

	// if ( cmd == supported_commands.end() ) {
	// 	// didn't find the command_name
	// 	cerr << "Command not supported: \'" << gtp_cmd.command_name << "\'";
	// 	cerr << endl; 
	// 	return; 
	// } else {
	// 	// look up the number of arguments in the unordered map
	// 	int num_args = cmd->second;
	// 	cout << num_args << endl;  
	// }