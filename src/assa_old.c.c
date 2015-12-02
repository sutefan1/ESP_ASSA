//------------------------------------------------------------------------------
// assa.c
//
// This program is intended to interprete Brainfuck files and debug them.
//
// Gruppenbeispiel A
//
// Group: 4 study assistant Michael Schwarz
//
// Authors: Stefan Papst 1430868
//          Harald Deutschmann XXXXXX
//          Julia Heritsch XXXXXX
//
// Latest Changes: 22.11.2015 (by Stefan Papst)
//------------------------------------------------------------------------------
//
/*this is a special flag in visual studio for secure function errors/ Can be commented if there is a error in gcc*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//for error message in fopen()
//#include <errno.h>




#define SUCCESS 0
#define FALSE_ARGUMENTS 1
#define OUT_OF_MEMORY 2
#define PARSE_FILE_ERROR 3
#define READING_FILE_FAIL 4
#define END_OF_FILE 5

#define TRUE 0
#define FALSE 1
#define NEUTRAL -1

#define OPEN 0
#define CLOSE 1

typedef struct {
  unsigned char* data_segment_;
  int* break_points_;
  int code_length_;
  int data_segment_size_;
  int number_of_loops_;
  int last_stop_in_code;
  unsigned char* last_program_counter_stop_;
} Data;

typedef struct {
  char* command_;
  char** args_;
  int args_count_;
} Input;


/*------------HEADER NEED TO BE DONE-----------*/
int readCodeFromFile(Data* data, char* name);
int checkCodeCorrectness(unsigned char* data_segment, int* number_of_loops);
int checkCommandOrComment(unsigned char current_char);
int runCode(Data* data);
int parseCode(unsigned char* data_segment, int*** brackets, int number_of_loops);
int getIndexOfBracket(int** bracket_index, int size_of_array, int current_command_counter, int open_or_close);
int createBracketIndex(int*** bracket_index, int number_of_loops);
int checkIfEqalWithBreakPoint(int current_command_counter, int** break_points);
int getCommandAndArgs(Input* input);
int deleteBracketIndex(int*** bracket_index, int number_of_loops);

//-----------------------------------------------------------------------------
//
/// The main program
/// SOME TEXT
///
/// @param argc The counter how many arguments are written on the commandline
/// @param argv The value of the command line arguments
///
/// @return 0
/// @return 1
/// @return 2 
/// @return 3
///

int main(int argc, char* argv[])
{
  Data data;
  data.data_segment_ = NULL;
  data.break_points_ = NULL;
  data.break_points_ = (int*) malloc(sizeof (int) * 3);
  data.break_points_[0] = 5;
  data.break_points_[1] = 60;
  data.break_points_[2] = -2;
  data.code_length_ = 0;
  data.data_segment_size_ = 1024;
  data.number_of_loops_ = 0;
  data.last_stop_in_code = 0;
  data.last_program_counter_stop_ = NULL;
  int return_value = SUCCESS;

  if (argc == 3)
  {
    if (!strcmp(argv[1], "-e"))
    {
      return_value = readCodeFromFile(&data, argv[2]);
      runCode(&data);

    }
    else
    {
      printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
      return FALSE_ARGUMENTS;
    }
  }
  else if (argc == 1)
  {
    /*-----START DEBUG MODE-----*/
    int debug_mode_on = TRUE;
    while(debug_mode_on == TRUE)
    {
      printf("esp> ");
      Input input;
      input.args_ = NULL;


      if((return_value = getCommandAndArgs(&input)) == OUT_OF_MEMORY)
        return return_value;

      printf("command: %s\n", input.command_);
      //printf("arg: %s\n", input.args_[0]);
      
    
      if(strcmp(input.command_, "load") == TRUE)
      {
        if(input.args_count_ == 0)
        {
          printf("[ERR] wrong parameter count\n");
          free(input.command_);
          continue;
        }
        if((return_value = readCodeFromFile(&data, input.args_[0])) != SUCCESS)
        {
          if(return_value == OUT_OF_MEMORY)
            return return_value;
          else if(return_value == READING_FILE_FAIL)
          {
            continue;
          }
        }
        
        //-----------------parse should be here!!!!!!!------------------
        if (checkCodeCorrectness(data.data_segment_, &(data.number_of_loops_)) != SUCCESS)
        {
          printf("[ERR] parsing of input failed\n");
          continue;
        }
        
      }
      else if(strcmp(input.command_, "run") == TRUE)
      { 
        return_value = runCode(&data);
        if (return_value == OUT_OF_MEMORY || return_value == END_OF_FILE)
        {
          debug_mode_on = FALSE;
        }
      }
      else if (strcmp(input.command_, "quit") == TRUE)
      {
        printf("Bye.\n");
        debug_mode_on = FALSE;
      }
      
      
      //free the input
      free(input.command_);
      int counter;
      for(counter = 0; counter < input.args_count_; counter++)
      {
        free(input.args_[counter]);
      }
    }
  }
  else
  {
    printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
    return FALSE_ARGUMENTS;
  }
  free(data.data_segment_);
  free(data.break_points_);
  data.data_segment_ = NULL;
  data.break_points_ = NULL;

  return return_value;
}

