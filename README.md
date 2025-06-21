# Hamming Code (31, 26) File Encoder/Decoder

This project provides a command-line utility written in C to encode and decode files using the Hamming (31, 26) error-correcting code.

This implementation reads a file, breaks it down into 26-bit data blocks, and encodes each one into a 31-bit block by adding 5 parity bits. The resulting encoded file can be transmitted or stored. The decoder can then read this file, correct any single-bit error within each 31-bit block, and reconstruct the original file.

## Features

- **File Encoding**: Converts any file into a Hamming (31, 26) encoded format.
- **File Decoding**: Reads an encoded file and restores the original data.
- **Single-Bit Error Correction**: Automatically detects and corrects one-bit errors in every 31-bit block of the encoded data during the decoding process.

## How It Works

The program operates in one of two modes: encode or decode.

1.  **Encoding (`-e`)**:
    -   The input file is read byte by byte.
    -   Bits are grouped into 26-bit data blocks (`BLOCK_SIZE`).
    -   For each block, the `hamming_31_26_encode` function calculates 5 parity bits and inserts them at positions that are powers of two (1, 2, 4, 8, 16).
    -   The resulting 31-bit coded blocks (`CODED_SIZE`) are written to the output file.

2.  **Decoding (`-d`)**:
    -   The encoded input file is read in 31-bit blocks.
    -   For each block, the `hamming_31_26_decode` function recalculates the parity bits to generate a "syndrome" value.
    -   If the syndrome is non-zero, it indicates the position of the bit that is in error. The program flips this bit to correct it.
    -   The parity bits are then removed, and the original 26-bit data block is recovered.
    -   The data blocks are reassembled and written to the output file to restore the original content.
