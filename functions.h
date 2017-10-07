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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdbool.h>
#include <stdint.h>

#include "board.h"

static inline int msb(uint64_t b) {
    return 63 ^ __builtin_clzll(b);
}

static inline int lsb(uint64_t b) {
    return __builtin_ctzll(b);
}

// magic.c
extern void InitMagics();
extern uint64_t PawnAttacks(const int side, const int sq);
extern uint64_t KnightAttacks(const int sq);
extern uint64_t BishopAttacks(const int sq, const uint64_t occ);
extern uint64_t RookAttacks(const int sq, const uint64_t occ);
extern uint64_t QueenAttacks(const int sq, const uint64_t occ);
extern uint64_t KingAttacks(const int sq);

// attacked.c
extern bool IsAttacked(struct Board * b, int side, int square);
extern bool IsIllegal(struct Board * b);
extern bool IsInCheck(struct Board * b);

// fen.c
extern void ClearBoard(struct Board * b);
extern void ParseFEN(struct Board * b, char * fen);

// makemove.c
extern void MakeMove(struct Board * b, struct Undo * u, struct Move m);
extern void UnmakeMove(struct Board * b, struct Undo * u, struct Move m);

// movegen.c
extern int GenerateQuiets(struct Board * b, struct Move * m, int movecount);
extern int GenerateCaptures(struct Board * b, struct Move * m, int movecount);

// movesort.c
extern void InitSort(struct Board * b, struct Sort * s);
extern int NextMove(struct Sort * s, struct Move * m);
extern int MoveValue(struct Board * b, struct Move m);

// perft.c
extern uint64_t Perft(struct Board * b, int depth);
extern uint64_t Divide(struct Board * b, int depth);

#endif