int readCodeFromFile(Data* data, char* name)
{
  data->data_segment_ = (unsigned char*) calloc(1024, sizeof (unsigned char));
  if (data->data_segment_ == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }
  //set end of string
  data->data_segment_[(int) (1024 * 0.8) + 1] = '\0';

  //open file
  FILE* file;
  //errno_t error_type;
  //error_type = fopen_s(&file, name, "r");
  file = fopen(name, "r");
  if (file == NULL)
  {
    printf("[ERR] reading the file failed\n");
    //printf("errno type: %s\n", strerror(error_type));
    return READING_FILE_FAIL;
  }

  int current_char;
  int counter = 0;
  while ((current_char = getc(file)) != EOF)
  {
    if (checkCommandOrComment(current_char) == TRUE)
    {
      if (counter < (int) (data->data_segment_size_ * 0.8))
      {
        (data->data_segment_)[counter] = current_char;
        (data->data_segment_)[counter + 1] = '\0';

      }
      else
      {
        unsigned char* new_data_segment = NULL;
        data->data_segment_size_ *= 2;

        new_data_segment = (unsigned char*) realloc(data->data_segment_, data->data_segment_size_);
        if (new_data_segment == NULL)
        {
          printf("[ERR] out of memory\n");
          return OUT_OF_MEMORY;
        }
        unsigned char* start = &(new_data_segment[data->data_segment_size_ / 2]);
        memset(start, 0, data->data_segment_size_ / 2);

        data->data_segment_ = new_data_segment;
        (data->data_segment_)[counter] = current_char;
        (data->data_segment_)[counter + 1] = '\0';
      }
      data->code_length_ = (++counter) + 1;
    }
  }
  //puts(*data_segment);

  fclose(file);

  return SUCCESS;
}

int checkCodeCorrectness(unsigned char* data_segment, int* number_of_loops)
{

  //1 because in data_segment[0] is a value of how long the code part is. 
  int counter = 1;
  int open_brackets_count = 0;
  int close_brackets_count = 0;
  while (data_segment[counter] != '\0')
  {
    if (data_segment[counter] == '[')
      open_brackets_count++;
    else if (data_segment[counter] == ']')
      close_brackets_count++;
    counter++;
  }
  *number_of_loops = open_brackets_count;
  return open_brackets_count - close_brackets_count;
}

int checkCommandOrComment(unsigned char current_char)
{
  switch (current_char)
  {
  case '<':
    return TRUE;
    break;
  case '>':
    return TRUE;
    break;
  case '+':
    return TRUE;
    break;
  case '-':
    return TRUE;
    break;
  case '.':
    return TRUE;
    break;
  case ',':
    return TRUE;
    break;
  case '[':
    return TRUE;
    break;
  case ']':
    return TRUE;
    break;
  default:
    return FALSE;
    break;
  }
}

