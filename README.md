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
mp3tag <file.mp3>

# Modify metadata
mp3tag -[tTaAycg] "value" <file.mp3>

# Information options
mp3tag -h | --help
mp3tag -v | -V
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
| `-g` | `<string>` | Modifies Genre | `TCON` |
| `-h`, `--help` | *None* | Displays command help and usage | — |
| `-v`, `-V` | *None* | Displays application version info | — |

---

## Usage Examples

### 1. Viewing Metadata
```bash
$ ./mp3tag sample.mp3
==================================================
              MP3 TAG DETAILS
==================================================
File: sample.mp3
Version: ID3v2.3
--------------------------------------------------
Title   : Sunny Sunny
Artist  : Yo Yo Honey Singh
Album   : Yaariyan
Track   : 1/10
Year    : 2013
Genre   : Pop (id 13)
Comment : [Description: Comment] [Lang: eng] Downloaded From Internet
APIC    : [image/jpeg] Cover (front), 45210 bytes
==================================================
```

### 2. Updating Artist and Title
```bash
./mp3tag -a "Coldplay" sample.mp3
./mp3tag -t "Yellow" sample.mp3
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

- [ID3v2.3.0 Standard Specification (id3.org)](http://www.id3.org/id3v2.3.0.txt)
- [ID3 Tag Wikipedia Overview](http://en.wikipedia.org/wiki/ID3)
- [MPEG Audio Framing Information (ISO/IEC 11172-3)](https://www.iso.org/)