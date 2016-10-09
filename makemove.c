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

#include <inttypes.h>

#include "board.h"

void MakeMove(struct Board * b, struct Undo * u, struct Move m)
{
    uint64_t frombb, destbb;

    frombb = 1ULL << (m.from & 63);
    destbb = 1ULL << (m.dest & 63);

    u->ep = b->ep;
    b->ep = INVALID;

    switch (m.type) {
    case QUIET:
        break;

    case DOUBLE_PUSH:
        if (b->side == WHITE) {
            b->ep = m.dest - 8;
        } else {
            b->ep = m.dest + 8;
        }
        break;

    case CAPTURE:
        // This is awkward
        u->cap = INVALID;
        if (destbb & b->pieces[PAWN])
            u->cap = PAWN;
        else if (destbb & b->pieces[KNIGHT])
            u->cap = KNIGHT;
        else if (destbb & b->pieces[BISHOP])
            u->cap = BISHOP;
        else if (destbb & b->pieces[ROOK])
            u->cap = ROOK;
        else if (destbb & b->pieces[QUEEN])
            u->cap = QUEEN;
        else if (destbb & b->pieces[KING])
            u->cap = KING;

        b->pieces[u->cap] ^= destbb;
        b->pieces[!(m.color&1)] ^= destbb;
        break;
    }

    b->pieces[m.piece&7] ^= frombb | destbb;
    b->colors[m.color&1] ^= frombb | destbb;

    b->side ^= 1;
}

void UnmakeMove(struct Board * b, struct Undo * u, struct Move m)
{
    uint64_t frombb, destbb;

    frombb = 1ULL << (m.from & 63);
    destbb = 1ULL << (m.dest & 63);

    b->side ^= 1;

    switch (m.type) {
    case QUIET:
        break;

    case DOUBLE_PUSH:
        break;

    case CAPTURE:
        b->pieces[u->cap] ^= destbb;
        b->pieces[!(m.color&1)] ^= destbb;
        break;
    }

    b->ep = u->ep;

    b->pieces[m.piece&7] ^= frombb | destbb;
    b->colors[m.color&1] ^= frombb | destbb;
}
