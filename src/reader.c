#include "reader.h"

/* Open MP3 file in binary read mode */
Status open_reader_file(MP3Reader *reader, const char *filename) {
    if (reader == NULL || filename == NULL) {
        return e_failure;
    }

    strncpy(reader->filename, filename, MAX_SIZE - 1);
    reader->filename[MAX_SIZE - 1] = '\0';
    reader->tag_version = TAG_NONE;

    reader->fptr = fopen(filename, "rb");
    if (reader->fptr == NULL) {
        return e_failure;
    }

    return e_success;
}

/* Parse ID3 tags: checks ID3v2.3 first, falls back to ID3v1 */
Status read_mp3_tags(MP3Reader *reader) {
    if (reader == NULL || reader->fptr == NULL) {
        return e_failure;
    }

    /* 1. Try reading ID3v2.3 tag from beginning of file */
    if (read_id3v2_tag(reader->fptr, &reader->v2_tag) == e_success) {
        reader->tag_version = TAG_ID3V2_3;
        return e_success;
    }

    /* 2. If not found, try reading ID3v1 tag from end of file */
    if (read_id3v1_tag(reader->fptr, &reader->v1_tag) == e_success) {
        reader->tag_version = check_id3v1_version(&reader->v1_tag);
        return e_success;
    }

    reader->tag_version = TAG_NONE;
    return e_failure;
}

/* Display formatted metadata table to console */
void display_mp3_tags(const MP3Reader *reader) {
    if (reader == NULL) {
        return;
    }

    printf("==================================================\n");
    printf("              MP3 TAG DETAILS                     \n");
    printf("==================================================\n");
    printf("File: %s\n", reader->filename);

    if (reader->tag_version == TAG_ID3V2_3) {
        printf("Version: ID3v2.3\n");
        printf("--------------------------------------------------\n");
        display_id3v2_tag(&reader->v2_tag);
    } else if (reader->tag_version == TAG_ID3V1_1) {
        printf("Version: ID3v1.1\n");
        printf("--------------------------------------------------\n");
        display_id3v1_tag(&reader->v1_tag);
    } else if (reader->tag_version == TAG_ID3V1) {
        printf("Version: ID3v1.0\n");
        printf("--------------------------------------------------\n");
        display_id3v1_tag(&reader->v1_tag);
    }

    printf("==================================================\n");
}

/* Close file pointer */
void close_reader(MP3Reader *reader) {
    if (reader != NULL && reader->fptr != NULL) {
        fclose(reader->fptr);
        reader->fptr = NULL;
    }
}

/* Print help and usage menu */
void print_help(void) {
    printf("usage: mp3tag [file.mp3]\n");
    printf("       mp3tag -[tTaAycg] \"value\" file1\n");
    printf("       mp3tag -v\n\n");
    printf("Options:\n");
    printf("  -t   Modifies a Title tag\n");
    printf("  -T   Modifies a Track tag\n");
    printf("  -a   Modifies an Artist tag\n");
    printf("  -A   Modifies an Album tag\n");
    printf("  -y   Modifies a Year tag\n");
    printf("  -c   Modifies a Comment tag\n");
    printf("  -g   Modifies a Genre tag\n");
    printf("  -h   Displays this help info\n");
    printf("  -v   Prints version info\n");
}

/* Print version info */
void print_version(void) {
    printf("MP3 Tag Reader and Editor version %s\n", APP_VERSION);
}
