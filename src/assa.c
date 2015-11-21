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
// Latest Changes: 21.11.2015 (by Stefan Papst)
//------------------------------------------------------------------------------
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define SUCCESS 0
#define FALSE_ARGUMENTS 1
#define OUT_OF_MEMORY 2
#define COULD_NOT_PARSE_FILE 3
#define READING_FILE_FAIL 4



char* readCodeFromFile(char* name);



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
  char* data_segment = NULL;

  if (argc == 3)
  {
    //if (strcmp(argv[1], "-e"))
    {
      data_segment = readCodeFromFile(argv[2]);
    }
  }
  
  printf("Datasegment : %s \n", data_segment);

  return 0;
}

char* readCodeFromFile(char* name)
{
  char* data_segment = (char*) calloc(sizeof(char) * 1024, 0);
  if (data_segment == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }

  FILE* file;
  fopen_s(&file, name, "r");
  if (file == NULL)
  {
    printf("[ERR] reading the file failed\n");
    return READING_FILE_FAIL;
  }

  int current_char;
  int data_segment_size = 1024;
  int counter = 0;
  while ((current_char = getc(file)) != EOF)
  {
    
    if (counter < (int)(data_segment_size * 0,8))
    {
      data_segment[counter] = current_char;
      data_segment[counter + 1] = NULL;
    }
    else
    {
      char* new_datasegment = NULL;
      data_segment_size *= 2;

      new_datasegment = (char*)realloc(data_segment, data_segment_size);
      if (new_datasegment == NULL)
      {
        printf("[ERR] out of memory\n");
        free(data_segment);
        return OUT_OF_MEMORY;
      }
      
      data_segment = new_datasegment;
    }
  }

  fclose(file);
}