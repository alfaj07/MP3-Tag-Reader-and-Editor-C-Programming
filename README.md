# MP3 Tag Reader and Editor (`mp3tag`)

A modular C-based command-line interface (CLI) utility designed to inspect, parse, display, and edit ID3 metadata (ID3v1, ID3v1.1, and ID3v2.3) embedded within MPEG audio (`.mp3`) files.

---

## Table of Contents

- [Overview](#overview)
- [Key Features](#key-features)
- [Supported Specifications & Architecture](#supported-specifications--architecture)
  - [ID3v1 / ID3v1.1](#id3v1--id3v11)
  - [ID3v2.3](#id3v23)
  - [Synchsafe Integer Representation](#synchsafe-integer-representation)
- [Project Directory Structure](#project-directory-structure)
- [Prerequisites & Build Instructions](#prerequisites--build-instructions)
- [CLI Usage & Options](#cli-usage--options)
  - [Command Syntax](#command-syntax)
  - [Viewing Metadata](#viewing-metadata)
  - [Editing Metadata](#editing-metadata)
  - [Help and Version](#help-and-version)
- [Implementation Details](#implementation-details)
  - [Tag Parsing Flow](#tag-parsing-flow)
  - [Tag Editing Mechanism](#tag-editing-mechanism)
- [Error Handling](#error-handling)
- [References](#references)
- [License](#license)

---

## Overview

Audio files often contain metadata such as track title, contributing artists, album name, genre, and release year embedded directly alongside raw audio bitstreams. This project implements a lightweight and robust tool written in C to read and manipulate this metadata according to the official **ID3v2.3.0 informal standard** and classic **ID3v1 / ID3v1.1** specifications.

---

## Key Features

- **Tag Detection & Version Identification**: Automatically identifies ID3 version presence (ID3v1, ID3v1.1, and ID3v2.3).
- **Metadata Viewer**: Extracts and pretty-prints song details:
  - Track Title (`TIT2`)
  - Lead Artist / Performer (`TPE1`)
  - Album Title (`TALB`)
  - Track Number & Total (`TRCK`)
  - Release Year (`TYER`)
  - Content Genre (`TCON`, including ID3v1 index mapping)
  - Comments (`COMM`)
  - Attached Picture info (`APIC` / MIME type, picture type, data length)
- **Tag Modification & Editing**: In-place and temporary-file based metadata updates for custom title, artist, album, year, track, comment, and genre fields.
- **Strict Binary Format Compliance**: Accurate handling of synchsafe integers (7-bit byte encoding) and Big-Endian integer conversions.
- **Robust Error Handling**: Gracefully reports non-existent files, corrupt headers, unsupported tag revisions, and missing tags.

---

## Supported Specifications & Architecture

### ID3v1 / ID3v1.1
- Located at the final **128 bytes** of the audio file.
- Header identifier: `"TAG"` at offset `file_size - 128`.
- Fixed fields:
  - Title: 30 bytes
  - Artist: 30 bytes
  - Album: 30 bytes
  - Year: 4 bytes
  - Comment: 30 bytes (ID3v1) or 28 bytes + 1 byte zero-delimiter + 1 byte Track Number (ID3v1.1)
  - Genre: 1 byte index

### ID3v2.3
- Located at the beginning of the file (offset 0).
- **Header Structure (10 bytes)**:
  - Identifier: `"ID3"` (3 bytes)
  - Version: `$03 00` (2 bytes)
  - Flags: `%abc00000` (1 byte: Unsynchronisation, Extended Header, Experimental)
  - Tag Size: 4 synchsafe bytes (`%0xxxxxxx` $\times 4$, 28 bits total)
- **Frame Structure (10 bytes header + content)**:
  - Frame ID: 4 ASCII characters (e.g., `TIT2`, `TPE1`, `TALB`, `TYER`, `TRCK`, `COMM`, `APIC`)
  - Frame Size: 4-byte 32-bit unsigned Big-Endian integer (excluding 10-byte frame header)
  - Flags: 2 bytes (Status and encoding flags)
  - Frame Body: Begins with encoding byte (`$00` for ISO-8859-1, `$01` for Unicode UTF-16 with BOM), followed by payload data.

### Synchsafe Integer Representation
In ID3v2 headers, sizes avoid MPEG sync word collisions (`0xFFE0` to `0xFFFF`) by setting the 7th bit of every size byte to `0`. A 28-bit synchsafe integer is parsed as follows:

$$Size = (b_0 \ll 21) \,|\, (b_1 \ll 14) \,|\, (b_2 \ll 7) \,|\, b_3$$

---

## Project Directory Structure

```text
mp3_tag_reader/
├── Makefile
├── README.md
├── include/
│   ├── common.h          # Shared status codes, macros, and types
│   ├── id3v1.h           # ID3v1 structures and reader prototypes
│   ├── id3v2.h           # ID3v2 header, frame definitions, and prototypes
│   ├── reader.h          # Tag parsing and display functions
│   └── editor.h          # Tag editing and file rewrite logic
└── src/
    ├── main.c            # CLI arguments parsing and dispatch
    ├── id3v1.c           # ID3v1 implementation
    ├── id3v2.c           # ID3v2 frame handling and synchsafe utilities
    ├── reader.c          # Formatted console output display
    └── editor.c          # Modification routine (frame updating and buffer copy)
```

---

## Prerequisites & Build Instructions

### Prerequisites
- GCC / Clang compiler supporting C
- GNU Make
- Standard C libraries (`stdio.h`, `stdlib.h`, `string.h`, `stdint.h`)

### Compilation
To build the executable, run:

```bash
make
```

To clean compiled binaries and intermediate object files:

```bash
make clean
```

---

## CLI Usage & Options

### Command Syntax

```bash
# View metadata
mp3tag -v <file.mp3>
# (also supports: mp3tag <file.mp3>)

# Modify metadata
mp3tag -e -[tTaAycgm] "value" <file.mp3>
# (also supports: mp3tag -e-[flag] "value" <file.mp3> or mp3tag -[flag] "value" <file.mp3>)

# Information options
mp3tag -h | --help
mp3tag -V
```

### Options Reference

| Flag | Argument | Description | Frame Target |
| :--- | :--- | :--- | :--- |
| `-t` | `<string>` | Modifies Track Title | `TIT2` |
| `-T` | `<number>` | Modifies Track Number | `TRCK` |
| `-a` | `<string>` | Modifies Artist Name | `TPE1` |
| `-A` | `<string>` | Modifies Album Name | `TALB` |
| `-y` | `<number>` | Modifies Year | `TYER` |
| `-c` | `<string>` | Modifies Comment | `COMM` |
| `-g`, `-m` | `<string>` | Modifies Genre / Music | `TCON` |
| `-e` | *Modifier* | Edit flag prefix (e.g., `-e -t "Title" file.mp3`) | — |
| `-v` | `<file.mp3>` | Views MP3 tag details | — |
| `-V` | *None* | Prints application version info | — |
| `-h`, `--help` | *None* | Displays command help and usage | — |

---

## Usage Examples

### 1. Viewing Metadata
```bash
$ ./mp3tag -v sample.mp3
+---------------------------------------------------------------------------------+
|                              SELECTED VIEW DETAILS                              |
+---------------------------------------------------------------------------------+
|                                                                                 |
|                   MP3 TAG READER AND EDITOR FOR ID3v2                            |
|                                                                                 |
+-------------------+-------------------------------------------------------------+
| TAG NAME          | TAG VALUE                                                   |
+-------------------+-------------------------------------------------------------+
| TITLE             | updated-name                                                |
| ARTIST            | aniruth                                                     |
| ALBUM             | new-albu                                                    |
| YEAR              | 1999                                                        |
| MUSIC             | kollywood                                                   |
| COMMENT           | sample-download                                             |
+-------------------+-------------------------------------------------------------+
```

### 2. Updating Metadata
```bash
# Edit Title
./mp3tag -e -t "Yellow" sample.mp3

# Edit Artist
./mp3tag -e -a "Coldplay" sample.mp3

# Edit Album
./mp3tag -e -A "Parachutes" sample.mp3

# Edit Year
./mp3tag -e -y "2000" sample.mp3

# Edit Music / Genre
./mp3tag -e -m "Alternative Rock" sample.mp3

# Edit Comment
./mp3tag -e -c "Great album" sample.mp3
```

### 3. Displaying Help & Error Messages
```bash
$ ./mp3tag
+-------------------------------------------------------------------------------+
|                           ERROR: Missing arguments.                           |
+-------------------------------------------------------------------------------+
|                                                                               |
| USAGE:                                                                        |
|   mp3tag -v [file.mp3]                                                        |
|   mp3tag -[tTaAycg] "value" file1                                             |
|   mp3tag -V                                                                   |
|                                                                               |
+-------------------------------------------------------------------------------+

+-------------------------------------------------------------------------------+
|                                   OPTIONS:                                    |
+-------------------------------------------------------------------------------+
|   -t    Modifies a Title tag                                                  |
|   -T    Modifies a Track tag                                                  |
|   -a    Modifies an Artist tag                                                |
|   -A    Modifies an Album tag                                                 |
|   -y    Modifies a Year tag                                                   |
|   -c    Modifies a Comment tag                                                |
|   -g    Modifies a Genre tag                                                  |
|   -h    Displays this help info                                               |
|   -v    Views tag details                                                     |
|   -V    Prints version info                                                   |
+-------------------------------------------------------------------------------+
```

### 4. Displaying Version Information
```bash
$ ./mp3tag -V
MP3 Tag Reader and Editor version 1.0.0
```

---

## Implementation Details

### Tag Parsing Flow
1. Open the file in binary read mode (`rb`).
2. Read the first 10 bytes and test for `"ID3"`.
3. If found:
   - Check version byte (e.g., `0x03` $\rightarrow$ ID3v2.3).
   - Compute total tag length from synchsafe bytes.
   - Iterate over frame headers until tag boundaries or padding (`0x00`) are reached.
   - Read and decode content per frame ID.
4. If `"ID3"` is absent, seek to `EOF - 128 bytes` and look for `"TAG"` to parse ID3v1/ID3v1.1 data.

### Tag Editing Mechanism
Because changing the contents of a frame often alters its size:
1. The original MP3 file is opened alongside a temporary output file (`temp.mp3`).
2. If the tag already exists, existing frames are selectively replaced or copied.
3. Updated frames (new size, flags, and data) are computed and written to `temp.mp3`.
4. The remaining raw audio stream is copied verbatim to preserve fidelity.
5. `temp.mp3` replaces the source file atomically using standard file operations (`remove` and `rename`).

---

## Error Handling

| Scenario | Application Behavior |
| :--- | :--- |
| **File not found / unreadable** | Emits `Error: Unable to open file '<path>'` and exits with code `1`. |
| **No ID3 tags detected** | Reports `Error: No ID3v1 or ID3v2 tag found in '<path>'`. |
| **Unsupported version (e.g., ID3v2.4)** | Alerts `Error: ID3v2.4 is currently unsupported. Supported versions: ID3v1, ID3v1.1, ID3v2.3.` |
| **Corrupt frame header** | Terminates frame scan cleanly and displays parsed tags up to point of failure. |

---

## References

- [ID3v2.3.0 Informal Standard](https://id3.org/id3v2.3.0) — ID3v2.3.0 Specification and Frame Definitions
- [ID3v1 / ID3v1.1 Specification](https://id3.org/ID3v1) — Official ID3v1 Structure and Genre Mappings
- [ISO/IEC 11172-3](https://www.iso.org/standard/22412.html) — Information technology — Coding of moving pictures and associated audio for digital storage media at up to about 1,5 Mbit/s — Part 3: Audio

---

## License

This project is licensed under the [MIT License](LICENSE) — see the [LICENSE](LICENSE) file for details.
