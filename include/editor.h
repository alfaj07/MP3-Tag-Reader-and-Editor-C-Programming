#ifndef EDITOR_H
#define EDITOR_H

#include "common.h"
#include "id3v2.h"

/* Default temporary filename during tag modification */
#define TEMP_FILE "temp.mp3"

/* Structure to manage MP3 tag editing operations */
typedef struct {
    char src_filename[MAX_SIZE];  /* Original MP3 file path */
    char temp_filename[MAX_SIZE]; /* Temporary MP3 file path */
    FILE *src_fp;                 /* Pointer to source file */
    FILE *dest_fp;                /* Pointer to temporary file */
    EditTagType edit_type;        /* Tag field to modify (e.g., EDIT_TITLE) */
    char new_value[MAX_SIZE];     /* New string value for the tag */
} MP3Editor;

/* Function Prototypes */

/* Open source file in read mode and temp file in write mode */
Status open_editor_files(MP3Editor *editor, const char *src_file, const char *temp_file);

/* Main function to update the specified tag in ID3v2 */
Status edit_mp3_tag(MP3Editor *editor);

/* Copy the remaining audio data from source to temporary file */
Status copy_remaining_audio_data(FILE *src_fp, FILE *dest_fp);

/* Close file pointers, delete original file, and rename temp to original */
Status finalize_editing(MP3Editor *editor);

/* Close all open files used by the editor */
void close_editor_files(MP3Editor *editor);

/* Helper to map EditTagType to 4-character ID3v2 Frame ID */
const char *get_frame_id_from_type(EditTagType type);

#endif
