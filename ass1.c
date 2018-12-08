/* Author: James Barnes (barnesj2, 820946) */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>


#define MAX_LINE_LENGTH 1000   /* maximum length for a line's text */

#define MAX_RESULTS 5          /* number of lines printed (stage 4) */

#define SEPARATOR_LENGTH_1 3   /* separators lengths and separator sring */
#define SEPARATOR_LENGTH_2 48  
#define SEPARATOR_STRING "-"

#define FREQ_OFFSET 1          /* offsets in line scoring */
#define WORD_OFFSET 8.5        

#define FULL_OUTPUT 1          /* print the full output (stage 2/3) */


/* stores the information for a line */
typedef struct line_t
{
	int line_number;
	int word_count;
	int length;      /* stored to not require recalculation */
	double score;
	char text[MAX_LINE_LENGTH + 1];
} line_t;


/* function prototypes */
void print_query(char *query[], int query_count, char *arguments[]);
void print_line(line_t *line);
void print_line_info(line_t *line);
void print_max_lines(line_t *max_lines[]);
void print_separator(int length);
char get_line(line_t *line);
void analyse_line(line_t *line, char *query[], int query_count);
int get_word(line_t *line, int start_index, char *word);
void rank_lines(line_t *max_lines[]);
int valid_query(char *word);
int is_prefix(char *word, char *prefix);
int mygetchar();


int main(int argc, char *argv[])
{
	int query_count = argc - 1, line_count = 1, i;
	char *query[query_count], final_char = '\0';
	line_t *max_lines[MAX_RESULTS + 1], *line;
	
	/* initialise max_lines */
	for(i = 0; i <= MAX_RESULTS; i++)
	{
		max_lines[i] = (line_t *)malloc(sizeof(line_t));
		max_lines[i]->score = 0.0;
		max_lines[i]->word_count = 0;
		max_lines[i]->length = 0;
		max_lines[i]->line_number = 0;
		strcpy(max_lines[i]->text, "\0");
	};
	
	/* STAGE 1 */
	
	print_query(query, query_count, argv);

	/* STAGE 2/3 */
	
	while (final_char != EOF)
	{
		/* get next line and calculate the score, word count, etc. */
		final_char = get_line((line = max_lines[MAX_RESULTS]));
		
		analyse_line(line, query, query_count);
		line->line_number = line_count++;
		
		if (line->length > 0 && FULL_OUTPUT)
		{
			print_line_info(line);
			
			rank_lines(max_lines);
		};
	};
	
	/* STAGE 4 */
	
	print_max_lines(max_lines);
	
	for (i = 0; i <= MAX_RESULTS; i++)
	{
		free(max_lines[i]);
		max_lines[i] = NULL;
	};
	
	return 0;
};


/* copies and prints the query from arguments, then prints any invalid queries.
   exits on invalid query */
void print_query(char *query[], int query_count, char *arguments[])
{
	int invalid = 0, i;
	
	printf("S1: ");
	
	if (query_count < 1) /* no query found */
	{
		printf("No query specified, must provide at least one word\n");
		invalid = 1;
	}
	else 
	{
		/* copy and print query from arguments */	
		printf("query =");
		for (i = 0; i < query_count; i++)
		{
			query[i] = arguments[i + 1];
			printf(" %s", query[i]);
		};
		
		printf("\n");
		
		/* print invalid query words */
		for (i = 0; i < query_count; i++)
		{
			if(!valid_query(query[i]))
			{
				printf("S1: %s: invalid character(s) in query\n", query[i]);
				invalid = 1;
			};
		};
	};
	
	if (invalid)
	{
		exit(EXIT_FAILURE);
	};
};


/* print line text */
void print_line(line_t *line)
{
	printf("%s\n", line->text);
};


/* print line and line informtaion (stages 2 & 3) */
void print_line_info(line_t *line)
{
	print_separator(SEPARATOR_LENGTH_1);
	
	print_line(line);
	
	printf("S2: line = %d, bytes = %d, words = %d\n",
		line->line_number, line->length, line->word_count);
	printf("S3: line = %d, score = %.3f\n",
		line->line_number, line->score);
};


