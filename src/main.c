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
    if (opt == NULL) return EDIT_NONE;

    /* Handle combined prefixes like "-e-t", "-e-a", etc. */
    if (strncmp(opt, "-e-", 3) == 0) {
        opt += 2; /* Now points to "-t", "-a", etc. */
    } else if (strncmp(opt, "-e", 2) == 0 && strlen(opt) == 3) {
        /* Handle "-et", "-ea", etc. */
        char flag = opt[2];
        if (flag == 't') return EDIT_TITLE;
        if (flag == 'T') return EDIT_TRACK;
        if (flag == 'a') return EDIT_ARTIST;
        if (flag == 'A') return EDIT_ALBUM;
        if (flag == 'y') return EDIT_YEAR;
        if (flag == 'c') return EDIT_COMMENT;
        if (flag == 'g' || flag == 'm') return EDIT_GENRE;
        return EDIT_NONE;
    }

    if (strcmp(opt, "-t") == 0) return EDIT_TITLE;
    if (strcmp(opt, "-T") == 0) return EDIT_TRACK;
    if (strcmp(opt, "-a") == 0) return EDIT_ARTIST;
    if (strcmp(opt, "-A") == 0) return EDIT_ALBUM;
    if (strcmp(opt, "-y") == 0) return EDIT_YEAR;
    if (strcmp(opt, "-c") == 0) return EDIT_COMMENT;
    if (strcmp(opt, "-g") == 0 || strcmp(opt, "-m") == 0) return EDIT_GENRE;
    return EDIT_NONE;
}

int main(int argc, char *argv[]) {
    /* If no arguments passed, display help */
    if (argc < 2) {
        print_help("ERROR: Missing arguments.");
        return 1;
    }

    /* 1. Help option */
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help(NULL);
        return 0;
    }

    /* 2. Version option: mp3tag -V */
    if (strcmp(argv[1], "-V") == 0) {
        print_version();
        return 0;
    }

    /* 3. View mode: mp3tag -v <file.mp3> OR mp3tag <file.mp3> */
    const char *view_file = NULL;
    if (strcmp(argv[1], "-v") == 0) {
        if (argc < 3) {
            print_help("ERROR: Missing arguments.");
            return 1;
        }
        view_file = argv[2];
    } else if (argc == 2 && is_mp3_file(argv[1])) {
        view_file = argv[1];
    }

    if (view_file != NULL) {
        if (!is_mp3_file(view_file)) {
            printf("Error: '%s' is not a valid .mp3 file.\n", view_file);
            return 1;
        }

        MP3Reader reader;
        if (open_reader_file(&reader, view_file) != e_success) {
            printf("Error: Unable to open file '%s'\n", view_file);
            return 1;
        }

        if (read_mp3_tags(&reader) == e_success) {
            display_mp3_tags(&reader);
        } else {
            printf("Error: No ID3v1 or ID3v2 tag found in '%s'\n", view_file);
        }

        close_reader(&reader);
        return 0;
    }

    /* 3. Edit mode:
     *    Format A: mp3tag -e -[flag] "value" <file.mp3>  (argc == 5)
     *    Format B: mp3tag -e-[flag] "value" <file.mp3>  (argc == 4)
     *    Format C: mp3tag -[flag] "value" <file.mp3>    (argc == 4)
     */
    EditTagType edit_type = EDIT_NONE;
    const char *new_val = NULL;
    const char *target_file = NULL;

    if (argc == 5 && strcmp(argv[1], "-e") == 0) {
        edit_type = get_edit_type(argv[2]);
        new_val = argv[3];
        target_file = argv[4];
        if (edit_type == EDIT_NONE) {
            print_help("ERROR: Invalid option.");
            return 1;
        }
    } else if (argc == 4) {
        edit_type = get_edit_type(argv[1]);
        new_val = argv[2];
        target_file = argv[3];
        if (edit_type == EDIT_NONE) {
            print_help("ERROR: Invalid option.");
            return 1;
        }
    }

    if (edit_type != EDIT_NONE) {
        if (!is_mp3_file(target_file)) {
            printf("Error: '%s' is not a valid .mp3 file.\n", target_file);
            return 1;
        }

        MP3Editor editor;
        editor.edit_type = edit_type;
        strncpy(editor.new_value, new_val, MAX_SIZE - 1);
        editor.new_value[MAX_SIZE - 1] = '\0';

        if (open_editor_files(&editor, target_file, TEMP_FILE) != e_success) {
            printf("Error: Unable to open file '%s'\n", target_file);
            return 1;
        }

        printf("Updating tag in %s...\n", target_file);
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
    print_help("ERROR: Invalid arguments.");
    return 1;
}
