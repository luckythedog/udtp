#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <string>
#define SPLIT_SIZE 548

using namespace std;

vector<char*> split;
vector<int> split_this_shit;

int main(int argc, char* argv[]){

	ifstream open_file("logo_myDog.png", ios::binary);
		int file_size;
	char* fucking_file_data = new char[46028];
	if(open_file.is_open()){
		open_file.seekg(0, open_file.end);
		cout << "Successfully opened file." << endl;
		cout << open_file.tellg() << endl;
		file_size = open_file.tellg();
		cout << file_size << endl;
		//Let's find how many files there are
		int packets = (open_file.tellg()/SPLIT_SIZE);
		open_file.seekg(0, open_file.beg);
		cout << packets << endl;
		for(int i=0; i<=packets; i++){
			int packet_size = (SPLIT_SIZE>=(open_file.end - open_file.tellg())?SPLIT_SIZE:(open_file.end - open_file.tellg()));
			char* read_data = new char[packet_size];
			int put_this_shit = packet_size;
			open_file.read(read_data, packet_size);
			split.push_back(read_data);
			split_this_shit.push_back(put_this_shit);
		}
		unsigned int offset = 0;
		for(int i=0; i<=packets; i++){
			memcpy(&fucking_file_data[offset], split[i], split_this_shit[i]);
			offset += split_this_shit[i];
		}

	}

	ofstream out_file("finally.png", ofstream::binary);
	if(out_file.is_open()){
		out_file.write(&fucking_file_data[0],file_size);
	}
	open_file.close();
	out_file.close();
	cin.get();
}
