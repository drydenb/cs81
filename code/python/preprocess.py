from __future__ import print_function
import numpy as np 
import os 
import copy 
import time
from gomill import sgf
from gomill import sgf_moves
from gomill import boards 

BLACK = 1 
WHITE = 2 
BOARDSIZE = 19 

################################################################################
# FUNCTION DEFINITIONS
################################################################################

def print_board(arr):
	""" Prints out a numpy array assumed to represent a 
	    19 x 19 board. 
	"""
	for i in range(BOARDSIZE):
		for j in range(BOARDSIZE):
			print(arr[i][j], end=" ")
		print()
	print() 
	return 

def print_arr_seq(arr):
	""" Prints out a numpy array assumed to represent a 
	    set of 19 x 19 board states. 
	"""
	depth = np.shape(arr)[0]
	for i in range(depth):
		print_board(arr[i])
		print()
	return 

def make_board_arr(occupied_points):
	""" Returns a numpy array that represents a board from a list 
		of occupied points of the form ('color', (row, col))
	"""
	arr = np.zeros((BOARDSIZE, BOARDSIZE), dtype=int)
	for point in occupied_points: 
		if point[0] == 'b':
			arr[point[1][0]][point[1][1]] = BLACK
		else: 
			arr[point[1][0]][point[1][1]] = WHITE 
	return arr 

def tuple_to_index(tup):
	""" Converts a tuple of the form (row, col) to an index in the 
	    board -- from 0 to 360. 
	"""
	row, col = tup[0], tup[1]
	# print(row)
	# print(col)
	assert (row in range(BOARDSIZE)) and (col in range(BOARDSIZE))
	idx = int(col + (row * BOARDSIZE))  
	return idx 

def make_move_arr(move_seq, depth):
	""" Returns a numpy array that takes the move sequence acquired from 
	    gomill boards and creates a sequence of indices into the go board. 
	"""
	move_arr = np.empty(depth, dtype=int)
	for i in range(depth):
		tup = move_seq[i][1]
		move_arr[i] = tuple_to_index(tup) 
	return move_arr

def partition_board_arr(board_arr, color_to_move):
	""" Partitions a board state consisting of black (1) and white (2) pieces
		into two numpy arrays. The first represents the pieces of the player 
		who will move next, and the second represents the pieces of the player
		who just moved. These arrays are binary matrices. 
	"""
	# gomill's parser should always yeild either 'w' or 'b' as valid colors 
	assert (color_to_move == 'w') or (color_to_move == 'b')

	# first initialize the black and white boards as the original board
	# cannot use e.g. black_arr = board_arr as this just creates a ref. 
	black_arr = np.copy(board_arr)
	white_arr = np.copy(board_arr) 

	# construct the white board by removing the white pieces 
	black_arr[black_arr == WHITE] = 0

	# construct the black board by removing the black pieces 
	white_arr[white_arr == BLACK] = 0 
	white_arr[white_arr == WHITE] = 1 

	# always return the tuple in the form (color-to-move, color-moved) 
	if color_to_move == 'w':
		return (white_arr, black_arr)
	else:
		return (black_arr, white_arr) 

def partition_board_states(states_arr, move_seq, depth):
	""" Takes the 3D numpy array of board states and constructs 
	    two move 3D numpy arrays. The first contains binary matrices
	    for the player to move, and the second contains binary matrices
	    of the player who just moved. 
	"""
	move_arr = np.empty((depth, BOARDSIZE, BOARDSIZE), dtype=int)
	next_arr = np.empty((depth, BOARDSIZE, BOARDSIZE), dtype=int)
	for i in range(depth):
		(move_arr_i, next_arr_i) = partition_board_arr(states_arr[i], 
			move_seq[i][0])
		move_arr[i] = move_arr_i 
		next_arr[i] = next_arr_i 
	return (move_arr, next_arr)

def preprocess_sgf(sgf_filename):
	""" Takes an .sgf file and creates three numpy arrays. Every boardstate 
	    is partitioned into the pieces of the player to play and the player 
	    who just played. These create the first two numpy arrays. The third 
	    numpy array is simply the sequence of resulting moves given a 
	    boardstate. 
	"""
	# retrieve the raw data 
	with open(sgf_filename) as f:
		raw_sgf = f.read()

	# load the game into the sgf game object 
	game = sgf.Sgf_game.from_string(raw_sgf)
	assert game.get_size() == BOARDSIZE 

	# retrieve the initial board setup and the sequence of moves for the game 
	(init_board, moves) = sgf_moves.get_setup_and_moves(game) 
	main_seq = [node.get_move() for node in game.get_main_sequence()]

	# assume the loaded data structure has (None, None) as the first move
	# in the main sequence 
	assert main_seq[0] == (None, None)
	main_seq = main_seq[1:] 

	# if the game ended with two passes, remove them. we won't consider them 
	# for move prediction 
	if (main_seq[-1][1] is None) and (main_seq[-2][1] is None):
		main_seq = main_seq[:len(main_seq) - 2]

	# assert that no other moves are passes -- this is a good assumption but 
	# we can double check for the moment 
	for move in main_seq:
		assert move[1] is not None

	# intitialize a 3D numpy array to keep track of all the games 
	depth = len(main_seq) 
	G = np.empty((depth, BOARDSIZE, BOARDSIZE), dtype=int)
	G[0] = make_board_arr(init_board.list_occupied_points())

	# contains board class instances for each state throughout the game  
	states_list = [init_board]

	# populate the 3D array of game states via states_list 
	for i, move in enumerate(main_seq):
		if i == depth - 1:
			break 
		new_state = copy.deepcopy(states_list[-1])         
		new_state.play(move[1][0], move[1][1], move[0])    
		G[i + 1] = make_board_arr(new_state.list_occupied_points())              
		states_list.append(new_state)                  

	assert len(states_list) == len(main_seq)    # sanity check 

	# parition all the board states to create the first two arrays, then
	# create the third array 
	(curr_arr, prev_arr) = partition_board_states(G, main_seq, depth) 
	move_arr = make_move_arr(main_seq, depth) 

	# create filenames to save the processed arrays 
	sgf_file_prefix = os.path.splitext(sgf_filename)[0]
	full_filename = './processed/' + sgf_file_prefix + '_full.npy' # complete states 
	curr_filename = './processed/' + sgf_file_prefix + '_curr.npy' # to move 
	prev_filename = './processed/' + sgf_file_prefix + '_prev.npy' # just moved
	move_filename = './processed/' + sgf_file_prefix + '_move.npy' # resulting moves 

	# create a directory for the processed arrays 
	newpath = r'./processed' 
	if not os.path.exists(newpath):
	    os.makedirs(newpath)

	# save the arrays 
	np.save(curr_filename, fix_imports=True, arr=curr_arr)
	np.save(prev_filename, fix_imports=True, arr=prev_arr)
	np.save(move_filename, fix_imports=True, arr=move_arr)

	return 

################################################################################
# MAIN
################################################################################

if __name__ == '__main__': 
	start_time = time.time() # for keeping track of runtime 
	preprocess_sgf('./testgame.sgf')
	print("--- Main completed in %s seconds ---" % (time.time() - start_time)) 
	