int runCode(Data* data)
{

  /*THIS HOLE PART SHOULD BE IN LOAD!*/
  int** bracket_index = NULL;
  if (createBracketIndex(&bracket_index, data->number_of_loops_) != SUCCESS)
    return OUT_OF_MEMORY;

  parseCode(data->data_segment_, &bracket_index, data->number_of_loops_);
  //--------------------------------------------------------------------------


  /*
  //--------------------------------------------------------------------------------------
  // TODO: implement run function! 
  //-------------------------------------------------------------------------------------
   */


  //data - part of segment
  unsigned char* start_address = (data->data_segment_) + data->code_length_;
  unsigned char* program_counter;
  if (data->last_program_counter_stop_ == NULL)
  {
    program_counter = start_address;
  }
  else
  {
    program_counter = data->last_program_counter_stop_;
  }
  


  int current_command_counter = data->last_stop_in_code;
  
  char eof;
  while ((eof = (data->data_segment_)[current_command_counter]) != '\0' && (checkIfEqalWithBreakPoint(current_command_counter, &data->break_points_) == FALSE))
  {
    if (program_counter >= ((data->data_segment_) + data->data_segment_size_ - 1))
    {
      printf("out of memory now\n");
      data->data_segment_size_ *= 2;

      unsigned char* new_data_segment = (unsigned char*) realloc(data->data_segment_, data->data_segment_size_);
      if (new_data_segment == NULL)
      {
        printf("[ERR] out of memory\n");
        //Do THIS IN A FUNCTION!!!!_______________
        deleteBracketIndex(&bracket_index, data->number_of_loops_);
        return OUT_OF_MEMORY;
      }

      unsigned char* start = &(new_data_segment[data->data_segment_size_ / 2]);
      memset(start, 0, data->data_segment_size_ / 2);
      data->data_segment_ = new_data_segment;
      printf("new memory done\n");
    }



    switch ((data->data_segment_)[current_command_counter])
    {
    case '<':
      //should fix the out of field bug

      if (program_counter > start_address)
      {
        --program_counter;
      }
      break;
    case '>':
      ++program_counter;
      break;
    case '+':
      ++(*program_counter);
      break;
    case '-':
      --(*program_counter);
      break;
    case '.':
      putchar(*program_counter);
      break;
    case ',':
      *program_counter = getchar();
      break;
    case '[':

      if (*program_counter == 0)
      {
        int index_of_current_command = getIndexOfBracket(bracket_index, data->number_of_loops_, current_command_counter, OPEN);
        current_command_counter = bracket_index[index_of_current_command][CLOSE];
      }
      break;
    case ']':
    {
      int index_of_current_command = getIndexOfBracket(bracket_index, data->number_of_loops_, current_command_counter, CLOSE);
      current_command_counter = bracket_index[(index_of_current_command)][OPEN];
      current_command_counter--;
      break;
    }

    default:
      break;
    }
   
    current_command_counter++;
   
  }
  
  
  data->last_stop_in_code = current_command_counter;
  data->last_program_counter_stop_ = program_counter;
  
  //DO THIS IN A FUNCTION!!!!
  deleteBracketIndex(&bracket_index, data->number_of_loops_);
  /* ---Problem with compare with nullbyte
  if(strcmp(eof, "\0") == TRUE)
  return END_OF_FILE;
  */
  return SUCCESS;
}

int parseCode(unsigned char* data_segment, int*** brackets, int number_of_loops)
{
  int counter = 1;
  int latest_open_bracket = 0;
  int bracket_queue_counter = 0;
  int* bracket_queue = (int*) calloc(number_of_loops, sizeof (int));
  if (bracket_queue == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }

  while (data_segment[counter] != '\0')
  {
    if (number_of_loops == 0)
    {
      free(bracket_queue);
      return SUCCESS;
    }
    if (data_segment[counter] == '[')
    {

      (*brackets)[latest_open_bracket][0] = counter;
      *(bracket_queue + bracket_queue_counter++) = latest_open_bracket++;
    }
    else if (data_segment[counter] == ']')
    {
      (*brackets)[*(bracket_queue + --bracket_queue_counter)][1] = counter;
      number_of_loops--;
    }
    counter++;
  }
  free(bracket_queue);
  return PARSE_FILE_ERROR;
}

int getIndexOfBracket(int** bracket_index, int size_of_array, int current_command_counter, int open_or_close)
{
  int counter;

  for (counter = 0; counter < size_of_array; counter++)
  {
    if (current_command_counter == bracket_index[counter][open_or_close])
      return counter;
  }
  return NEUTRAL;
}

int createBracketIndex(int*** bracket_index, int number_of_loops)
{
  *bracket_index = (int**) calloc(number_of_loops, sizeof (int*));
  if (*bracket_index == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }
  int col_count;
  for (col_count = 0; col_count < number_of_loops; col_count++)
  {
    (*bracket_index)[col_count] = (int*) calloc(2, sizeof ((**bracket_index)[col_count]));
    if ((*bracket_index)[col_count] == NULL)
    {
      printf("[ERR] out of memory\n");
      col_count--;
      while (col_count >= 0)
      {
        free((*bracket_index)[col_count]);
        (*bracket_index)[col_count--] = NULL;
      }
      return OUT_OF_MEMORY;
    }
  }
  return SUCCESS;
}

