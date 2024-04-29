#include "text_parse.h"

static const int kSysCmdLen = 64;

static size_t GetLinesCount(const char *buf);

static size_t SetLineNumbers(const char *buf,
                             size_t      lines_count,
                             size_t     *line_numbers);

//==============================================================================

void SkipSpaces(char **line)
{
    while ((**line == ' ' || **line == '\t') && **line != '\0')
    {
        *((*line)++) = '\0';
    }
}

//==============================================================================

void TextDtor(Text *text)
{
    free(text->buf);
    text->buf = nullptr;

    free(text->lines_ptr);
    text->lines_ptr = nullptr;

    text->lines_count = 0;
}

//==============================================================================

size_t GetFileSize(FILE *ptr_file)
{
    struct stat file_info = {};

    fstat(fileno(ptr_file), &file_info);

    return  file_info.st_size;
}

//==============================================================================

TextErrs_t ReadTextFromFile(Text       *text,
                            const char *file_name)
{
    FILE *input_file = fopen(file_name, "rb");

    if (input_file == nullptr)
    {
        perror("\nReadTextFromFile() failed to open input file\n");

        return kOpenError;
    }
    text->buf_size = GetFileSize(input_file) + 1;
    text->buf      = (char *) calloc(text->buf_size, sizeof(char));

    if (!text->buf)
    {
        perror("\n>>ReadTextFromFile() failed to allocate memory for buf");

        return kAllocError;
    }

    size_t check_read = fread(text->buf, sizeof(char), text->buf_size - 1, input_file);

    if (check_read != (text->buf_size - 1))
    {
        perror("\n>>ReadTextFromFile() failed to read from file");

        return kReadingError;
    }

    text->lines_count = GetLinesCount(text->buf);

    size_t *line_numbers = (size_t *) calloc(text->lines_count, sizeof(size_t));

    SetLineNumbers(text->buf, text->lines_count, line_numbers);

    SplitBufIntoLines(text->buf);

    FillText(text);

    for (size_t i = 0; i < text->lines_count; i++)
    {
        text->lines_ptr[i].real_line_number = line_numbers[i];
    }

    if (fclose(input_file))
    {
        perror("\n>>ReadTextFromFile() failed to close input file");

        return kCloseError;
    }

    return kSuccess;
}

//==============================================================================

TextErrs_t ReadWordsFromFile(Text       *text,
                             const char *file_name)
{
    FILE *input_file = fopen(file_name, "rb");

    if (input_file == nullptr)
    {
        perror("\nReadTextFromFile() failed to open input file\n");

        return kOpenError;
    }

    text->buf_size = GetFileSize(input_file) + 1;
    text->buf      = (char *) calloc(text->buf_size, sizeof(char));

    if (!text->buf)
    {
        perror("\n>>ReadTextFromFile() failed to allocate memory for buf");

        return kAllocError;
    }

    size_t check_read = fread(text->buf, sizeof(char), text->buf_size - 1, input_file);

    if (check_read != (text->buf_size - 1))
    {
        perror("\n>>ReadTextFromFile() failed to read from file");

        return kReadingError;
    }

    text->lines_count = SplitBufIntoWords(text->buf, " \n\t\r");

    FillText(text);

    if (fclose(input_file))
    {
        perror("\n>>ReadTextFromFile() failed to close input file");

        return kCloseError;
    }

    return kSuccess;
}

//==============================================================================

static size_t GetLinesCount(const char *buf)
{
    size_t lines_count = 0;

    for (size_t i = 0; buf[i] != '\0';)
    {
        if (buf[i] == '\n')
        {
            ++lines_count;

            while (buf[i] == '\n' || buf[i] == '\r')
            {
                ++i;
            }
        }
        else
        {
            ++i;
        }
    }

    return lines_count;
}

//==============================================================================

static size_t SetLineNumbers(const char *buf,
                             size_t      lines_count,
                             size_t     *line_numbers)
{
    size_t line_number = 1;
    size_t cur_line    = 0;

    *(line_numbers + cur_line++) = line_number;

    for (size_t i = 0; buf[i] != '\0';)
    {
        if (buf[i] == '\n')
        {
            while (buf[i] == '\n' || buf[i] == '\r')
            {
                if (buf[i] == '\n')
                {
                    ++line_number;
                }

                ++i;
            }

            if (cur_line < lines_count)
            {
                *(line_numbers + cur_line++) = line_number;
            }
        }
        else
        {
            ++i;
        }
    }

    return line_number;
}

//==============================================================================

size_t SplitBufIntoWords(      char *buf,
                         const char *delimiters)
{
    size_t lines_count = 0;

    while (*buf != '\0')
    {
        if (*buf == '\"')
        {
            *(buf++) = '\0';

            while (*buf != '\"')
            {
                ++buf;
            }

            *(buf++) = '\0';

            ++lines_count;
        }

        if (strchr(delimiters, *buf))
        {
            while (strchr(delimiters, *buf) && *buf != '\0')
            {
                *(buf++) = '\0';
            }
        }
        else
        {
            while (!strchr(delimiters, *buf) && *buf != '\0')
            {
                ++buf;
            }

            ++lines_count;
        }
    }

    return lines_count;
}

//==============================================================================

size_t SplitBufIntoLines(char *buf)
{
    char *comment = nullptr;

    if ((comment = strchr(buf, '#')) != nullptr)
    {
        *comment = '\0';
    }

    size_t lines_count = 0;

    while (*buf != '\0')
    {
        if ((*buf) == '\n' || (*buf) == '\r')
        {
            while (*buf == '\n' || *buf == '\r')
            {
                *(buf++) = '\0';
            }
        }
        else
        {
            while ((*buf) != '\n' && *buf != '\0' && *buf != '\r')
            {
                ++buf;
            }

            ++lines_count;
        }
    }

    return lines_count;
}

