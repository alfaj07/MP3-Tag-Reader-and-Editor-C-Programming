#include "id3v2.h"

/* Convert 4 synchsafe bytes into an unsigned integer */
unsigned int synchsafe_to_int(const unsigned char bytes[4]) {
    return ((unsigned int)(bytes[0] & 0x7F) << 21) |
           ((unsigned int)(bytes[1] & 0x7F) << 14) |
           ((unsigned int)(bytes[2] & 0x7F) << 7)  |
           ((unsigned int)(bytes[3] & 0x7F));
}

/* Convert an unsigned integer into 4 synchsafe bytes */
void int_to_synchsafe(unsigned int size, unsigned char bytes[4]) {
    bytes[0] = (unsigned char)((size >> 21) & 0x7F);
    bytes[1] = (unsigned char)((size >> 14) & 0x7F);
    bytes[2] = (unsigned char)((size >> 7)  & 0x7F);
    bytes[3] = (unsigned char)(size & 0x7F);
}

/* Convert 4 big-endian bytes into an unsigned integer */
unsigned int big_endian_to_int(const unsigned char bytes[4]) {
    return ((unsigned int)bytes[0] << 24) |
           ((unsigned int)bytes[1] << 16) |
           ((unsigned int)bytes[2] << 8)  |
           ((unsigned int)bytes[3]);
}

/* Convert an unsigned integer into 4 big-endian bytes */
void int_to_big_endian(unsigned int size, unsigned char bytes[4]) {
    bytes[0] = (unsigned char)((size >> 24) & 0xFF);
    bytes[1] = (unsigned char)((size >> 16) & 0xFF);
    bytes[2] = (unsigned char)((size >> 8)  & 0xFF);
    bytes[3] = (unsigned char)(size & 0xFF);
}

/* Read and validate 10-byte ID3v2 header */
Status read_id3v2_header(FILE *fp, ID3v2Header *header) {
    if (fp == NULL || header == NULL) {
        return e_failure;
    }

    /* ID3v2 header is at the very beginning of the file */
    fseek(fp, 0, SEEK_SET);

    if (fread(header, 1, ID3V2_HEADER_SIZE, fp) != ID3V2_HEADER_SIZE) {
        return e_failure;
    }

    /* Validate "ID3" identifier */
    if (strncmp(header->id, "ID3", 3) != 0) {
        return e_failure;
    }

    /* Check supported version: ID3v2.3 (version byte == 3) */
    if (header->version != 3) {
        return e_failure;
    }

    return e_success;
}

/* Helper function to extract text payload from a frame */
static void extract_text(const char *data, unsigned int size, char *dest, int max_len) {
    if (size <= 1) {
        dest[0] = '\0';
        return;
    }

    /* Encoding byte: 0 = ISO-8859-1 (ASCII), 1 = UTF-16 with BOM */
    if (data[0] == 0) {
        unsigned int len = size - 1;
        if (len >= (unsigned int)max_len) {
            len = max_len - 1;
        }
        strncpy(dest, data + 1, len);
        dest[len] = '\0';
    } else if (data[0] == 1) {
        /* Simple UTF-16 extraction: skip encoding byte and BOM */
        unsigned int i = 3;
        int out_idx = 0;
        while (i < size && out_idx < max_len - 1) {
            char ch = data[i];
            if (ch != '\0') {
                dest[out_idx++] = ch;
            }
            i += 2;
        }
        dest[out_idx] = '\0';
    } else {
        /* Raw fallback */
        unsigned int len = size;
        if (len >= (unsigned int)max_len) {
            len = max_len - 1;
        }
        strncpy(dest, data, len);
        dest[len] = '\0';
    }
}

/* Helper function to extract comment frame */
static void extract_comment(const char *data, unsigned int size, char *dest, int max_len) {
    if (size <= 4) {
        dest[0] = '\0';
        return;
    }

    char lang[4] = {0};
    strncpy(lang, data + 1, 3);

    /* Skip short description up to null terminator */
    unsigned int idx = 4;
    while (idx < size && data[idx] != '\0') {
        idx++;
    }
    idx++; /* skip null terminator */

    if (idx < size) {
        snprintf(dest, max_len, "[Lang: %s] %.*s", lang, (int)(size - idx), data + idx);
    } else {
        snprintf(dest, max_len, "[Lang: %s]", lang);
    }
}

