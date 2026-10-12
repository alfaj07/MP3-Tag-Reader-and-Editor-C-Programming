#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Application Version */
#define APP_VERSION "1.0.0"

/* Common buffer size for strings */
#define MAX_SIZE 256

/* Status codes used for function return values */
typedef enum {
    e_success = 0,
    e_failure = 1
} Status;

/* Operation modes based on command line arguments */
typedef enum {
    e_view,        /* View tag details (e.g., ./mp3tag song.mp3) */
    e_edit,        /* Edit tag details (e.g., ./mp3tag -t "Title" song.mp3) */
    e_help,        /* Show help menu (--help or -h) */
    e_version,     /* Show version info (-v or -V) */
    e_unsupported  /* Invalid or unsupported option */
} OperationType;

/* Tag field to edit */
typedef enum {
    EDIT_NONE,
    EDIT_TITLE,    /* -t : Track Title */
    EDIT_TRACK,    /* -T : Track Number */
    EDIT_ARTIST,   /* -a : Artist Name */
    EDIT_ALBUM,    /* -A : Album Name */
    EDIT_YEAR,     /* -y : Release Year */
    EDIT_COMMENT,  /* -c : Comment */
    EDIT_GENRE     /* -g : Content Genre */
} EditTagType;

/* Detected ID3 tag version */
typedef enum {
    TAG_NONE,      /* No ID3 tag found */
    TAG_ID3V1,     /* ID3v1.0 tag */
    TAG_ID3V1_1,   /* ID3v1.1 tag (has track number) */
    TAG_ID3V2_3    /* ID3v2.3 tag */
} ID3Version;

#endif /* COMMON_H */
