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

#include <algorithm>
#include <array>

#include <stdlib.h>

#include "board.h"
#include "functions.h"

static inline int CompareMoves(const void * p1, const void * p2)
{
    if (((struct Move*)p1)->score >  ((struct Move *)p2)->score) return -1;
    if (((struct Move*)p1)->score == ((struct Move *)p2)->score) return 0;
    if (((struct Move*)p1)->score <  ((struct Move *)p2)->score) return +1;
}

void InitSort(struct Board * b, struct Sort * s, struct Move ttm)
{
    s->movecount = GenerateCaptures(b, s->m.data(), 0);
    s->movecount = GenerateQuiets(b, s->m.data(), s->movecount);

    if (ttm.from != ttm.dest) {
        for (s->i = 0; s->i < s->movecount; s->i++) {
            if (s->m[s->i].from == ttm.from &&
                    s->m[s->i].dest == ttm.dest &&
                    s->m[s->i].type == ttm.type) {
                s->m[s->i].score = 32000;
                break;
            }
        }
    }

    s->i = 0;
}

void InitSortQuies(struct Board * b, struct Sort * s)
{
    s->movecount = GenerateCaptures(b, s->m.data(), 0);

    s->i = 0;
}

int NextMove(struct Board * b, struct Sort * s, struct Move * m)
{
    if (s->i < s->movecount) {
        int best = s->i, bestvalue = s->m[s->i].score;
        while (true) {
            /* Select best */
            for (int i = s->i + 1; i < s->movecount; i++) {
                if (s->m[i].score > bestvalue) {
                    best = i;
                    bestvalue = s->m[i].score;
                }
            }
    
            /* Defer if actually a bad capture. */
            if (s->m[best].type == CAPTURE &&
                s->m[best].score > 10000 &&
                s->m[best].score < 30000 &&
                SEE(b, s->m[best].from, s->m[best].dest, piece(b, s->m[best].dest), s->m[best].piece) < 0) {
                s->m[best].score -= 20000;
                continue;
            }

            break;
        }
        
        /* Swap */
        struct Move bestmove = s->m[best];
        s->m[best] = s->m[s->i];
        s->m[s->i] = bestmove;

        *m = s->m[s->i];
        s->i++;
        return 1;
    }
    return 0;
};

int MoveValue(struct Board * b, struct Move m)
{
    int value = 0, cap;

    char from = m.from & 63;
    char dest = m.dest & 63;
    char piece = m.piece & 7;

    uint64_t destbb = 1ULL << dest;

    // PST difference as base move score.
    value = pst[piece][0][dest] - pst[piece][0][from];

    cap = NO_PIECE;
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

    if (cap != NO_PIECE) {
        value += 20000;
        value += piecevals[cap][0] - piece;
    }

    return value;
}
