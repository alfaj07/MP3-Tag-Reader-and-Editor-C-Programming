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

static void print_tag_row(const char *name, const char *value) {
    printf("| %-17s | %-59s |\n", name, (value != NULL && value[0] != '\0') ? value : "");
}

/* Display formatted metadata table to console */
void display_mp3_tags(const MP3Reader *reader) {
    if (reader == NULL) {
        return;
    }

    const char *border_full = "+---------------------------------------------------------------------------------+";
    const char *border_table = "+-------------------+-------------------------------------------------------------+";

    printf("%s\n", border_full);
    printf("|                              SELECTED VIEW DETAILS                              |\n");
    printf("%s\n", border_full);
    printf("|                                                                                 |\n");
    if (reader->tag_version == TAG_ID3V1 || reader->tag_version == TAG_ID3V1_1) {
        printf("|                   MP3 TAG READER AND EDITOR FOR ID3v1                            |\n");
    } else {
        printf("|                   MP3 TAG READER AND EDITOR FOR ID3v2                            |\n");
    }
    printf("|                                                                                 |\n");
    printf("%s\n", border_table);
    printf("| TAG NAME          | TAG VALUE                                                   |\n");
    printf("%s\n", border_table);

    if (reader->tag_version == TAG_ID3V2_3) {
        print_tag_row("TITLE", reader->v2_tag.title);
        print_tag_row("ARTIST", reader->v2_tag.artist);
        print_tag_row("ALBUM", reader->v2_tag.album);
        print_tag_row("YEAR", reader->v2_tag.year);
        print_tag_row("MUSIC", reader->v2_tag.genre);
        print_tag_row("COMMENT", reader->v2_tag.comment);
    } else if (reader->tag_version == TAG_ID3V1 || reader->tag_version == TAG_ID3V1_1) {
        print_tag_row("TITLE", reader->v1_tag.title);
        print_tag_row("ARTIST", reader->v1_tag.artist);
        print_tag_row("ALBUM", reader->v1_tag.album);
        print_tag_row("YEAR", reader->v1_tag.year);
        print_tag_row("MUSIC", get_genre_name(reader->v1_tag.genre));
        print_tag_row("COMMENT", reader->v1_tag.comment);
    }

    printf("%s\n", border_table);
}

/* Close file pointer */
void close_reader(MP3Reader *reader) {
    if (reader != NULL && reader->fptr != NULL) {
        fclose(reader->fptr);
        reader->fptr = NULL;
    }
}

/* Helper to print centered text within borders */
static void print_centered(const char *text, int width) {
    int len = (int)strlen(text);
    int pad = (width - len) / 2;
    int pad_right = width - len - pad;
    printf("|%*s%s%*s|\n", pad, "", text, pad_right, "");
}

/* Print help and usage menu with boxed styling */
void print_help(const char *error_msg) {
    const char *border = "+-------------------------------------------------------------------------------+";
    printf("%s\n", border);
    if (error_msg != NULL) {
        print_centered(error_msg, 79);
    } else {
        print_centered("HELP", 79);
    }
    printf("%s\n", border);
    printf("|                                                                               |\n");
    printf("| USAGE:                                                                        |\n");
    printf("|   mp3tag -v [file.mp3]                                                        |\n");
    printf("|   mp3tag -[tTaAycg] \"value\" file1                                             |\n");
    printf("|   mp3tag -V                                                                   |\n");
    printf("|                                                                               |\n");
    printf("%s\n\n", border);

    printf("%s\n", border);
    printf("|                                   OPTIONS:                                    |\n");
    printf("%s\n", border);
    printf("|   -t    Modifies a Title tag                                                  |\n");
    printf("|   -T    Modifies a Track tag                                                  |\n");
    printf("|   -a    Modifies an Artist tag                                                |\n");
    printf("|   -A    Modifies an Album tag                                                 |\n");
    printf("|   -y    Modifies a Year tag                                                   |\n");
    printf("|   -c    Modifies a Comment tag                                                |\n");
    printf("|   -g    Modifies a Genre tag                                                  |\n");
    printf("|   -h    Displays this help info                                               |\n");
    printf("|   -v    Views tag details                                                     |\n");
    printf("|   -V    Prints version info                                                   |\n");
    printf("%s\n", border);
}

/* Print version info */
void print_version(void) {
    printf("MP3 Tag Reader and Editor version %s\n", APP_VERSION);
}
