#ifndef TEXT_PARSE_HEADER
#define TEXT_PARSE_HEADER

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../debug/debug.h"

typedef enum
{
    kSuccess        = 0,
    kOpenError      = 1,
    kAllocError     = 2,
    kFreeError      = 3,
    kCloseError     = 4,
    kReadingError   = 5,
    kReallocError   = 6,
    kBufferOverflow = 7,
    kEOF            = 8,
} TextErrs_t;

struct Line
{
    char   *str;
    size_t  real_line_number;
};

struct Word
{
    size_t word_len;
    char *str;
};

struct WordSet
{
    size_t word_count;
    Word *word_array;
};

struct Text
{
    Line    *lines_ptr;
    size_t   lines_count;
    char    *buf;
    size_t   buf_size;
};

void SkipSpaces(char **line);


size_t SplitBufIntoWords(      char *buf,
                         const char *delimiters);

size_t GetFileSize(FILE *ptr_file);

size_t SplitBufIntoLines(char *buf);

void FillText(Text *text);

void TextDtor(Text *text);

void PrintTextInFile(FILE *output_file,
                     Text *text);

TextErrs_t GetStr(char   *command_string,
                  size_t  max_size);

TextErrs_t ConvertTextToWordSet(Text    *text,
                                WordSet *word_set);

TextErrs_t ReadTextFromFile(Text       *text,
                            const char *file_name);

TextErrs_t ReadWordsFromFile(Text       *text,
                             const char *file_name);

TextErrs_t WriteWordSetInFile(WordSet    *word_set,
                              const char *file_name);

TextErrs_t ReadWordSetOutOfFile(WordSet    *word_set,
                                const char *file_name);

TextErrs_t WordSetDtor(WordSet *word_set);

TextErrs_t GetMaxWordLen(WordSet *word_set,
                         size_t  *len);

#endif
