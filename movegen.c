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
#include <stdio.h>

#include "board.h"
#include "functions.h"

static inline void AddMove(struct Board * b, struct Move * m, int * movecount, int from, int dest, int type, int prompiece, int color, int piece)
{
    struct Move n;
    n.from = from;
    n.dest = dest;
    n.type = type;
    n.prom = prompiece;
    n.color = color;
    n.piece = piece;
    n.score = MoveValue(b, n);

    m[*movecount] = n;
    *movecount = *movecount + 1;
}

int GenerateQuiets(struct Board * b, struct Move * m)
{
    uint64_t pawns, knights, bishops, rooks, queens, kings;
    uint64_t singles, doubles, attacks;
    uint64_t occ, empty;
    int from, dest, movecount;

    movecount = 0;
    occ = b->colors[WHITE] | b->colors[BLACK];
    empty = ~occ;

    // Pawns
    if (b->side == WHITE) {
        pawns = b->pieces[PAWN] & b->colors[WHITE];

        // Single push
        singles = (pawns << 8) & empty;

        // Separate promotions
        singles &= ~Rank8Mask;

        while (singles) {
            dest = lsb(singles);

            AddMove(b, m, &movecount, dest - 8, dest, QUIET, INVALID, WHITE, PAWN);

            singles &= singles - 1;
        }

        // Double push
        singles = ((pawns & Rank2Mask) << 8) & empty;
        doubles = (singles << 8) & empty;

        while (doubles) {
            dest = lsb(doubles);

            AddMove(b, m, &movecount, dest - 16, dest, DOUBLE_PUSH, INVALID, WHITE, PAWN);

            doubles &= doubles - 1;
        }

        // Promotions
        singles = ((pawns & Rank7Mask) >> 8) & empty;

        while (singles) {
            dest = lsb(singles);

            AddMove(b, m, &movecount, dest - 8, dest, PROMOTION, QUEEN, WHITE, PAWN);
            AddMove(b, m, &movecount, dest - 8, dest, PROMOTION, ROOK, WHITE, PAWN);
            AddMove(b, m, &movecount, dest - 8, dest, PROMOTION, BISHOP, WHITE, PAWN);
            AddMove(b, m, &movecount, dest - 8, dest, PROMOTION, KNIGHT, WHITE, PAWN);


            singles &= singles - 1;
        }
    } else {
        pawns = b->pieces[PAWN] & b->colors[BLACK];

        // Single push
        singles = (pawns >> 8) & empty;

        // Separate promotions
        singles &= ~Rank1Mask;

        while (singles) {
            dest = lsb(singles);
            AddMove(b, m, &movecount, dest + 8, dest, QUIET, INVALID, BLACK, PAWN);
            singles &= singles - 1;
        }

        // Double push
        singles = ((pawns & Rank7Mask) >> 8) & empty;
        doubles = (singles >> 8) & empty;

        while (doubles) {
            dest = lsb(doubles);
            AddMove(b, m, &movecount, dest + 16, dest, DOUBLE_PUSH, INVALID, BLACK, PAWN);
            doubles &= doubles - 1;
        }

        // Promotions
        singles = ((pawns & Rank2Mask) >> 8) & empty;

        while (singles) {
            dest = lsb(singles);

            AddMove(b, m, &movecount, dest + 8, dest, PROMOTION, QUEEN, BLACK, PAWN);
            AddMove(b, m, &movecount, dest + 8, dest, PROMOTION, ROOK, BLACK, PAWN);
            AddMove(b, m, &movecount, dest + 8, dest, PROMOTION, BISHOP, BLACK, PAWN);
            AddMove(b, m, &movecount, dest + 8, dest, PROMOTION, KNIGHT, BLACK, PAWN);


            singles &= singles - 1;
        }
    }

    // Knights
    knights = b->pieces[KNIGHT] & b->colors[b->side];

    while (knights) {
        from = lsb(knights);

        attacks = KnightAttacks(from) & empty;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, from, dest, QUIET, INVALID, b->side, KNIGHT);

            attacks &= attacks - 1;
        }

        knights &= knights - 1;
    }

    // Bishops
    bishops = b->pieces[BISHOP] & b->colors[b->side];

    while (bishops) {
        from = lsb(bishops);

        attacks = BishopAttacks(from, occ) & empty;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, from, dest, QUIET, INVALID, b->side, BISHOP);

            attacks &= attacks - 1;
        }

        bishops &= bishops - 1;
    }

    // Rooks
    rooks = b->pieces[ROOK] & b->colors[b->side];

    while (rooks) {
        from = lsb(rooks);

        attacks = RookAttacks(from, occ) & empty;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, from, dest, QUIET, INVALID, b->side, ROOK);

            attacks &= attacks - 1;
        }

        rooks &= rooks - 1;
    }

    // Queens
    queens = b->pieces[QUEEN] & b->colors[b->side];

    while (queens) {
        from = lsb(queens);

        attacks = QueenAttacks(from, occ) & empty;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, from, dest, QUIET, INVALID, b->side, QUEEN);

            attacks &= attacks - 1;
        }

        queens &= queens - 1;
    }

    // Kings
    kings = b->pieces[KING] & b->colors[b->side];

    while (kings) {
        from = lsb(kings);

        attacks = KingAttacks(from) & empty;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, from, dest, QUIET, INVALID, b->side, KING);

            attacks &= attacks - 1;
        }

        kings &= kings - 1;
    }

    // Castling - can't castle out of check
    if (!IsInCheck(b)) {

        from = lsb(b->pieces[KING] & b->colors[b->side]);

        if (b->castle & (1 << (2*(b->side == BLACK)))) {
            /* Can't castle through check */
            if (!IsAttacked(b,!b->side,from+1) && !IsAttacked(b,!b->side,from+2) &&
                    ((1ULL << (from+1)) & empty) && ((1ULL << (from+2)) & empty)) {
                AddMove(b, m, &movecount, from, from + 2, CASTLE, INVALID, b->side, KING);
            }
        }

        if (b->castle & (2 << (2*(b->side == BLACK)))) {
            if (!IsAttacked(b,!b->side,from-1) && !IsAttacked(b,!b->side,from-2) &&
                    ((1ULL << (from-1)) & empty) && ((1ULL << (from-2)) & empty) && ((1ULL << (from-3)) & empty)) {
                AddMove(b, m, &movecount, from, from - 2, CASTLE, INVALID, b->side, KING);
            }
        }
    }

    return movecount;
}