/* Helper function to extract attached picture (APIC) details */
static void extract_apic(const char *data, unsigned int size, ID3v2TagInfo *info) {
    if (size <= 2) {
        return;
    }

    /* MIME type begins after encoding byte (offset 1) */
    const char *mime = data + 1;
    strncpy(info->image_mime, mime, MAX_SIZE - 1);
    info->image_mime[MAX_SIZE - 1] = '\0';

    unsigned int mime_len = strlen(mime);
    unsigned int offset = 1 + mime_len + 1; /* encoding + mime + null */

    /* Skip picture type (1 byte) and description */
    if (offset < size) {
        offset++; /* skip picture type byte */
        while (offset < size && data[offset] != '\0') {
            offset++;
        }
        offset++; /* skip description null terminator */
    }

    if (offset < size) {
        info->image_size = size - offset;
    } else {
        info->image_size = 0;
    }
}

/* Read all frames from ID3v2 tag and populate ID3v2TagInfo */
Status read_id3v2_tag(FILE *fp, ID3v2TagInfo *info) {
    if (fp == NULL || info == NULL) {
        return e_failure;
    }

    memset(info, 0, sizeof(ID3v2TagInfo));

    ID3v2Header header;
    if (read_id3v2_header(fp, &header) != e_success) {
        return e_failure;
    }

    unsigned int tag_size = synchsafe_to_int(header.size);
    unsigned int bytes_read = 0;

    /* Read frames one by one */
    while (bytes_read + ID3V2_FRAME_HEADER_SIZE < tag_size) {
        ID3v2FrameHeader frame;
        if (fread(&frame, 1, ID3V2_FRAME_HEADER_SIZE, fp) != ID3V2_FRAME_HEADER_SIZE) {
            break;
        }

        /* Check for padding zeros */
        if (frame.id[0] == 0) {
            break;
        }

        unsigned int frame_size = big_endian_to_int(frame.size);
        if (frame_size == 0 || (bytes_read + ID3V2_FRAME_HEADER_SIZE + frame_size) > tag_size) {
            break;
        }

        char *buffer = (char *)malloc(frame_size + 1);
        if (buffer == NULL) {
            break;
        }

        if (fread(buffer, 1, frame_size, fp) != frame_size) {
            free(buffer);
            break;
        }
        buffer[frame_size] = '\0';

        /* Match frame ID with known tags */
        if (strncmp(frame.id, FRAME_TITLE, 4) == 0) {
            extract_text(buffer, frame_size, info->title, MAX_SIZE);
        } else if (strncmp(frame.id, FRAME_ARTIST, 4) == 0) {
            extract_text(buffer, frame_size, info->artist, MAX_SIZE);
        } else if (strncmp(frame.id, FRAME_ALBUM, 4) == 0) {
            extract_text(buffer, frame_size, info->album, MAX_SIZE);
        } else if (strncmp(frame.id, FRAME_YEAR, 4) == 0) {
            extract_text(buffer, frame_size, info->year, MAX_SIZE);
        } else if (strncmp(frame.id, FRAME_TRACK, 4) == 0) {
            extract_text(buffer, frame_size, info->track, MAX_SIZE);
        } else if (strncmp(frame.id, FRAME_GENRE, 4) == 0) {
            extract_text(buffer, frame_size, info->genre, MAX_SIZE);
        } else if (strncmp(frame.id, FRAME_COMMENT, 4) == 0) {
            extract_comment(buffer, frame_size, info->comment, MAX_SIZE);
        } else if (strncmp(frame.id, FRAME_APIC, 4) == 0) {
            extract_apic(buffer, frame_size, info);
        }

        free(buffer);
        bytes_read += ID3V2_FRAME_HEADER_SIZE + frame_size;
    }

    return e_success;
}

/* Display parsed ID3v2 tag metadata */
void display_id3v2_tag(const ID3v2TagInfo *info) {
    if (info == NULL) {
        return;
    }

    if (info->title[0] != '\0')   printf("%-8s: %s\n", "Title", info->title);
    if (info->artist[0] != '\0')  printf("%-8s: %s\n", "Artist", info->artist);
    if (info->album[0] != '\0')   printf("%-8s: %s\n", "Album", info->album);
    if (info->track[0] != '\0')   printf("%-8s: %s\n", "Track", info->track);
    if (info->year[0] != '\0')    printf("%-8s: %s\n", "Year", info->year);
    if (info->genre[0] != '\0')   printf("%-8s: %s\n", "Genre", info->genre);
    if (info->comment[0] != '\0') printf("%-8s: %s\n", "Comment", info->comment);

    if (info->image_size > 0) {
        printf("%-8s: [%s] Cover (front), %u bytes\n", "APIC", info->image_mime, info->image_size);
    }
}
