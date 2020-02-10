// ScriptQuiz - A program that will ask randomly generated questions based off a script
//              given in a specified format

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Has to be at least 20
#define LINE_SIZE 500

// Macro to allow using LINE_SIZE in format string
// https://stackoverflow.com/questions/12844117/printing-defined-constants
#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)

// Contains a quote with the line_num of the quote, the author of the quote and the next author/quote
typedef struct Question
{
	int line_num;
	char author[LINE_SIZE];
	char quote[LINE_SIZE];
	char next_author[LINE_SIZE];
	char next_quote[LINE_SIZE];
} Question;

// Function prototypes
void err (char *str);
char toLower (char c);
int isAlphaNumeric (char c);
char cleanResponse (char *str);
int numOfLines (char *filename);
void compileScript (char **script, char *filename, int lineCount);
void freeScript(char **script, int lineCount);
Question *createQuestion (char **script, int lineCount, int line_num);
Question *findQuestion (char **script, int lineCount, int diffLineNum);
int askQuestion (char **script, int lineCount, int question_num, int num_responses, int diffFlag);
void playQuiz (char *name, char **script, int lineCount, int num_questions, int num_responses, int diffFlag);


// Will display an error message and then exit the program
void err (char *str)
{
	printf("\nError: %s\n\n", str);
	exit(1);
}

// Creates a lowercase version of the char provided
char toLower (char c)
{
	char r = c;
	
	if (c >= 'A' && c <= 'Z')
		r = c + 32;

	return r;
}

