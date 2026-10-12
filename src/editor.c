#include "editor.h"

/* Map EditTagType to 4-character ID3v2 Frame ID */
const char *get_frame_id_from_type(EditTagType type) {
    switch (type) {
        case EDIT_TITLE:   return FRAME_TITLE;
        case EDIT_ARTIST:  return FRAME_ARTIST;
        case EDIT_ALBUM:   return FRAME_ALBUM;
        case EDIT_YEAR:    return FRAME_YEAR;
        case EDIT_TRACK:   return FRAME_TRACK;
        case EDIT_COMMENT: return FRAME_COMMENT;
        case EDIT_GENRE:   return FRAME_GENRE;
        default:           return NULL;
    }
}

/* Open source file in read mode and temporary file in write mode */
Status open_editor_files(MP3Editor *editor, const char *src_file, const char *temp_file) {
    if (editor == NULL || src_file == NULL || temp_file == NULL) {
        return e_failure;
    }

    strncpy(editor->src_filename, src_file, MAX_SIZE - 1);
    editor->src_filename[MAX_SIZE - 1] = '\0';

    strncpy(editor->temp_filename, temp_file, MAX_SIZE - 1);
    editor->temp_filename[MAX_SIZE - 1] = '\0';

    editor->src_fp = fopen(src_file, "rb");
    if (editor->src_fp == NULL) {
        return e_failure;
    }

    editor->dest_fp = fopen(temp_file, "wb");
    if (editor->dest_fp == NULL) {
        fclose(editor->src_fp);
        editor->src_fp = NULL;
        return e_failure;
    }

    return e_success;
}

/* Close all open editor files */
void close_editor_files(MP3Editor *editor) {
    if (editor == NULL) {
        return;
    }
    if (editor->src_fp != NULL) {
        fclose(editor->src_fp);
        editor->src_fp = NULL;
    }
    if (editor->dest_fp != NULL) {
        fclose(editor->dest_fp);
        editor->dest_fp = NULL;
    }
}

/* Helper function to write a new or updated frame to destination file */
static void write_frame(FILE *dest_fp, const char *frame_id, const char *value) {
    ID3v2FrameHeader frame;
    memset(&frame, 0, sizeof(frame));
    strncpy(frame.id, frame_id, 4);

    if (strncmp(frame_id, FRAME_COMMENT, 4) == 0) {
        unsigned int val_len = strlen(value);
        unsigned int payload_size = 5 + val_len; /* encoding + 3 lang bytes + null + text */
        int_to_big_endian(payload_size, frame.size);

        fwrite(&frame, 1, ID3V2_FRAME_HEADER_SIZE, dest_fp);

        char comm_prefix[5] = {0, 'e', 'n', 'g', 0};
        fwrite(comm_prefix, 1, 5, dest_fp);
        fwrite(value, 1, val_len, dest_fp);
    } else {
        unsigned int val_len = strlen(value);
        unsigned int payload_size = 1 + val_len; /* 1 encoding byte + text */
        int_to_big_endian(payload_size, frame.size);

        fwrite(&frame, 1, ID3V2_FRAME_HEADER_SIZE, dest_fp);

        char encoding = 0; /* ISO-8859-1 */
        fwrite(&encoding, 1, 1, dest_fp);
        fwrite(value, 1, val_len, dest_fp);
    }
}

/* Copy remaining audio data from source to temporary file */
Status copy_remaining_audio_data(FILE *src_fp, FILE *dest_fp) {
    if (src_fp == NULL || dest_fp == NULL) {
        return e_failure;
    }

    char buffer[4096];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_fp)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest_fp) != bytes_read) {
            return e_failure;
        }
    }

    return e_success;
}

