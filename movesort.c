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

static int history[64][64];
static int butterfly[64][64];

void ClearHistory()
{
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            history[i][j] = 0;
            butterfly[i][j] = 0;
        }
    }
}

void ReduceHistory()
{
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            history[i][j] >>= 8;
            butterfly[i][j] >>= 8;
        }
    }
}

void UpdateHistory(struct Sort * s, int depth)
{
    int i;

    /* This move failed high, increase the score for it. */
    if ((s->m[s->i].type&7) != CAPTURE && (s->m[s->i].type&7) != CAPTURE_PROMOTION)
        history[s->m[s->i].from&63][s->m[s->i].dest&63] += depth*depth;

    /* The others we searched didn't, decrease the score for them. */
    for (i = 0; i < s->i; i++) {
        /* But only if they aren't a capture. */
        if (s->m[i].type&7 != CAPTURE && s->m[i].type&7 != CAPTURE_PROMOTION)
            butterfly[s->m[i].from&63][s->m[i].dest&63] += 1;
    }
}

static inline int CompareMoves(const void * p1, const void * p2)
{
    if (((struct Move*)p1)->score >  ((struct Move *)p2)->score) return -1;
    if (((struct Move*)p1)->score == ((struct Move *)p2)->score) return 0;
    if (((struct Move*)p1)->score <  ((struct Move *)p2)->score) return +1;
}

void InitSort(struct Board * b, struct Sort * s, struct Move ttm)
{
    s->movecount = GenerateCaptures(b, s->m, 0);
    s->movecount = GenerateQuiets(b, s->m, s->movecount);

    if (ttm.from != ttm.dest) {
        for (s->i = 0; s->i < s->movecount; s->i++) {
            if (s->m[s->i].from == ttm.from &&
                s->m[s->i].dest == ttm.dest &&
                s->m[s->i].type == ttm.type) {
                s->m[s->i].score = 4000;
                break;
            }
        }
    }

    qsort(s->m, s->movecount, sizeof(struct Move), CompareMoves);

    s->i = 0;
}

void InitSortQuies(struct Board * b, struct Sort * s)
{
    s->movecount = GenerateCaptures(b, s->m, 0);

    qsort(s->m, s->movecount, sizeof(struct Move), CompareMoves);

    s->i = 0;
}

int NextMove(struct Sort * s, struct Move * m)
{
    if (s->i < s->movecount) {
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

    if (m.type == CAPTURE || m.type == CAPTURE_PROMOTION) {
        // MVV/LVA for captures
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

        // Put captures at front of move list.
        value += 2000;
    } else {
        // PST difference for quiet moves.
        if (b->side == WHITE) {
            value = pst[piece][0][dest] - pst[piece][0][from];
        } else {
            value = pst[piece][0][dest^56] - pst[piece][0][from^56];
        }
    }

    return value;
}