int checkIfEqalWithBreakPoint(int current_command_counter, int** break_points)
{
  int counter = 0;
  int current_break_point = 0;
  while((current_break_point = (*break_points)[counter]) != -2)
  {
    if (current_break_point == NEUTRAL)
    {
      counter++;
      continue;
    }
      
    
    if(current_command_counter == current_break_point)
    {
      (*break_points)[counter] = NEUTRAL;
      return TRUE;
    }
      
    counter++;
  }
  return FALSE;
}

int getCommandAndArgs(Input* input)
{
  char current_char;
  input->args_count_ = 0;
  input->args_ = NULL;
  int command_setted = FALSE;
  
  //max command is 6+nullbyte
  input->command_ = (char*)calloc(7, sizeof(char));
  if(input->command_ == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }
  
  int allocated_size_for_string = 7;
  
  
  int shell_count = 0;
  int last_char_was_space = FALSE;
  //starts the real read from stdin
  while((current_char = (char)getchar()) != '\n')
  {
    //check if there is space or more
    if(current_char == ' ' && last_char_was_space == FALSE)
    {
      command_setted = TRUE;
  
      char** new_memory_for_strings = (char**)realloc(input->args_, input->args_count_ + 1);
      if (new_memory_for_strings == NULL)
      {
        printf("[ERR] out of memory\n");
        return OUT_OF_MEMORY;
      }
      input->args_ = new_memory_for_strings;
      allocated_size_for_string = 5;
      //most args are smaller than 4 + nullbyte
      input->args_[input->args_count_] = (char*) calloc(allocated_size_for_string, sizeof(char));
      if(input->args_[input->args_count_] == NULL)
      {
        printf("[ERR] out of memory\n");
        return OUT_OF_MEMORY;
      }
      input->args_count_++;

      last_char_was_space = TRUE;
      shell_count = 0;
      continue;
    }
    else if(current_char == ' ' && last_char_was_space == TRUE)
    {
        return SUCCESS;
    }
    //because the if's above would catch it
    last_char_was_space = FALSE;
    
    //check for right string to write.
    if(command_setted == FALSE)
    {
      if((shell_count + 1) < allocated_size_for_string)
      {
        input->command_[shell_count] = current_char;
        ++shell_count;
        input->command_[shell_count] = '\0';
      }
      else
      {
        //allocated_size_for_string = last size of memoryblock
        allocated_size_for_string *= 2;
        char* new_memory_for_input = (char*)realloc(input->command_, allocated_size_for_string);
        if(new_memory_for_input == NULL)
        {
          printf("[ERR] out of memory\n");
          return OUT_OF_MEMORY;
        }
        input->command_ = new_memory_for_input;
        
        input->command_[shell_count] = current_char;
        ++shell_count;
        input->command_[shell_count] = '\0';
      }
    }
    else
    {
       if((shell_count + 1) < allocated_size_for_string)
      {
        input->args_[input->args_count_ - 1][shell_count] = current_char;
        ++shell_count;
        input->args_[input->args_count_ - 1][shell_count] = '\0';
      }
      else
      {
        //allocated_size_for_string double memory space
        allocated_size_for_string *= 2;
        char* new_memory_for_input = (char*)realloc(input->args_[input->args_count_ - 1], allocated_size_for_string);
        if(new_memory_for_input == NULL)
        {
          printf("[ERR] out of memory\n");
          return OUT_OF_MEMORY;
        }
        input->args_[input->args_count_ - 1] = new_memory_for_input;
        
        input->args_[input->args_count_ - 1][shell_count] = current_char;
        ++shell_count;
        input->args_[input->args_count_ - 1][shell_count] = '\0';
      }
    }
  }
  
  return SUCCESS;
}

int deleteBracketIndex(int*** bracket_index, int number_of_loops)
{
  int counter;
  for (counter = 0; counter < number_of_loops; counter++)
  {
    free((*bracket_index)[counter]);
    (*bracket_index)[counter] = NULL;
  }
  free(*bracket_index);
  *bracket_index = NULL;
  
  return SUCCESS;
}