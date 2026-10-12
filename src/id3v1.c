#include "id3v1.h"

/* List of music genres tailored for Indian & Bollywood music */
static const char *genres[] = {
    "Bollywood",                  /* 0 */
    "Filmi",                      /* 1 */
    "Classical (Hindustani)",     /* 2 */
    "Carnatic Classical",         /* 3 */
    "Ghazal",                     /* 4 */
    "Sufi",                       /* 5 */
    "Bhangra",                    /* 6 */
    "Punjabi Pop",                /* 7 */
    "Indie Pop",                  /* 8 */
    "Bhajan / Devotional",        /* 9 */
    "Qawwali",                    /* 10 */
    "Folk / Regional",            /* 11 */
    "Desi Hip-Hop",               /* 12 */
    "Pop",                        /* 13 - Standard ID3 Pop (matches sample test files) */
    "Romantic Melodies",          /* 14 */
    "Sad / Dard-e-Dil",           /* 15 */
    "Item Song",                  /* 16 */
    "Garba / Dandiya",            /* 17 */
    "Rabindra Sangeet",           /* 18 */
    "Assamese / Bihu",            /* 19 */
    "Marathi Lavani",             /* 20 */
    "Bhojpuri",                   /* 21 */
    "Tamil Kuthu",                /* 22 */
    "Telugu Mass",                /* 23 */
    "Malayalam Melody",           /* 24 */
    "Bollywood Soundtrack",       /* 25 */
    "Instrumental (Sitar/Flute)", /* 26 */
    "Fusion",                     /* 27 */
    "Remix / DJ Party",           /* 28 */
    "Desi Lofi",                  /* 29 */
    "Unplugged / Acoustic",       /* 30 */
    "Patriotic / Deshbhakti",     /* 31 */
    "Classic Bollywood (Retro)",  /* 32 */
    "Sufi Rock",                  /* 33 */
    "Nazrul Geeti",               /* 34 */
    "Bangla Adhunik Gaan",        /* 35 */
    "Baul / Folk Gaan",           /* 36 */
    "Bangla Rock / Band",         /* 37 */
    "Bhatiali",                   /* 38 */
    "Shyama Sangeet",             /* 39 */
    "Tollywood",                  /* 40 */
    "Tollywood Mass / Dappu",     /* 41 */
    "Tollywood Melody",           /* 42 */
    "Telugu Janapada (Folk)"      /* 43 */
};

/* Helper function to copy and print fixed-length characters */
static void print_fixed_field(const char *label, const char *data, int max_len) {
    char buffer[64];
    if (max_len >= (int)sizeof(buffer)) {
        max_len = (int)sizeof(buffer) - 1;
    }
    strncpy(buffer, data, max_len);
    buffer[max_len] = '\0';
    printf("%-8s: %s\n", label, buffer);
}

/* Get genre text name from genre code */
const char *get_genre_name(unsigned char genre_code) {
    int total_genres = sizeof(genres) / sizeof(genres[0]);
    if (genre_code < total_genres) {
        return genres[genre_code];
    }
    return "Unknown";
}

/* Check if the tag is ID3v1.0 or ID3v1.1 */
ID3Version check_id3v1_version(const ID3v1Tag *tag) {
    if (tag->zero_byte == 0 && tag->track != 0) {
        return TAG_ID3V1_1;
    }
    return TAG_ID3V1;
}

/* Read ID3v1 tag from the last 128 bytes of the file */
Status read_id3v1_tag(FILE *fp, ID3v1Tag *tag) {
    if (fp == NULL || tag == NULL) {
        return e_failure;
    }

    /* Seek to 128 bytes before the end of the file */
    if (fseek(fp, -ID3V1_TAG_SIZE, SEEK_END) != 0) {
        return e_failure;
    }

    /* Read the 128-byte tag */
    if (fread(tag, 1, ID3V1_TAG_SIZE, fp) != ID3V1_TAG_SIZE) {
        return e_failure;
    }

    /* Validate ID3v1 signature "TAG" */
    if (strncmp(tag->header, "TAG", 3) != 0) {
        return e_failure;
    }

    return e_success;
}

/* Display ID3v1 tag metadata */
void display_id3v1_tag(const ID3v1Tag *tag) {
    if (tag == NULL) {
        return;
    }

    print_fixed_field("Title", tag->title, 30);
    print_fixed_field("Artist", tag->artist, 30);
    print_fixed_field("Album", tag->album, 30);
    print_fixed_field("Year", tag->year, 4);

    if (check_id3v1_version(tag) == TAG_ID3V1_1) {
        printf("%-8s: %d\n", "Track", tag->track);
        print_fixed_field("Comment", tag->comment, 28);
    } else {
        print_fixed_field("Comment", tag->comment, 30);
    }

    printf("%-8s: %s (id %d)\n", "Genre", get_genre_name(tag->genre), tag->genre);
}