//==============================================================================

void FillText(Text *text)
{
    text->lines_ptr = (Line *) calloc(text->lines_count, sizeof(Line));

    char *cur_word = text->buf;

    size_t i = 0;

    size_t words_pos = 0;

    while (i < text->buf_size)
    {
        if (*(text->buf + i) == '\0')
        {
            while (*(text->buf + i) == '\0')
            {
                ++i;
            }

            text->lines_ptr[words_pos++].str = cur_word;

            cur_word = text->buf + i;
        }

        ++i;
    }

}

//==============================================================================

void PrintTextInFile(FILE *output_file,
                     Text *text)
{
    for (size_t i = 0; i < text->lines_count; i++)
    {
        fprintf(output_file, "%s\n", (text->lines_ptr + i)->str);
    }
}

//==============================================================================

TextErrs_t GetStr(char   *string,
                  size_t  max_size)
{
    int i = 0;

    int c = 0;

    bool BufferOverflowStatus = false;

    while ((c = getchar()) != '\n')
    {
        if (i < max_size - 1)
        {
            if (c != EOF)
            {
                string[i++] = (char) c;
            }
            else
            {
                return kEOF;
            }
        }
        else
        {
            BufferOverflowStatus = true;
        }
    }

    if (BufferOverflowStatus)
    {
        return kBufferOverflow;
    }

    if (i > 0)
    {
        string[i] = '\0';
    }

    return kSuccess;;
}

static const size_t k64ByteWordSize = 64;

//==============================================================================

TextErrs_t ConvertTextToWordSet(Text    *text,
                                WordSet *word_set)
{
    CHECK(text);
    CHECK(word_set);

    word_set->word_count = text->lines_count;

    word_set->word_array = (Word *) calloc(word_set->word_count, sizeof(Word));

    if (word_set->word_array == nullptr)
    {
        perror(">> ConvertTextToWordSet() failed to allocate memory");

        return kAllocError;
    }

    for (size_t i = 0; i < text->lines_count; i++)
    {
        #ifdef WORD_IS_64_BYTE_SIZE

            word_set->word_array[i].str = (char *) calloc(k64ByteWordSize, sizeof(char));

            memcpy(word_set->word_array[i].str, text->lines_ptr[i].str, strlen(text->lines_ptr[i].str));

            word_set->word_array[i].word_len = k64ByteWordSize;

        #else

            word_set->word_array[i].str      = text->lines_ptr[i].str;

            word_set->word_array[i].word_len = strlen(word_set->word_array[i].str);

        #endif

        printf("%s - %lu\n", word_set->word_array[i].str, word_set->word_array[i].word_len);

    }

    return kSuccess;
}

//==============================================================================

TextErrs_t WriteWordSetInFile(WordSet    *word_set,
                              const char *file_name)
{
    CHECK(file_name);
    CHECK(word_set);

    FILE *word_file = fopen(file_name, "wb");

    if (word_file == nullptr)
    {
        perror(">> WriteWordSetInFile() failed to open file");

        return kOpenError;
    }

    fwrite(&word_set->word_count, sizeof(size_t), 1, word_file);

    for(size_t i = 0; i < word_set->word_count; i++)
    {
        fwrite(&word_set->word_array[i].word_len, sizeof(size_t), 1, word_file);

        fwrite(word_set->word_array[i].str,
               sizeof(char),
               word_set->word_array[i].word_len,
               word_file);
    }

    fclose(word_file);

    return kSuccess;
}

//==============================================================================
TextErrs_t ReadWordSetOutOfFile(WordSet    *word_set,
                                const char *file_name)
{
    CHECK(file_name);
    CHECK(word_set);

    FILE *word_file = fopen(file_name, "rb");

    if (word_file == nullptr)
    {
        perror(">> ReadWordSetOutOfFile() failed to open file");

        return kOpenError;
    }

    int readed_data_size = 0;

    readed_data_size = fread(&word_set->word_count, sizeof(size_t), 1, word_file);

    word_set->word_array = (Word *) calloc(word_set->word_count, sizeof(Word));

    for(size_t i = 0; i < word_set->word_count; i++)
    {
        readed_data_size = fread(&word_set->word_array[i].word_len, sizeof(size_t), 1, word_file);

        word_set->word_array[i].str = (char *) aligned_alloc(64, word_set->word_array[i].word_len * sizeof(char));

        fread(word_set->word_array[i].str, sizeof(char), word_set->word_array[i].word_len, word_file);
    }

    fclose(word_file);

    return kSuccess;
}

//==============================================================================

TextErrs_t WordSetDtor(WordSet *word_set)
{
    for (size_t i = 0; i < word_set->word_count; i++)
    {
        free(word_set->word_array[i].str);

        word_set->word_array[i].word_len = 0;

        word_set->word_array[i].str = nullptr;
    }

    free(word_set->word_array);

    return kSuccess;
}

//==============================================================================

TextErrs_t GetMaxWordLen(WordSet *word_set,
                         size_t  *len)
{
    CHECK(word_set);
    CHECK(len);

    size_t max_word_len = 0;

    for (size_t i = 0; i < word_set->word_count; i++)
    {
        if (max_word_len < word_set->word_array[i].word_len)
        {
            max_word_len = word_set->word_array[i].word_len;

            printf("max word - %s\n", word_set->word_array[i].str);
        }
    }

    *len = max_word_len;

    return kSuccess;
}

//==============================================================================