/* print lines with highest scores */
void print_max_lines(line_t *max_lines[])
{
	int i;

	print_separator(SEPARATOR_LENGTH_2);
	
	for (i = 0; i < MAX_RESULTS && max_lines[i]->score > 0.0; i++)
	{
		printf("S4: line = %d, score = %.3f\n", 
			max_lines[i]->line_number, max_lines[i]->score);
		print_line(max_lines[i]);
		
		print_separator(SEPARATOR_LENGTH_1);
	};
};


/* prints the separator betwen lines/stages */
void print_separator(int length)
{
	int i;
	
	for (i = 0; i < length; i++)
	{
		printf(SEPARATOR_STRING);
	};
	
	printf("\n");
};


/* reads the next line (ending with '\n' or EOF) from stdin */
char get_line(line_t *line)
{
	char c = '\0';
		
	/* get next line from stdin */
	line->length = 0;
	while ((c = mygetchar()) != '\n' && c != EOF)
	{
		line->text[line->length++] = c;
	};
	line->text[line->length] = '\0';
	
	return c;
};


/* get line informtion (word_count, score) */
void analyse_line(line_t *line, char *query[], int query_count)
{
	int freq[query_count], i, pos = 0;
	char word[MAX_LINE_LENGTH + 1];
	
	line->word_count = 0;
	line->score = 0.0;
	
	/* initialise frequency array */
	for (i = 0; i < query_count; i++)
	{
		freq[i] = 0;
	};
	
	/* count words */
	while (pos < line->length)
	{		
		pos = get_word(line, pos, word);
		
		if (strlen(word) > 0)
		{
			/* tally number of times a query is a prefix of word */
			for (i = 0; i < query_count; i++)
			{
				if (is_prefix(word, query[i]))
				{
					freq[i]++;
				};
			};
			
			line->word_count++;
		};
	};
	
	/* calculate score */
	for (i = 0; i < query_count; i++)
	{
		line->score += log(FREQ_OFFSET + freq[i]); 
	};
	line->score /= log(WORD_OFFSET + line->word_count);	
};


/* sets word to the next string of alphanumeric chars after start.
   retruns the index after the word */
int get_word(line_t *line, int start_index, char *word)
{
	int start = start_index, word_len = 0;
	
	while (start < line->length + 1 && !isalnum(line->text[start]))
	{
		start++;
	};
	
	/* get next word */
	while (start + word_len < line->length && 
		isalnum(line->text[start + word_len]))
	{
		word[word_len] = tolower(line->text[start + word_len]);
		word_len++;
	};
	word[word_len] = '\0';
	
	return start + word_len;
};


/* sort max_lines, inserting the newest line into it's correct postion.
   bubble-style sort is used as the size of max_lines is small, and only
   one iteration is required */
void rank_lines(line_t *max_lines[])
{
	int i;
	line_t *tmp;
	
	/* sort max_lines */
	for (i = MAX_RESULTS;
		i > 0 && max_lines[i]->score > max_lines[i - 1]->score; i--)
	{
		tmp = max_lines[i];
		max_lines[i] = max_lines[i - 1];
		max_lines[i - 1] = tmp;
	};	
};


/* checks if a given word is all lowercase/numeric */
int valid_query(char *word)
{
	int i;
	
	for (i = 0; word[i] != '\0'; i++)
	{
		if (!(islower(word[i]) || isdigit(word[i])))
		{
			return 0;
		};
	};
	
	return 1;
};


/* checks if prefix matches the start of word */
int is_prefix(char *word, char *prefix)
{
	int i, len = strlen(prefix);
	
	if (strlen(word) < len)
	{
		return 0;
	};

	for (i = 0; i < len; i++)
	{
		if (word[i] != prefix[i])
		{
			return 0;
		};
	};
	
	return 1;
};


/* source: assignment specification */
int mygetchar() 
{
	int c;
	
	while ((c = getchar()) == '\r') 
	{
	};
	
	return c;
};


/* algorithms are fun */