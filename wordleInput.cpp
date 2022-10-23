#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

void print_all_words();

int server;
int server2;
ifstream in;

int main(int argc, char **argv){
	char *server_pipe_path = argv[1];
	char *server2_pipe_path = argv[2];
	int valid;
	if (unlink(server2_pipe_path) != 0 && errno != ENOENT)
		exit(EXIT_FAILURE);

	if (mkfifo(server2_pipe_path, 0640) < 0)
		exit(EXIT_FAILURE);

	server = open(server_pipe_path, O_WRONLY);
	if (server == -1)
		exit(EXIT_FAILURE);

	server2 = open(server2_pipe_path, O_RDONLY);
	if (server2 == -1)
		exit(EXIT_FAILURE);

	while (true) {
		string attempt;
		cin >> attempt;
		if (attempt.compare("help") == 0){
			print_all_words();
			continue;
		}
		ssize_t ret = write(server, &attempt, sizeof(attempt));
		if (ret != sizeof(attempt))
			exit(EXIT_FAILURE);
		
		if (attempt.compare("quit") == 0){
			break;
		}
		ret = read(server2, &valid, sizeof(valid));
		if (ret != sizeof(valid))
			exit(EXIT_FAILURE);
		if (valid == 0)
			break;
	}
	if (close(server) != 0)
		return -1;
	if (close(server2) != 0)
		return -1;
	if (unlink(server2_pipe_path) != 0)
		exit(EXIT_FAILURE);

}

void print_all_words(){
	string line;
	in.open("words.txt");
	while(in.peek()!=EOF){
		getline(in, line);
		cout << line << " ";
	}
	cout << "\n";
	in.close();
}