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

int Eval(struct Board * b)
{
    int value;

    value = 0;

    // Material
    value += __builtin_popcountll(b->pieces[PAWN] & b->colors[WHITE]) * piecevals[PAWN];
    value -= __builtin_popcountll(b->pieces[PAWN] & b->colors[BLACK]) * piecevals[PAWN];

    value += __builtin_popcountll(b->pieces[KNIGHT] & b->colors[WHITE]) * piecevals[KNIGHT];
    value -= __builtin_popcountll(b->pieces[KNIGHT] & b->colors[BLACK]) * piecevals[KNIGHT];

    value += __builtin_popcountll(b->pieces[BISHOP] & b->colors[WHITE]) * piecevals[BISHOP];
    value -= __builtin_popcountll(b->pieces[BISHOP] & b->colors[BLACK]) * piecevals[BISHOP];

    value += __builtin_popcountll(b->pieces[ROOK] & b->colors[WHITE]) * piecevals[ROOK];
    value -= __builtin_popcountll(b->pieces[ROOK] & b->colors[BLACK]) * piecevals[ROOK];

    value += __builtin_popcountll(b->pieces[QUEEN] & b->colors[WHITE]) * piecevals[QUEEN];
    value -= __builtin_popcountll(b->pieces[QUEEN] & b->colors[BLACK]) * piecevals[QUEEN];

    // Side to move
    if (b->side == BLACK)
        value = -value;

    return value;
}