int GenerateCaptures(struct Board * b, struct Move * m)
{
    uint64_t pawns, knights, bishops, rooks, queens, kings;
    uint64_t attacks;
    uint64_t occ, empty;
    int from, dest, movecount;

    movecount = 0;
    occ = b->colors[WHITE] | b->colors[BLACK];
    empty = ~occ;

    // Pawns
    if (b->side == WHITE) {
        pawns = b->pieces[PAWN] & b->colors[WHITE];

        // Left captures
        attacks = ((pawns & ~FileAMask) << 7) & b->colors[BLACK] & ~Rank8Mask;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, dest - 7, dest, CAPTURE, INVALID, WHITE, PAWN);

            attacks &= attacks - 1;
        }

        // Left capture-promotions
        attacks = ((pawns & ~FileAMask) << 7) & b->colors[BLACK] & Rank8Mask;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, dest - 7, dest, CAPTURE_PROMOTION, QUEEN, WHITE, PAWN);
            AddMove(b, m, &movecount, dest - 7, dest, CAPTURE_PROMOTION, ROOK, WHITE, PAWN);
            AddMove(b, m, &movecount, dest - 7, dest, CAPTURE_PROMOTION, BISHOP, WHITE, PAWN);
            AddMove(b, m, &movecount, dest - 7, dest, CAPTURE_PROMOTION, KNIGHT, WHITE, PAWN);

            attacks &= attacks - 1;
        }

        // Right captures
        attacks = ((pawns & ~FileHMask) << 9) & b->colors[BLACK] & ~Rank8Mask;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, dest - 9, dest, CAPTURE, INVALID, WHITE, PAWN);

            attacks &= attacks - 1;
        }

        // Right capture-promotions
        attacks = ((pawns & ~FileHMask) << 9) & b->colors[BLACK] & Rank8Mask;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, dest - 9, dest, CAPTURE_PROMOTION, QUEEN, WHITE, PAWN);
            AddMove(b, m, &movecount, dest - 9, dest, CAPTURE_PROMOTION, ROOK, WHITE, PAWN);
            AddMove(b, m, &movecount, dest - 9, dest, CAPTURE_PROMOTION, BISHOP, WHITE, PAWN);
            AddMove(b, m, &movecount, dest - 9, dest, CAPTURE_PROMOTION, KNIGHT, WHITE, PAWN);

            attacks &= attacks - 1;
        }

        // En passant
        if (b->ep != INVALID) {
            attacks = PawnAttacks(BLACK, b->ep) & b->pieces[PAWN] & b->colors[WHITE];

            while (attacks) {
                from = lsb(attacks);

                AddMove(b, m, &movecount, from, b->ep, ENPASSANT, INVALID, WHITE, PAWN);

                attacks &= attacks - 1;
            }
        }
    } else {
        pawns = b->pieces[PAWN] & b->colors[BLACK];

        // Left captures
        attacks = ((pawns & ~FileAMask) >> 9) & b->colors[WHITE] & ~Rank1Mask;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, dest + 9, dest, CAPTURE, INVALID, BLACK, PAWN);

            attacks &= attacks - 1;
        }

        // Left capture-promotions
        attacks = ((pawns & ~FileAMask) >> 9) & b->colors[WHITE] & Rank1Mask;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, dest + 9, dest, CAPTURE_PROMOTION, QUEEN, BLACK, PAWN);
            AddMove(b, m, &movecount, dest + 9, dest, CAPTURE_PROMOTION, ROOK, BLACK, PAWN);
            AddMove(b, m, &movecount, dest + 9, dest, CAPTURE_PROMOTION, BISHOP, BLACK, PAWN);
            AddMove(b, m, &movecount, dest + 9, dest, CAPTURE_PROMOTION, KNIGHT, BLACK, PAWN);

            attacks &= attacks - 1;
        }

        // Right captures
        attacks = ((pawns & ~FileHMask) >> 7) & b->colors[WHITE] & ~Rank1Mask;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, dest + 7, dest, CAPTURE, INVALID, BLACK, PAWN);

            attacks &= attacks - 1;
        }

        // Right capture-promotions
        attacks = ((pawns & ~FileHMask) >> 7) & b->colors[WHITE] & Rank1Mask;

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, dest + 7, dest, CAPTURE_PROMOTION, QUEEN, BLACK, PAWN);
            AddMove(b, m, &movecount, dest + 7, dest, CAPTURE_PROMOTION, ROOK, BLACK, PAWN);
            AddMove(b, m, &movecount, dest + 7, dest, CAPTURE_PROMOTION, BISHOP, BLACK, PAWN);
            AddMove(b, m, &movecount, dest + 7, dest, CAPTURE_PROMOTION, KNIGHT, BLACK, PAWN);

            attacks &= attacks - 1;
        }

        // En passant
        if (b->ep != INVALID) {
            attacks = PawnAttacks(WHITE, b->ep) & b->pieces[PAWN] & b->colors[BLACK];

            while (attacks) {
                from = lsb(attacks);

                AddMove(b, m, &movecount, from, b->ep, ENPASSANT, INVALID, BLACK, PAWN);

                attacks &= attacks - 1;
            }
        }
    }

    // Knights
    knights = b->pieces[KNIGHT] & b->colors[b->side];

    while (knights) {
        from = lsb(knights);

        attacks = KnightAttacks(from) & b->colors[!b->side];

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, from, dest, CAPTURE, INVALID, b->side, KNIGHT);

            attacks &= attacks - 1;
        }

        knights &= knights - 1;
    }

    // Bishops
    bishops = b->pieces[BISHOP] & b->colors[b->side];

    while (bishops) {
        from = lsb(bishops);

        attacks = BishopAttacks(from, occ) & b->colors[!b->side];

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, from, dest, CAPTURE, INVALID, b->side, BISHOP);

            attacks &= attacks - 1;
        }

        bishops &= bishops - 1;
    }

    // Rooks
    rooks = b->pieces[ROOK] & b->colors[b->side];

    while (rooks) {
        from = lsb(rooks);

        attacks = RookAttacks(from, occ) & b->colors[!b->side];

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, from, dest, CAPTURE, INVALID, b->side, ROOK);

            attacks &= attacks - 1;
        }

        rooks &= rooks - 1;
    }

    // Queens
    queens = b->pieces[QUEEN] & b->colors[b->side];

    while (queens) {
        from = lsb(queens);

        attacks = QueenAttacks(from, occ) & b->colors[!b->side];

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, from, dest, CAPTURE, INVALID, b->side, QUEEN);

            attacks &= attacks - 1;
        }

        queens &= queens - 1;
    }

    // Kings
    kings = b->pieces[KING] & b->colors[b->side];

    while (kings) {
        from = lsb(kings);

        attacks = KingAttacks(from) & b->colors[!b->side];

        while (attacks) {
            dest = lsb(attacks);

            AddMove(b, m, &movecount, from, dest, CAPTURE, INVALID, b->side, KING);

            attacks &= attacks - 1;
        }

        kings &= kings - 1;
    }

    return movecount;
}
