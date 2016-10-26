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

#include <stdint.h>

#include "board.h"
#include "functions.h"

uint64_t zobrist_piece[2][6][64];
uint64_t zobrist_side;
uint64_t zobrist_castle[16];
uint64_t zobrist_ep[8];

static uint64_t rngseed = 0xd6fa57e9aea945e8ULL; /* 8 bytes from random.org */

uint64_t xorshift64star()
{
    rngseed ^= rngseed >> 12; // a
    rngseed ^= rngseed << 25; // b
    rngseed ^= rngseed >> 27; // c
    return rngseed * 2685821657736338717ULL;
}

void InitZobrist()
{
    int i, j, k;

    for (i = WHITE; i <= BLACK; i++) {
        for (j = PAWN; j <= KING; j++) {
            for (k = 0; k < 64; k++) {
                zobrist_piece[i][j][k] = xorshift64star();
            }
        }
    }

    zobrist_side = xorshift64star();

    for (i = 0; i < 16; i++) {
        zobrist_castle[i] = xorshift64star();
    }

    for (i = 0; i < 8; i++) {
        zobrist_ep[i] = xorshift64star();
    }
}

void CalculateHash(struct Board * b)
{
    int colour, piece;

    uint64_t hash = 0, piecebb;

    for (colour = WHITE; colour <= BLACK; colour++) {
        for (piece = PAWN; piece <= KING; piece++) {
            piecebb = b->pieces[piece] & b->colors[colour];

            while (piecebb) {
                hash ^= zobrist_piece[colour][piece][lsb(piecebb)];

                piecebb &= piecebb - 1;
            }
        }
    }

    if (b->side == BLACK) {
        hash ^= zobrist_side;
    }

    hash ^= zobrist_castle[b->castle];

    if (b->ep != INVALID && b->ep >= 0 && b->ep <= 63) {
        hash ^= zobrist_ep[COL(b->ep)];
    }

    b->hash = hash;
}
