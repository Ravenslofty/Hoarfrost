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

#include <assert.h>
#include <stdint.h>

#include "board.h"
#include "functions.h"

static uint64_t GetXRays(struct Board * b, uint64_t occ, int sq)
{
    assert(b != NULL);
    assert(sq >= 0 && sq <= 63);

    uint64_t rooksqueens = (b->rooks() | b->queens()) & occ;
    uint64_t bishsqueens = (b->bishops() | b->queens()) & occ;

    rooksqueens &= RookAttacks(sq, occ);
    bishsqueens &= BishopAttacks(sq, occ);

    return rooksqueens | bishsqueens;
}

static uint64_t AttacksTo(struct Board * b, int square, int side)
{
    assert(b != NULL);
    assert(square >= 0 && square <= 63);
    assert(side == WHITE || side == BLACK);

    uint64_t pawns, knights, bishopsqueens, rooksqueens, kings;

    pawns = b->pawns() & b->colors[side];
    pawns &= PawnAttacks(!side, square);

    knights = b->knights() & b->colors[side];
    knights &= KnightAttacks(square);

    kings = b->kings() & b->colors[side];
    kings &= KingAttacks(square);

    bishopsqueens = (b->bishops() | b->queens()) & b->colors[side];
    bishopsqueens &= BishopAttacks(square, (b->colors[WHITE] | b->colors[BLACK]));

    rooksqueens = (b->rooks() | b->queens()) & b->colors[side];
    rooksqueens &= RookAttacks(square, (b->colors[WHITE] | b->colors[BLACK]));

    return pawns | knights | bishopsqueens | rooksqueens | kings;
}

/* Most of this comes from the CPW - thanks Gerd! */
static uint64_t GetLeastValuablePiece(struct Board * b, uint64_t bb, int colour, int * piece)
{
    for (*piece = PAWN; *piece <= KING; *piece += 1) {
        uint64_t subset = bb & b->colors[colour] & b->pieces[*piece];
        if (subset) {
            return subset & -subset;
        }
    }
    return 0;
}

int SEE(struct Board * b, int from, int to, int cap, int att)
{
   assert(b != NULL);
   assert(from >= 0 && from <= 63);
   assert(to >= 0 && to <= 63);
   assert(cap >= PAWN && cap <= NO_PIECE);
   assert(att >= PAWN && att <= KING);

   if (from == to)
      return 0;

   static int seevals[7] = {1, 3, 3, 5, 9, 20, 0 };

   int gain[32];
   unsigned int d = 0;
   uint64_t mayXray = b->pieces[PAWN] | b->pieces[BISHOP] | b->pieces[ROOK] | b->pieces[QUEEN];
   uint64_t fromSet = 1ULL << from;
   uint64_t occ     = b->colors[WHITE] | b->colors[BLACK];
   uint64_t attadef = AttacksTo(b, to, WHITE) | AttacksTo(b, to, BLACK);
   gain[d]     = seevals[cap];

   do {
      d++; // next depth and side
      gain[d]  = seevals[att] - gain[d-1]; // speculative store, if defended
      if (max (-gain[d-1], gain[d]) < 0) break; // pruning does not influence the result
      attadef ^= fromSet; // reset bit in set to traverse
      occ     ^= fromSet; // reset bit in temporary occupancy (for x-Rays)
      if ( fromSet & mayXray )
         attadef |= GetXRays(b, occ, to);
      fromSet  = GetLeastValuablePiece (b, attadef, (d + b->side) & 1, &att);
   } while (fromSet && d < 32);

   while ((--d) > 0) {
      gain[d-1] = -max (-gain[d-1], gain[d]);
   }

   return gain[0];
}
