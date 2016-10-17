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

#include <stdlib.h>

#include "board.h"
#include "functions.h"

static inline int CompareMoves(const void * p1, const void * p2)
{
    if (((struct Move*)p1)->score >  ((struct Move *)p2)->score) return -1;
    if (((struct Move*)p1)->score == ((struct Move *)p2)->score) return 0;
    if (((struct Move*)p1)->score <  ((struct Move *)p2)->score) return +1;
}

void InitSort(struct Sort * s)
{
    s->state = CAPTURES;
    s->movecount = 0;
}

int NextMove(struct Board * b, struct Sort * s, struct Move * m)
{
    switch (s->state) {
    case CAPTURES:
        if (s->movecount == 0) {
            s->movecount = GenerateCaptures(b, s->m);

            qsort(s->m, s->movecount, sizeof(struct Move), CompareMoves);

            if (s->movecount != 0) {
                s->i = 0;
                m = &s->m[0];
                return 1;
            } else {
                s->movecount = 0;
                s->state = QUIETS;
            }
        } else {
            if (s->i < s->movecount) {
                s->i++;
                m = &s->m[s->i];
                return 1;
            } else {
                s->movecount = 0;
                s->state = QUIETS;
            }
        }

    /* Fall through if we don't have a move. */
    case QUIETS:
        if (s->movecount == 0) {
            s->movecount = GenerateQuiets(b, s->m);

            qsort(s->m, s->movecount, sizeof(struct Move), CompareMoves);

            if (s->movecount != 0) {
                m = &s->m[0];
                return 1;
            } else {
                return 0;
            }
        } else {
            if (s->i < s->movecount) {
                s->i++;
                m = &s->m[s->i];
                return 1;
            } else {
                return 0;
            }
        }
    default:
        return 0;
    }

    return 0;
};

int MoveValue(struct Board * b, struct Move m)
{
    static int piecevals[6] = { 100, 300, 300, 500, 900, 2000 };
    int value = 0, cap;

    char dest = m.dest & 63;
    char piece = m.piece & 7;

    uint64_t destbb = 1ULL << dest;

    // MVV/LVA for captures
    if (m.type == CAPTURE || m.type == CAPTURE_PROMOTION) {
        cap = INVALID;
        if (destbb & b->pieces[PAWN])
            cap = PAWN;
        else if (destbb & b->pieces[KNIGHT])
            cap = KNIGHT;
        else if (destbb & b->pieces[BISHOP])
            cap = BISHOP;
        else if (destbb & b->pieces[ROOK])
            cap = ROOK;
        else if (destbb & b->pieces[QUEEN])
            cap = QUEEN;
        else if (destbb & b->pieces[KING])
            cap = KING;

        value += piecevals[cap] - piece;
    }

    // TODO: quiet move sorting.

    return value;
}
