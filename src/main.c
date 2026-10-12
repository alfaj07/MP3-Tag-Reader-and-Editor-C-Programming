#include "common.h"
#include "reader.h"
#include "editor.h"

/* Helper function to check if the file has .mp3 extension */
static int is_mp3_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (ext != NULL && (strcmp(ext, ".mp3") == 0 || strcmp(ext, ".MP3") == 0)) {
        return 1;
    }
    return 0;
}

/* Helper function to map CLI flag to edit type */
static EditTagType get_edit_type(const char *opt) {
    if (strcmp(opt, "-t") == 0) return EDIT_TITLE;
    if (strcmp(opt, "-T") == 0) return EDIT_TRACK;
    if (strcmp(opt, "-a") == 0) return EDIT_ARTIST;
    if (strcmp(opt, "-A") == 0) return EDIT_ALBUM;
    if (strcmp(opt, "-y") == 0) return EDIT_YEAR;
    if (strcmp(opt, "-c") == 0) return EDIT_COMMENT;
    if (strcmp(opt, "-g") == 0) return EDIT_GENRE;
    return EDIT_NONE;
}

int main(int argc, char *argv[]) {
    /* If no arguments passed, display help */
    if (argc < 2) {
        printf("Error: Missing arguments.\n");
        print_help();
        return 1;
    }

    /* 1. Help option */
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }

    /* 2. Version option */
    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "-V") == 0) {
        print_version();
        return 0;
    }

    /* 3. View mode: mp3tag <file.mp3> */
    if (argc == 2) {
        if (!is_mp3_file(argv[1])) {
            printf("Error: '%s' is not a valid .mp3 file.\n", argv[1]);
            return 1;
        }

        MP3Reader reader;
        if (open_reader_file(&reader, argv[1]) != e_success) {
            printf("Error: Unable to open file '%s'\n", argv[1]);
            return 1;
        }

        if (read_mp3_tags(&reader) == e_success) {
            display_mp3_tags(&reader);
        } else {
            printf("Error: No ID3v1 or ID3v2 tag found in '%s'\n", argv[1]);
        }

        close_reader(&reader);
        return 0;
    }

    /* 4. Edit mode: mp3tag -[flag] "value" <file.mp3> */
    if (argc == 4) {
        EditTagType type = get_edit_type(argv[1]);
        if (type == EDIT_NONE) {
            printf("Error: Invalid option '%s'\n", argv[1]);
            print_help();
            return 1;
        }

        if (!is_mp3_file(argv[3])) {
            printf("Error: '%s' is not a valid .mp3 file.\n", argv[3]);
            return 1;
        }

        MP3Editor editor;
        editor.edit_type = type;
        strncpy(editor.new_value, argv[2], MAX_SIZE - 1);
        editor.new_value[MAX_SIZE - 1] = '\0';

        if (open_editor_files(&editor, argv[3], TEMP_FILE) != e_success) {
            printf("Error: Unable to open file '%s'\n", argv[3]);
            return 1;
        }

        printf("Updating tag '%s' in %s...\n", argv[1], argv[3]);
        if (edit_mp3_tag(&editor) == e_success) {
            if (finalize_editing(&editor) == e_success) {
                printf("Tag updated successfully!\n");
                return 0;
            } else {
                printf("Error: Failed to replace original file.\n");
                return 1;
            }
        } else {
            close_editor_files(&editor);
            remove(TEMP_FILE);
            printf("Error: Failed to update tag.\n");
            return 1;
        }
    }

    /* If unrecognized format */
    printf("Error: Invalid arguments.\n");
    print_help();
    return 1;
}
