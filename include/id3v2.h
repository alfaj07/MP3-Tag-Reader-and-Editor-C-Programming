#ifndef ID3V2_H
#define ID3V2_H

#include "common.h"

/* Header size constants */
#define ID3V2_HEADER_SIZE       10
#define ID3V2_FRAME_HEADER_SIZE 10

/* Standard Frame IDs for ID3v2.3 */
#define FRAME_TITLE   "TIT2"
#define FRAME_ARTIST  "TPE1"
#define FRAME_ALBUM   "TALB"
#define FRAME_YEAR    "TYER"
#define FRAME_TRACK   "TRCK"
#define FRAME_GENRE   "TCON"
#define FRAME_COMMENT "COMM"
#define FRAME_APIC    "APIC"

/* 10-byte ID3v2 Tag Header */
typedef struct {
    char id[3];              /* Must be "ID3" */
    unsigned char version;   /* Major version (e.g., 3 for ID3v2.3) */
    unsigned char revision;  /* Revision (usually 0) */
    unsigned char flags;     /* Flag byte */
    unsigned char size[4];   /* 4-byte synchsafe integer */
} ID3v2Header;

/* 10-byte ID3v2 Frame Header */
typedef struct {
    char id[4];              /* 4-character Frame ID (e.g. "TIT2") */
    unsigned char size[4];   /* 4-byte frame size (Big-Endian) */
    unsigned char flags[2];  /* 2 bytes of flags */
} ID3v2FrameHeader;

/* Storage for parsed ID3v2 metadata strings */
typedef struct {
    char title[MAX_SIZE];
    char artist[MAX_SIZE];
    char album[MAX_SIZE];
    char year[MAX_SIZE];
    char track[MAX_SIZE];
    char genre[MAX_SIZE];
    char comment[MAX_SIZE];
    char image_mime[MAX_SIZE];
    unsigned int image_size;
} ID3v2TagInfo;

/* Synchsafe and Big-Endian integer conversions */
unsigned int synchsafe_to_int(const unsigned char bytes[4]);
void int_to_synchsafe(unsigned int size, unsigned char bytes[4]);
unsigned int big_endian_to_int(const unsigned char bytes[4]);
void int_to_big_endian(unsigned int size, unsigned char bytes[4]);

/* Functions to read and display ID3v2 tags */
Status read_id3v2_header(FILE *fp, ID3v2Header *header);
Status read_id3v2_tag(FILE *fp, ID3v2TagInfo *info);
void display_id3v2_tag(const ID3v2TagInfo *info);

#endif