/* Update target tag in ID3v2 format and write to destination file */
Status edit_mp3_tag(MP3Editor *editor) {
    if (editor == NULL || editor->src_fp == NULL || editor->dest_fp == NULL) {
        return e_failure;
    }

    const char *target_frame_id = get_frame_id_from_type(editor->edit_type);
    if (target_frame_id == NULL) {
        return e_failure;
    }

    /* Read original ID3v2 header */
    ID3v2Header header;
    fseek(editor->src_fp, 0, SEEK_SET);
    if (fread(&header, 1, ID3V2_HEADER_SIZE, editor->src_fp) != ID3V2_HEADER_SIZE) {
        return e_failure;
    }

    if (strncmp(header.id, "ID3", 3) != 0) {
        return e_failure;
    }

    unsigned int orig_tag_size = synchsafe_to_int(header.size);

    /* Write temporary placeholder header to destination file */
    fwrite(&header, 1, ID3V2_HEADER_SIZE, editor->dest_fp);

    unsigned int bytes_read = 0;
    int replaced = 0;

    /* Loop through existing frames */
    while (bytes_read + ID3V2_FRAME_HEADER_SIZE < orig_tag_size) {
        ID3v2FrameHeader frame;
        if (fread(&frame, 1, ID3V2_FRAME_HEADER_SIZE, editor->src_fp) != ID3V2_FRAME_HEADER_SIZE) {
            break;
        }

        /* Check for padding zeros */
        if (frame.id[0] == 0) {
            break;
        }

        unsigned int frame_size = big_endian_to_int(frame.size);
        if (frame_size == 0 || (bytes_read + ID3V2_FRAME_HEADER_SIZE + frame_size) > orig_tag_size) {
            break;
        }

        /* Check if this is the frame we want to replace */
        if (strncmp(frame.id, target_frame_id, 4) == 0) {
            /* Skip old frame payload in source file */
            fseek(editor->src_fp, frame_size, SEEK_CUR);

            /* Write updated frame to destination file */
            write_frame(editor->dest_fp, target_frame_id, editor->new_value);
            replaced = 1;
        } else {
            /* Copy unchanged frame to destination file */
            char *buf = (char *)malloc(frame_size);
            if (buf == NULL) {
                return e_failure;
            }

            if (fread(buf, 1, frame_size, editor->src_fp) != frame_size) {
                free(buf);
                return e_failure;
            }

            fwrite(&frame, 1, ID3V2_FRAME_HEADER_SIZE, editor->dest_fp);
            fwrite(buf, 1, frame_size, editor->dest_fp);
            free(buf);
        }

        bytes_read += ID3V2_FRAME_HEADER_SIZE + frame_size;
    }

    /* If frame did not exist previously, append it */
    if (!replaced) {
        write_frame(editor->dest_fp, target_frame_id, editor->new_value);
    }

    /* Calculate total size of newly written tag (excluding 10-byte header) */
    long new_tag_end = ftell(editor->dest_fp);
    unsigned int new_tag_size = (unsigned int)(new_tag_end - ID3V2_HEADER_SIZE);

    /* Seek back to update header with new synchsafe tag size */
    int_to_synchsafe(new_tag_size, header.size);
    fseek(editor->dest_fp, 0, SEEK_SET);
    fwrite(&header, 1, ID3V2_HEADER_SIZE, editor->dest_fp);

    /* Move src_fp to the start of audio data */
    fseek(editor->src_fp, ID3V2_HEADER_SIZE + orig_tag_size, SEEK_SET);

    /* Move dest_fp to the end of the new tag */
    fseek(editor->dest_fp, new_tag_end, SEEK_SET);

    /* Copy remaining audio stream untouched */
    if (copy_remaining_audio_data(editor->src_fp, editor->dest_fp) != e_success) {
        return e_failure;
    }

    return e_success;
}

/* Close files, remove original, and rename temp file to original */
Status finalize_editing(MP3Editor *editor) {
    if (editor == NULL) {
        return e_failure;
    }

    /* Close file handles first (required for Windows remove/rename) */
    close_editor_files(editor);

    /* Remove original file */
    if (remove(editor->src_filename) != 0) {
        return e_failure;
    }

    /* Rename temporary file to original filename */
    if (rename(editor->temp_filename, editor->src_filename) != 0) {
        return e_failure;
    }

    return e_success;
}