// Tests if a char is alphanumeric
int isAlphaNumeric (char c)
{
	if ( toLower(c) >= 'a' && toLower(c) <= 'z')
		return 1;
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

// Converts a user response string into an answer, '\0' (0) if no answer could be determined
char cleanResponse (char *str)
{
	int i;
	
	if (str == NULL)
		return '\0';
	
	// Finds the first alphanumeric character and returns it as the answer
	for (i = 0; i < LINE_SIZE, str[i] != '\0'; i++)
		if (isAlphaNumeric(str[i]))
			return str[i];
	
	// Could not find any alphanumeric character
	return '\0';
}

// Returns the number of lines in a file
int numOfLines (char *filename)
{
	char c;
	int count = 1;
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
		return -1;
	
	// Counts number of lines in file
	// https://www.geeksforgeeks.org/c-program-count-number-lines-file/
	for (c = getc(fp); c != EOF; c = getc(fp)) 
        if (c == '\n') // Increment count if this character is newline 
            count = count + 1;
	
	fclose(fp);
	return count;
}

// Writes the contents of a script file into a double array
void compileScript (char **script, char *filename, int lineCount)
{
	char tmpstr[LINE_SIZE];
	int i = 0;
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
		return;
	
	if (script == NULL)
		return;
	
	// Writes each line of the script into script
	while (1)
	{
		if (fgets(tmpstr, sizeof(tmpstr), fp) == NULL)
			break;
		
		// Removes trailing newline and other weird ending characters
		// https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
		tmpstr[strcspn(tmpstr, "\r\n")] = 0;
		
		// Copies string into script
		script[i] = malloc(LINE_SIZE * sizeof(char));
		strcpy(script[i], tmpstr);
		i++;
	}
	
	fclose(fp);
	return;
}

// Deallocates the contents of the script array
void freeScript(char **script, int lineCount)
{
	int i;
	
	if (script == NULL)
		return;
	
	for (i = 0; i < lineCount; i++)
		free(script[i]);
	free(script);
	
	return;
}

// Creates the question, returns NULL if the question couldn't be created from the given line number
Question *createQuestion (char **script, int lineCount, int line_num)
{
	Question *q;
	char quote[500];
	int i = 0, j = 0, k = 0;
	
	if (script == NULL || script[line_num] == NULL)
		return NULL;
	
	// Tests if line_num is valid
	if (line_num < 0 || line_num >= lineCount)
		return NULL;
	
	// Tests if the line itself is a quote
	if (!isAlphaNumeric(script[line_num][0]) && script[line_num][0] != '\t')
		return NULL;
	
	// Allocates memory for the question
	q = malloc(sizeof(Question));
	if (q == NULL)
		err("Memory could not be allocated for the question.");
	
	// Saves the line number that the quote is taken from
	q->line_num = line_num;
	
	// Quote is not the first line of text by author... (\t..\tQUOTE)
	if (script[line_num][0] == '\t')
	{
		while (script[line_num][i] == '\t' || script[line_num][i] == ' ')
			i++;
		
		// Creates the quote by getting rid of all the previous whitespace
		for (; i < LINE_SIZE && script[line_num][i] != '\0'; i++)
			q->quote[j++] = script[line_num][i];
		// Since there is guaranteed to be at least one tab previous, this will never overflow
		q->quote[j] = '\0';
		
		// Finds the author of the quote
		for (i = line_num; i >= -1; i--)
		{
			// When no author could be found
			if (i == -1)
			{
				strcpy(q->author, "<UNKNOWN AUTHOR>");
				break;
			}
			
			// Tests if the beginning is the name of an author
			if (isAlphaNumeric(script[i][0]))
			{
				// Copies the name of the author into question
				for (j = 0; j < LINE_SIZE && script[i][j] != ':'; j++)
					q->author[j] = script[i][j];
				// Not guaranteed that this will not exceed the allocated size
				j == LINE_SIZE ? (q->author[LINE_SIZE-1] = '\0') : (q->author[j] = '\0');
				
				break;
			}
		}
	}
	// Quote is the first line by the author... (AUTHOR:\t..\tQUOTE)
	else
	{
		// Copies the name of the author into question
		for (i = 0; i < LINE_SIZE && script[line_num][i] != ':'; i++)
			q->author[i] = script[line_num][i];
		// Not guaranteed that this will not exceed the allocated size
		if (i == LINE_SIZE)
		{
			q->author[LINE_SIZE-1] = '\0';
			strcpy(q->quote, "<UNKNOWN QUOTE>");
		}
		else
		{
			q->author[i++] = '\0';
			
			for (j = 0; i < LINE_SIZE && script[line_num][i] != '\0'; i++)
			{
				if (script[line_num][i] == '\t')
					continue;
				q->quote[j++] = script[line_num][i];
			}
			// Since there is guaranteed to be at least one ':' previous, this will never overflow
			q->quote[j] = '\0';
		}
	}
	
	// Gets the next author and next quote
	for (i = line_num+1; i <= lineCount; i++)
	{
		// Could not find a next quote or author, thus this is not a proper question
		if (i == lineCount)
		{
			free(q);
			return NULL;
		}
		
		// Invalid quote or author beginnings
		if (!isAlphaNumeric(script[i][0]) && script[i][0] != '\t')
			continue;
		
		// Next quote from the same author
		if (script[i][0] == '\t')
		{
			// Sets the two authors to be the same
			strcpy(q->next_author, q->author);
			
			j = 0;
			// Creates the next quote by getting rid of all the previous whitespace
			while (script[i][j] == '\t' || script[i][j] == ' ')
				j++;
			for (; j < LINE_SIZE && script[i][j] != '\0'; j++)
				q->next_quote[k++] = script[i][j];
			// Since there is guaranteed to be at least one tab previous, this will never overflow
			q->next_quote[k] = '\0';
			
			break;
		}
		// Next quote from different author
		else
		{
			// Copies the name of the author into question
			for (j = 0; j < LINE_SIZE && script[i][j] != ':'; j++)
				q->next_author[j] = script[i][j];
			// Not guaranteed that this will not exceed the allocated size
			if (j == LINE_SIZE)
			{
				q->next_author[LINE_SIZE-1] = '\0';
				strcpy(q->next_quote, "<UNKNOWN QUOTE>");
			}
			else
			{
				q->next_author[j++] = '\0';
				
				for (k = 0; j < LINE_SIZE && script[i][j] != '\0'; j++)
				{
					if (script[i][j] == '\t')
						continue;
					q->next_quote[k++] = script[i][j];
				}
				// Since there is guaranteed to be at least one ':' previous, this will never overflow
				q->next_quote[k] = '\0';
			}
			
			break;
		}
	}
	
	
	return q;
}

// Finds a random quote in the script that has an author and a next quote
// If diffLineNum == -1, normal difficulty is active. If not, then this uses quotes within a radial distance of the line number passed
Question *findQuestion (char **script, int lineCount, int diffLineNum)
{
	Question *q;
	char *tmpstr;
	int line_num;
	
	if (script == NULL)
		return NULL;
	
	// Attempts to create questions from random lines of the script until it is a proper question
	do
	{
		// Normal Game Difficulty or creation of original question (the correct answer)
		if (diffLineNum < 0 || diffLineNum >= lineCount)
			line_num = rand() % lineCount;
		// Hard Game Difficulty... Generates quotes within 10 spaces of the diffLineNum
		else
			line_num = ((rand() % 20) - 10) + diffLineNum;
		// Creates the question at line_num
		q = createQuestion(script, lineCount, line_num);
	}
	while (q == NULL);
	
	// Returns the found quote
	return q;
}

// Will give a question to the player with given number of responses
int askQuestion (char **script, int lineCount, int question_num, int num_responses, int diffFlag)
{
	Question *q, *q_tmp;
	char response;
	char tmp_response[LINE_SIZE], tmpstr[LINE_SIZE];
	time_t t;
	int i, correct_response;
	
	if (script == NULL)
		return 0;
	
	// Seed the random number generator
	srand((unsigned) time(&t));
	
	q = findQuestion(script, lineCount, -1);
	if (q == NULL)
		err("Could not create a question for this script.");
	
	// Prints the question
	printf("\nQuestion #%d:\n", question_num+1);
	printf("After %s says\n\t\"%s\",\nwhat does %s say next?\n", q->author, q->quote, q->next_author);
	
	// Chooses a random response to be the correct one
	correct_response = rand() % num_responses;
	// Prints out the various answer choices
	for (i = 0; i < num_responses; i++)
	{
		// Prints the correct quote
		if (i == correct_response)
			printf("%c. %s\n", 'A' + i, q->next_quote);
		// Prints other randomly generated quotes
		else
		{
			// Finds a new question and makes sure it isn't the same quote as the actual answer
			// Also makes sure they dont share the same original quote by the same author
			while (1)
			{
				// Finds a quote for the other answer choices, determines if difficulty plays a factor or not
				q_tmp = findQuestion(script, lineCount, diffFlag ? q->line_num : -1);
				// Makes sure the answers are different
				if (strcmp(q->next_quote, q_tmp->next_quote) == 0)
				{
					free(q_tmp);
					continue;
				}
				// Makes sure either the original author or the original quote are different
				if (strcmp(q->quote, q_tmp->quote) == 0)
				{
					if (strcmp(q->author, q_tmp->author) == 0)
					{
						free(q_tmp);
						continue;
					}
				}
				break;
			}
			// Prints the false quotes and frees leftover data
			printf("%c. %s\n", 'A' + i, q_tmp->next_quote);
			free(q_tmp);
		}
	}
	
	// Gets user response
	printf("Answer: ");
	// Prevents users from causing a buffer overflow
	scanf("%"STRINGIFY(LINE_SIZE)"s", tmp_response);
	// Gets an answer from the user data
	response = cleanResponse(tmp_response);

	// Frees question from memory	
	free(q);
	
	// Prints out result of question
	if (toLower(response) == 'a' + correct_response)
		printf("--CORRECT!--\n");
	else
		printf("--INCORRECT! The right choice was %c!--\n", 'A' + correct_response);
	
	// Returns whether the answer was right or not
	return toLower(response) == 'a' + correct_response;
}

void playQuiz (char *name, char **script, int lineCount, int num_questions, int num_responses, int diffFlag)
{
	Question *reward;
	int i, score = 0;
	
	if (script == NULL)
		return;
	
	// Opening statement
	printf("\nWelcome to the %s script quiz!\n", name);
	printf("This is a %d question quiz where you will be given\n", num_questions);
	printf("%d possible response%sto a quote from the script.\n", num_responses, num_responses == 1 ? "" : "s ");
	printf("Type the corresponding letter to choose your answer. Good luck!\n\n");
	
	// Gives the player questions, if the player gets it right then their score increases
	for (i = 0; i < num_questions; i++)
		if (askQuestion(script, lineCount, i, num_responses, diffFlag))
			score++;
	
	// Shows their final score
	printf("\nYou got %d/%d correct.\n\n", score, num_questions);
	// Reward for scoring at least a 50%
	if (score * 1.0 / num_questions >= 0.5)
	{
		reward = findQuestion(script, lineCount, 0);
		printf("Congrats on getting %d correct!\n", score);
		printf("As a reward... %s has a special message for you...\n", reward->author);
		printf("\"%s\"\n\n", reward->quote);
		free(reward);
	}
	
	return;
}

int main (int argc, char **argv)
{
	int lineCount;
	char **script;
	
	// Makes sure an argument is given
	if (argc < 2)
		err("No script given.");
	
	// Gets the amount of lines in the script
	lineCount = numOfLines(argv[1]);
	if (lineCount < 1)
		err("Could not read the script.");
	
	// Allocates memory for the script
	script = malloc(lineCount * sizeof(char *));
	if (script == NULL)
		err("Memory could not be allocated for the script.");
	
	compileScript(script, argv[1], lineCount);
	
	// Plays a 10 question, 5 response quiz, and on hard difficulty
	playQuiz("BACKSTROKE OF THE WEST", script, lineCount, 10, 5, 1);
	
	freeScript(script, lineCount);
	
	return 0;
}