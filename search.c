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

#include <string.h>

#include "board.h"
#include "functions.h"

int nodes;

int Quies(struct Board * b, int alpha, int beta)
{
    struct Move m;
    struct Sort s;
    struct Undo u;

    int val;

    nodes++;

    val = Eval(b);

    if (val >= beta)
        return beta;
    if (val > alpha)
        alpha = val;

    InitSortQuies(b, &s);

    while (NextMove(&s, &m)) {
        MakeMove(b, &u, m);

        if (IsIllegal(b)) {
            UnmakeMove(b, &u, m);
            continue;
        }

        val = -Quies(b, -beta, -alpha);

        UnmakeMove(b, &u, m);

        if (val >= beta)
            return beta;

        if (val > alpha)
            alpha = val;
    }

    return alpha;
}

int first, cuts;

int Search(struct Board * b, int depth, int alpha, int beta, int ply, struct PV * pv)
{
    struct Move m, bestmove;
    struct Sort s;
    struct Undo u;
    struct PV childpv;
    int pvnode = (alpha == beta-1);

    int val, moves = 0;

    int flag = hashfALPHA;

    nodes++;

    if (depth == 0) {
        pv->count = 0;
        return Quies(b, alpha, beta);
    }

    m.from = m.dest = 0;
    bestmove.from = bestmove.dest = 0;

    // Hash probe
    CalculateHash(b);

    if ((val = ReadTT(b, &m, depth, alpha, beta, ply)) != 11000) {
        if (!pvnode) {
            pv->count = 0;
            return val;
        }
    }

    InitSort(b, &s, m);

    while (NextMove(&s, &m)) {
        MakeMove(b, &u, m);

        if (IsIllegal(b)) {
            UnmakeMove(b, &u, m);
            continue;
        }

        moves++;

        if (flag == hashfALPHA)
            val = -Search(b, depth - 1, -beta, -alpha, ply + 1, &childpv);
        else {
            val = -Search(b, depth - 1, -alpha-1, -alpha, ply + 1, &childpv);
            if (val > alpha && val < beta) {
                val = -Search(b, depth - 1, -beta, -alpha, ply + 1, &childpv);
            }
        }

        UnmakeMove(b, &u, m);

        if (val >= beta) {
            if (moves == 1)
                first++;
            cuts++;

            WriteTT(b, depth, val, hashfBETA, m, ply);

            return beta;
        }

        if (val > alpha) {
            alpha = val;

            pv->moves[0] = m;
            memcpy(pv->moves + 1, childpv.moves, childpv.count * sizeof(struct Move));
            pv->count = childpv.count + 1;

            bestmove = m;
            flag = hashfEXACT;
        }
    }

    if (!moves) {
        if (IsInCheck(b)) {
            return -MATE + ply;
        } else {
            return 0;
        }
    }

    WriteTT(b, depth, alpha, flag, bestmove, ply);

    return alpha;
}
