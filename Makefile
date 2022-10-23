all: play

compile1: wordle.o
	g++ wordle.o -o wordle

compile2: wordleInput.o
	g++ wordleInput.o -o wordleInput

compile: compile1 compile2

wordle.o:
	g++ -c wordle.cpp

wordleInput.o:
	g++ -c wordleInput.cpp

clean:
	rm -rf *o wordle wordleInput 1 2 3 4 5 aux*

play: clean compile terminal
	./wordle pipe1 pipe2

play_opened: clean compile
	./wordle pipe1 pipe2

terminal: 
	open -a Terminal .

input:
	./wordleInput pipe1 pipe2