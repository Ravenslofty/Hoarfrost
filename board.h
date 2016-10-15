/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Dan Ravensloft <dan.ravensloft@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BOARD_H
#define BOARD_H

#include <inttypes.h>
#include <stdio.h>

struct Board {
    uint64_t pieces[6];
    uint64_t colors[2];
    unsigned char side;
    unsigned char castle;
    unsigned char ep;
    unsigned char fifty;
};

struct Move {
    unsigned char from:6;
    unsigned char dest:6;
    unsigned char type:3;
    unsigned char prom:3;
    unsigned char color:1;
    unsigned char piece:3;
};

struct Undo {
    char ep;
    char cap;
    char castle;
};

#define COL(x) ((x)&7)
#define ROW(x) ((x)>>3)

#define INVALID 64

enum { WHITE, BLACK, FORCE };
enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum { QUIET, CASTLE, CAPTURE, ENPASSANT, PROMOTION, CAPTURE_PROMOTION, DOUBLE_PUSH };

static const uint64_t FileAMask = 0x0101010101010101ULL;
static const uint64_t FileBMask = 0x0202020202020202ULL;
static const uint64_t FileCMask = 0x0404040404040404ULL;
static const uint64_t FileDMask = 0x0808080808080808ULL;
static const uint64_t FileEMask = 0x1010101010101010ULL;
static const uint64_t FileFMask = 0x2020202020202020ULL;
static const uint64_t FileGMask = 0x4040404040404040ULL;
static const uint64_t FileHMask = 0x8080808080808080ULL;

static const uint64_t Rank1Mask = 0x00000000000000FFULL;
static const uint64_t Rank2Mask = 0x000000000000FF00ULL;
static const uint64_t Rank3Mask = 0x0000000000FF0000ULL;
static const uint64_t Rank4Mask = 0x00000000FF000000ULL;
static const uint64_t Rank5Mask = 0x000000FF00000000ULL;
static const uint64_t Rank6Mask = 0x0000FF0000000000ULL;
static const uint64_t Rank7Mask = 0x00FF000000000000ULL;
static const uint64_t Rank8Mask = 0xFF00000000000000ULL;

#define PRINT_MOVE(m) printf("%c%d%c%d", 'a' + COL (((m).from)&63), 1 + ROW (((m).from)&63), 'a' + COL (((m).dest)&63), 1 + ROW (((m).dest)&63))

#endif // BOARD_H
