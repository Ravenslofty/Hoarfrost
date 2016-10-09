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

struct Board {
    uint64_t pieces[6];
    uint64_t colors[2];
    char side;
    char castle;
    char ep;
    char fifty;
};

struct Move {
    char from:6;
    char dest:6;
    char type:3;
    char prom:3;
    char color:1;
    char piece:3;
};

struct Undo {
    char ep;
    char cap;
};

#define COL(x) ((x)&7)
#define ROW(x) ((x)>>3)

#define INVALID 64

enum { WHITE, BLACK, FORCE };
enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum { QUIET, CASTLE, CAPTURE, ENPASSANT, PROMOTION, CAPTURE_PROMOTION, DOUBLE_PUSH };

#define PRINT_MOVE(m) printf("%c%d%c%d", 'a' + COL (((m).from)&63), 1 + ROW (((m).from)&63), 'a' + COL (((m).dest)&63), 1 + ROW (((m).dest)&63))

#endif // BOARD_H
