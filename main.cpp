#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include "inode.h"
#include <math.h>
#include "sha256.h"
using namespace std;

const INODE INIT_ROOT = {  .IS_USED = 0x02,
				.IS_DIR = 0x02,
				.NAME = {'/', 0, 0, 0,0, 0, 0,0, 0, 0},
				.SIZE = 0,
				.DIRECT_BLOCKS = {0,0,0},
				.INDIRECT_BLOCKS = {0,0,0},
				.DOUBLE_INDIRECT_BLOCKS = {0,0,0}};



int main(int argc, char const *argv[])
{
	string command_line;
	string command, file_sys;
	istringstream ss;

	/* all commands must be as follow <command> <file> <inputs> e.g init fs.bin 5 10 2 */
	getline(cin, command_line);
	ss.str(command_line);

	ss >> command >> file_sys;
    // command.erase(0,1);
	/* init file system */
	if(!command.compare("init"))
	{
		int size_block, num_blocks, num_inodes;
		ss >> size_block >> num_blocks >> num_inodes;
		int bit_map_size = ceil((float)num_blocks/8);

		cout << sizeof(INIT_ROOT) << endl;

		ofstream file;
		/* trunc erases files with the same name and opens a clean one */
		file.open(file_sys.c_str(), ios::binary |  ofstream::trunc);
		
		/* loading preset configuration to new file system */
		uint8_t *bit_map 	= new uint8_t[bit_map_size](); // (uint8_t*) malloc (sizeof(uint8_t)*(1 + num_blocks/(int)8));
		INODE *vec_inodes 	= new INODE[num_inodes](); // (INODE*) malloc (sizeof(INODE)*num_inodes);
		uint8_t T, N, I;
		T = size_block;
		N = num_blocks;
		I = num_inodes;
		bit_map[0] = 0x01;
		vec_inodes[0] = INIT_ROOT;

		/* writing preset configuration to new file system */
		file << T << N << I;
		for (int i = 0 ; i < bit_map_size; i++)
			file << bit_map[i];
		    
		for (int i = 0; i < num_inodes; i++)
			file.write((char*)&vec_inodes[i], sizeof(INODE));
		file << (uint8_t)0; // root INODE id

		for (int i = size_block*num_blocks - 1; i >= 0; i--)
		file << (uint8_t)0; 		

		file.close();
//		printSha256(file_sys.c_str());
	}	


	return 0;
}