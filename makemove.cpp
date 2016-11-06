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
#include "functions.h"

static int castle_mask[64] = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
     7, 15, 15, 15,  3, 15, 15, 11
};

void MakeMove(struct Board * b, struct Undo * u, struct Move m)
{
    uint64_t frombb, destbb, tmpbb;
    char epdest;

    char from = m.from & 63;
    char dest = m.dest & 63;
    char type = m.type & 7;
    char prom = m.prom & 7;
    char color = m.color & 1;
    char piece = m.piece & 7;

    frombb = 1ULL << from;
    destbb = 1ULL << dest;

    u->ep = b->ep;
    b->ep = INVALID;

    u->castle = b->castle;
    b->castle &= castle_mask[from] & castle_mask[dest];

    switch (type) {
    case QUIET:
        break;

    case DOUBLE_PUSH:
        if (b->side == WHITE) {
            b->ep = dest - 8;
        } else {
            b->ep = dest + 8;
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
        b->colors[!b->side] ^= destbb;
        break;

    case ENPASSANT:
        if (b->side == WHITE) {
            epdest = dest - 8;
        } else {
            epdest = dest + 8;
        }

        b->pieces[PAWN] ^= 1ULL << epdest;
        b->colors[!b->side] ^= 1ULL << epdest;
        break;

    case CASTLE:
        // Kingside
        if (dest > from) {
            tmpbb = (1ULL << (dest+1)) | (1ULL << (from+1));
        // Queenside
        } else {
            tmpbb = (1ULL << (dest-2)) | (1ULL << (from-1));
        }

        // Move the rook.
        b->pieces[ROOK] ^= tmpbb;
        b->colors[b->side] ^= tmpbb;
        break;

    case PROMOTION:
        // Change the piece type.
        b->pieces[PAWN] ^= destbb;
        b->pieces[prom] ^= destbb;
        break;

    case CAPTURE_PROMOTION:
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

        // Remove the piece.
        b->pieces[u->cap] ^= destbb;
        b->colors[!b->side] ^= destbb;

        // Change the piece type.
        b->pieces[PAWN] ^= destbb;
        b->pieces[prom] ^= destbb;
        break;
    }

    // Move the piece.
    b->pieces[piece] ^= frombb | destbb;
    b->colors[b->side] ^= frombb | destbb;

    b->side ^= 1;
}

void UnmakeMove(struct Board * b, struct Undo * u, struct Move m)
{
    uint64_t frombb, destbb, tmpbb;
    char epdest;

    char from = m.from & 63;
    char dest = m.dest & 63;
    char type = m.type & 7;
    char prom = m.prom & 7;
    char color = m.color & 1;
    char piece = m.piece & 7;

    frombb = 1ULL << from;
    destbb = 1ULL << dest;

    b->side ^= 1;

    switch (type) {
    case QUIET:
        break;

    case DOUBLE_PUSH:
        break;

    case CAPTURE:
        // Add the captured piece.
        b->pieces[u->cap] ^= destbb;
        b->colors[!b->side] ^= destbb;
        break;

    case ENPASSANT:
        // Get the piece location.
        if (b->side == WHITE) {
            epdest = dest - 8;
        } else {
            epdest = dest + 8;
        }

        // Add the captured piece.
        b->pieces[PAWN] ^= 1ULL << epdest;
        b->colors[!b->side] ^= 1ULL << epdest;
        break;

    case CASTLE:
        // Kingside
        if (dest > from) {
            tmpbb = (1ULL << (dest+1)) | (1ULL << (from+1));
        // Queenside
        } else {
            tmpbb = (1ULL << (dest-2)) | (1ULL << (from-1));
        }

        // Move the rook.
        b->pieces[ROOK] ^= tmpbb;
        b->colors[b->side] ^= tmpbb;
        break;

    case PROMOTION:
        // Change the piece type.
        b->pieces[PAWN] ^= destbb;
        b->pieces[prom] ^= destbb;
        break;

    case CAPTURE_PROMOTION:
        // Remove the piece.
        b->pieces[u->cap] ^= destbb;
        b->colors[!b->side] ^= destbb;

        // Change the piece type.
        b->pieces[PAWN] ^= destbb;
        b->pieces[prom] ^= destbb;
        break;
    }

    b->castle = u->castle;

    b->ep = u->ep;

    // Move the piece.
    b->pieces[piece] ^= frombb | destbb;
    b->colors[b->side] ^= frombb | destbb;
}
