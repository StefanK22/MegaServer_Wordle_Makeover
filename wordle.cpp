#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


#define SIZE (5)
#define COLOUR_RED     "\x1b[31m"
#define COLOUR_GREEN   "\x1b[32m"
#define COLOUR_YELLOW  "\x1b[33m"
#define COLOUR_BLUE    "\x1b[34m"
#define COLOUR_RESET   "\x1b[0m"
#define PRINT_CMD "paste 1 2 3 4 5"
#define DELETE_FILES "rm 1 2 3 4 5 aux*"

int diff = 'A' - 'a';
int server;
int server2;
char* server_pipe_path;

using namespace std;

int verify_letter(int position, char c, string word, int file_size);
int count_all_words();
void welcome_message();
bool check_attempt(string attempt, int file_size);
string choose_random_word(int file_size);
void file_maker_aux(char *c, int num);
void print_letter(int colour, char* c, int num);

ifstream in;

int main(int argc, char **argv){

    int correct_letters = 0, attempts_num = 0, file_size = count_all_words(), valid = 0;
	string right_word = choose_random_word(file_size);
	welcome_message();
	server_pipe_path = argv[1];
	char *server2_pipe_path = argv[2];
	if (unlink(server_pipe_path) != 0 && errno != ENOENT)
        exit(EXIT_FAILURE);
    
    if (mkfifo(server_pipe_path, 0640) < 0)
        exit(EXIT_FAILURE);
    
    server = open(server_pipe_path, O_RDONLY);
    if (server == -1)
        exit(EXIT_FAILURE);

	server2 = open(server2_pipe_path, O_WRONLY);
	if (server2 == -1)
		exit(EXIT_FAILURE);

	while (true) {
		string attempt;
        ssize_t ret = read(server, &attempt, sizeof(attempt));
        if (ret == 0){
            if (close(server) != 0)
                exit(EXIT_FAILURE);
            server = open(server_pipe_path, O_RDONLY);
            if (server == -1)
                exit(EXIT_FAILURE);
        } else if (ret != sizeof(attempt))
            exit(EXIT_FAILURE);
		if (!check_attempt(attempt, file_size))
			continue;
		correct_letters = 0;
		for (int i = 0; i < SIZE; i++){
			int j = verify_letter(i, attempt[i], right_word, file_size);
			char c[2];
			c[0] = attempt[i];
			c[1] = '\0';
			print_letter(j, c, i + 1);
			if (j == 1)	
				correct_letters++;
		}
		cout << "\n";
		attempts_num++;
		system(PRINT_CMD);
		system(DELETE_FILES);
		valid = 0;
		if (attempts_num == 5 && correct_letters != 5){
			cout << "Perdeu. Palavra certa: " << right_word << "\n";
			if (write(server2, &valid, sizeof(valid)) != sizeof(valid))
				exit(EXIT_FAILURE);
			break;
		} else if (correct_letters == 5){
			printf(COLOUR_GREEN "PARABÉNS!!\n" COLOUR_RESET);
			if (write(server2, &valid, sizeof(valid)) != sizeof(valid))
				exit(EXIT_FAILURE);
			break;
		} else {
			valid = 1;
			if (write(server2, &valid, sizeof(valid)) != sizeof(valid))
				exit(EXIT_FAILURE);
			continue;
		}
	}

	if (close(server) != 0)
		exit(EXIT_FAILURE);
	if (close(server2) != 0)
		exit(EXIT_FAILURE);
	if (unlink(server_pipe_path) != 0)
		exit(EXIT_FAILURE);

	return 0;
}

int verify_letter(int position, char c, string word, int file_size){
	int i, found = -1;
	if (word[position] == c)
		return 1;

	for (i = 0; i < SIZE; i++){
		if (word[i] == c){
			found = 0;
			break;
		}
	}
	return found;
}

int count_all_words(){
	string line;
	int count = 0;
	in.open("words.txt");

	while(in.peek()!=EOF){
		getline(in, line);
		count++;
	}
	in.close();
	return count;
}

void welcome_message(){
	printf(COLOUR_RED "\n\n                                        Bem-vindo\n" COLOUR_RESET);
	printf(COLOUR_YELLOW "                                    W O R D L E - P T\n\n" COLOUR_RESET);
	printf(COLOUR_GREEN "Escreva:\n" COLOUR_RESET);
	printf(COLOUR_BLUE "    help" COLOUR_RESET);
	cout << " - Para ver a lista de palavras.\n";
	printf(COLOUR_BLUE "    quit" COLOUR_RESET);
	cout << " - Para sair do jogo.\n";
	printf(COLOUR_BLUE "    Uma palavra de 5 letras " COLOUR_RESET);
	cout << "(com letras minusculas) - para tentar adivinhar a resposta.\n\n";
	printf(COLOUR_YELLOW "                                 Tem apenas 5 tentativas\n" COLOUR_RESET);

}

bool check_attempt(string attempt, int file_size){

	if (attempt.compare("quit") == 0){
		if (close(server) != 0)
			exit(EXIT_FAILURE);
		if (close(server2) != 0)
			exit(EXIT_FAILURE);
		if (unlink(server_pipe_path) != 0)
			exit(EXIT_FAILURE);
			
		exit(EXIT_SUCCESS);
	}
	string word;
	in.open("words.txt");
	for (int i = 0; i < file_size; i++){
		getline(in, word);
		if (attempt.compare(word) == 0){
			in.close();
			return true;
		}
	}
	in.close();
	return false;
}

string choose_random_word(int file_size){
	string right_word;
	in.open("words.txt");

	srand((unsigned)time(0));
	for(int i = 0; i < rand() % file_size + 1; i++){
		getline(in, right_word);
	}
	in.close();
	if (right_word.length() != 5)
		exit(EXIT_FAILURE);

	return right_word;
}


void file_maker_aux(char* c, int num){
	char command[20];
	strcpy(command,"figlet "); 
	strcat(command, c);
	strcat(command, " >> aux");
	sprintf(command + 15, "%d", num);
	system(command);	
}

void print_letter(int colour, char* c, int num){

	char command[20];
	char aux_name_string[20], num_string[4];
	fstream aux_file, new_file;
	strcpy(aux_name_string, "aux");
	sprintf(aux_name_string + 3, "%d", num);
	sprintf(num_string, "%d", num);

	if (colour == 0)
		strcpy(command, "echo " COLOUR_YELLOW " >> ");
	else if (colour == 1)
		strcpy(command, "echo " COLOUR_GREEN " >> ");
	else
		strcpy(command, "touch ");
	strcat(command, num_string);

	system(command);
	file_maker_aux(c, num);

	string line;
	aux_file.open(aux_name_string);
	new_file.open(num_string);

	while(aux_file.peek() != EOF){
		if (colour == 0)
			new_file << COLOUR_YELLOW;
		else if (colour == 1)
			new_file << COLOUR_GREEN;
		getline(aux_file, line);
		new_file << line;
		if (colour != -1)
			new_file << "\x1b[0m\n";
		else
			new_file << '\n';
	}
	aux_file.close();
	new_file.close();

}
