/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Dan Ravensloft <dan.ravensloft@gmail.cppom>
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

static inline int cnt(uint64_t b) {
    return __builtin_popcountll(b);
}

static inline int swap(uint64_t b) {
    return __builtin_bswap64(b);
}

static inline int max(int a, int b)
{
    return (a > b) ? a : b;
}

static inline int min(int a, int b)
{
    return (a < b) ? a : b;
}

// attacked.cpp
extern bool IsAttacked(struct Board * b, int side, int square);
extern bool IsIllegal(struct Board * b);
extern bool IsInCheck(struct Board * b);

// eval.cpp
extern int Eval(struct Board * b);

// fen.cpp
extern void ClearBoard(struct Board * b);
extern void ParseFEN(struct Board * b, char * fen);

// main.cpp
extern int ReadClock();

// makemove.cpp
extern void MakeMove(struct Board * b, struct Undo * u, struct Move m);
extern void UnmakeMove(struct Board * b, struct Undo * u, struct Move m);

// magic.cpp
extern void InitMagics();
extern uint64_t PawnAttacks(const int side, const int sq);
extern uint64_t KnightAttacks(const int sq);
extern uint64_t BishopAttacks(const int sq, const uint64_t occ);
extern uint64_t RookAttacks(const int sq, const uint64_t occ);
extern uint64_t QueenAttacks(const int sq, const uint64_t occ);
extern uint64_t KingAttacks(const int sq);

// movegen.cpp
extern int GenerateQuiets(struct Board * b, struct Move * m, int movecount);
extern int GenerateCaptures(struct Board * b, struct Move * m, int movecount);

// movesort.cpp
extern void InitSort(struct Board * b, struct Sort * s, struct Move ttm);
extern void InitSortQuies(struct Board * b, struct Sort * s);
extern int NextMove(struct Sort * s, struct Move * m);
extern int MoveValue(struct Board * b, struct Move m);

extern void ClearHistory();
extern void ReduceHistory();
extern void UpdateHistory(struct Sort * s, int depth);

// perft.cpp
extern uint64_t Perft(struct Board * b, int depth);
extern uint64_t Divide(struct Board * b, int depth);

// search.cpp
extern int Quies(struct Board * b, int alpha, int beta);
extern int Search(struct Board * b, int depth, int alpha, int beta, int ply, struct PV * pv);

// see.cpp
extern int SEE(struct Board * b, int from, int to, int cap, int att);

// tt.cpp
extern void ResizeTT(int megabytes);
extern void ClearTT();
extern int ReadTT(struct Board * b, struct Move * m, int depth, int alpha, int beta, int ply);
extern void WriteTT(struct Board * b, int depth, int val, int hashf, struct Move m, int ply);

// tune.cpp
extern void LoadTests();
extern void LoadEval();
extern void OptimiseEval();

// zobrist.cpp
extern void InitZobrist();
extern void CalculateHash(struct Board * b);

#endif
