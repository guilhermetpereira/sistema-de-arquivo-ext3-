#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>
#include <vector>
#include <stdint.h>
#include <cstring>
#include "inode.h"
#include "sha256.h"
using namespace std;

const INODE INIT_ROOT = {  .IS_USED = 0x01,
				.IS_DIR = 0x01,
				.NAME = {'/', 0, 0, 0,0, 0, 0,0, 0, 0},
				.SIZE = 0,
				.DIRECT_BLOCKS = {0,0,0},
				.INDIRECT_BLOCKS = {0,0,0},
				.DOUBLE_INDIRECT_BLOCKS = {0,0,0}};

/* function to return index of avaible blocks */
uint8_t* find_avaible_blocks(uint8_t* bit_map_array, int size,int num_blocks_wanted)
{
	uint8_t *return_arr = new uint8_t[num_blocks_wanted]();
	int pos = 0;
	int i,j;
	/* iteration trought bytes */
	for (i = 0; i < size; ++i)
		/* iteration trought bits */
		for (j = 0; j < 9 && pos <= num_blocks_wanted; ++j)
			/* check if bit is 0 */
			if(!(bit_map_array[i] & (1 << j)))
				return_arr[pos++] = j + i*8; // if it is, store index in return array

	return return_arr;
}

/* function to get tokens of a string
 * credits to : https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
 */
vector<string> split(const string& str, const string& delim)
{
    vector<string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}


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

		ofstream file_write;
		/* trunc erases file_writes with the same name and opens a clean one */
		file_write.open(file_sys.c_str(), ios::binary |  ofstream::trunc);
		
		/* loading preset configuration to new file_write system */
		uint8_t *bit_map 	= new uint8_t[bit_map_size](); // (uint8_t*) malloc (sizeof(uint8_t)*(1 + num_blocks/(int)8));
		INODE *vec_inodes 	= new INODE[num_inodes](); // (INODE*) malloc (sizeof(INODE)*num_inodes);
		uint8_t T, N, I;
		T = size_block;
		N = num_blocks;
		I = num_inodes;
		bit_map[0] = 0x01;
		vec_inodes[0] = INIT_ROOT;

		/* writing preset configuration to new file_write system */
		file_write << T << N << I;
		/* write bit_map */
		for (int i = 0 ; i < bit_map_size; i++)
			file_write << bit_map[i];
		/* write all inodes, root + I-1 empty */
		for (int i = 0; i < num_inodes; i++)
			file_write.write((char*)&vec_inodes[i], sizeof(INODE));

		file_write << (uint8_t)0; // root INODE id
		/* write N empty blocks */
		for (int i = size_block*num_blocks - 1; i >= 0; i--)
		file_write << (uint8_t)0; 		

		/* save file */
		file_write.close();

		/* free alocated memory */
		delete[] bit_map;
		delete[] vec_inodes;
//		printSha256(file_sys.c_str());
	}	
	else if(!command.compare("add"))
	{
		string parse, content;
		ss >> parse >> content;
		/* divides file path into directiores and file */
		vector<string> file_path = split(parse.c_str(), "/");
		/*loads parent directiorie name or root */
		string dir = (file_path.size() > 1) ? file_path.rbegin()[1]  : "/";


		/* Read and load into variavles number and size of blocks, and number of inodes */
		uint8_t* buffer = new uint8_t[3];
		fstream file;
		file.open(file_sys.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out);
		file.read((char*)buffer, 3);

		int T, N, I;
		T = buffer[0];
		N = buffer[1];
		I = buffer[2];
		delete[] buffer;

		int bit_map_size = ceil((float)N/8);
		uint8_t *bit_map = new uint8_t(bit_map_size);	
		file.read((char*)bit_map, bit_map_size);

		int blocks_needed = ceil((float)content.size()/2);
		uint8_t *avaible_blocks = find_avaible_blocks(bit_map, bit_map_size, blocks_needed);
		if(avaible_blocks[blocks_needed-1] == 0)
		{
			cout << "No space for this file on blocks!" << endl;
			return -1;
		}
		/* att bitmap but doesnt write to file system yet, necessary to check if directorie will have space for the new file*/ 
		for (int i = 0; i < bit_map_size; ++i)
			for (int j = 0; j < blocks_needed; ++j)
				bit_map[i] |= 1 << avaible_blocks[j];

		INODE *parent_dir = NULL;
		string compare;
		int counter = I;
		do
		{
			buffer = new uint8_t(sizeof(INODE));
			file.read((char*)buffer, sizeof(INODE));
			parent_dir = (INODE*)buffer;
			compare = (parent_dir->IS_USED && parent_dir->IS_DIR) ? parent_dir->NAME : "";
			counter--;
			delete[] buffer;
		}while(counter > 0 && strcmp(compare.c_str(), dir.c_str()));

		if(counter <= 0 )
		{
			cout << "Couldn't find parent dir" << endl;
			return -2;
		}

		long parent_dir_pos = file.tellp() - sizeof(INODE);





		/* search for avaible inode */ 


		/* to do
		 * checar se existe espaço para mais um bloco
		 * processar o path para determinar se é arquivo no root ou outro diretório
		 * adicionar inode 
		 */
		file.close();
	}

	


	return 0;
}
