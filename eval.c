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

void EvalMaterial(struct Board * b, int * midgame, int * endgame)
{
    int material = 0;

    material += cnt(b->pieces[PAWN] & b->colors[WHITE]) * piecevals[PAWN];
    material -= cnt(b->pieces[PAWN] & b->colors[BLACK]) * piecevals[PAWN];

    material += cnt(b->pieces[KNIGHT] & b->colors[WHITE]) * piecevals[KNIGHT];
    material -= cnt(b->pieces[KNIGHT] & b->colors[BLACK]) * piecevals[KNIGHT];

    material += cnt(b->pieces[BISHOP] & b->colors[WHITE]) * piecevals[BISHOP];
    material -= cnt(b->pieces[BISHOP] & b->colors[BLACK]) * piecevals[BISHOP];

    material += cnt(b->pieces[ROOK] & b->colors[WHITE]) * piecevals[ROOK];
    material -= cnt(b->pieces[ROOK] & b->colors[BLACK]) * piecevals[ROOK];

    material += cnt(b->pieces[QUEEN] & b->colors[WHITE]) * piecevals[QUEEN];
    material -= cnt(b->pieces[QUEEN] & b->colors[BLACK]) * piecevals[QUEEN];

    *midgame += material;
    *endgame += material;
}

void EvalPST(struct Board * b, int * midgame, int * endgame)
{
    uint64_t piecebb;
    int piece;

    for (piece = PAWN; piece <= KING; piece++) {
        piecebb = b->pieces[piece] & b->colors[WHITE];

        while (piecebb) {
            *midgame += pst[piece][0][lsb(piecebb)];
            *endgame += pst[piece][1][lsb(piecebb)];

            piecebb &= piecebb - 1;
        }

        piecebb = b->pieces[piece] & b->colors[BLACK];

        while (piecebb) {
            *midgame -= pst[piece][0][lsb(piecebb)^56];
            *endgame -= pst[piece][1][lsb(piecebb)^56];

            piecebb &= piecebb - 1;
        }
    }
}

int Eval(struct Board * b)
{
    int midgame, endgame, phase, value;

    midgame = 0;
    endgame = 0;

    // Material
    // TODO: incremental update.
    EvalMaterial(b, &midgame, &endgame);

    // PST
    // TODO: incremental update.
    EvalPST(b, &midgame, &endgame);

    // Phase
    // Can be incremental updated?
    phase = 24;

    phase -= cnt(b->pieces[KNIGHT]);
    phase -= cnt(b->pieces[BISHOP]);
    phase -= cnt(b->pieces[ROOK]) << 1;
    phase -= cnt(b->pieces[QUEEN]) << 2;

    phase = (phase * 256 + 12) / 24;

    value = ((midgame * (256 - phase)) + (endgame * phase)) / 256;

    // Side to move
    if (b->side == BLACK)
        value = -value;

    return value;
}
