Script Quiz
===========
This is a program that creates randomly generated questions based off of a movie/film script that is given as input.

Currently, there is only one type of question:

*After SOMEONE says SOMETHING, what does NEXT PERSON say next?*

Running the Quiz
----------------
In linux, compile the scripts_quiz.c into an executable, and then perform the following command:

  ./[executable file] [number of questions] [number of possible responses] [difficulty flag]

* The minimum number of questions is 1.

* The minimum number of responses is 2, the maximum is 15. (NOTE: If the program cannot find the number of responses specified for a question, it will show the maximum possible.)

* A difficulty of 0 is normal, where possible answers can come from anywhere in the script.

* A difficulty of 1 is hard, where possible answers come within 10 lines of the question.

Format of Scripts
-----------------
All scripts that are inputed are expected to be of the following format:
  * All comment lines begin with "<". ">" is not required at the end of a line but does make it look nicer.
  * A Character must begin their dialogue with their name, followed by a ":", some number of TABS or SPACES and then their dialogue
  * All subsequent lines of dialogue by character must begin with at least one TAB or SPACE

**Examples** can be seen in the _scripts_ folder
