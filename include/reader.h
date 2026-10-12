#ifndef READER_H
#define READER_H

#include "common.h"
#include "id3v1.h"
#include "id3v2.h"

/* Structure to manage MP3 tag reading operations */
typedef struct {
    char filename[MAX_SIZE]; /* Path to the MP3 file */
    FILE *fptr;              /* File pointer */
    ID3Version tag_version;  /* Detected ID3 tag version */
    ID3v1Tag v1_tag;         /* Holds parsed ID3v1 tag */
    ID3v2TagInfo v2_tag;     /* Holds parsed ID3v2 tag */
} MP3Reader;

/* Function Prototypes */

/* Open MP3 file in binary read mode */
Status open_reader_file(MP3Reader *reader, const char *filename);

/* Detect and parse ID3 tags from MP3 file */
Status read_mp3_tags(MP3Reader *reader);

/* Display formatted tag details to console */
void display_mp3_tags(const MP3Reader *reader);

/* Close file pointer */
void close_reader(MP3Reader *reader);

/* Display help menu */
void print_help(void);

/* Display application version */
void print_version(void);

#endif
