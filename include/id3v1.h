#ifndef ID3V1_H
#define ID3V1_H

#include "common.h"

/* Total size of ID3v1 tag at the end of the MP3 file */
#define ID3V1_TAG_SIZE 128

/* Structure representing the 128-byte ID3v1 / ID3v1.1 Tag */
typedef struct {
    char header[3];          /* Must contain "TAG" */
    char title[30];          /* Song Title */
    char artist[30];         /* Artist Name */
    char album[30];          /* Album Name */
    char year[4];            /* 4-digit Year */
    char comment[28];        /* Comment */
    unsigned char zero_byte; /* 0 in ID3v1.1, or part of comment in ID3v1.0 */
    unsigned char track;     /* Track number in ID3v1.1 */
    unsigned char genre;     /* Genre code (0 - 147) */
} ID3v1Tag;

/* Function Prototypes */

/* Check and read ID3v1 tag from the end of the file */
Status read_id3v1_tag(FILE *fp, ID3v1Tag *tag);

/* Display ID3v1 tag information to console */
void display_id3v1_tag(const ID3v1Tag *tag);

/* Check if the tag is ID3v1.0 or ID3v1.1 */
ID3Version check_id3v1_version(const ID3v1Tag *tag);

/* Get genre text name from genre code */
const char *get_genre_name(unsigned char genre_code);

#endif
