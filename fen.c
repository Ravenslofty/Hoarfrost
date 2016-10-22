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
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>

#include "board.h"
#include "functions.h"

void ClearBoard(struct Board * b)
{
    b->pieces[PAWN] = 0;
    b->pieces[KNIGHT] = 0;
    b->pieces[BISHOP] = 0;
    b->pieces[ROOK] = 0;
    b->pieces[QUEEN] = 0;
    b->pieces[KING] = 0;

    b->colors[WHITE] = 0;
    b->colors[BLACK] = 0;

    b->side = FORCE;

    b->ep = INVALID;

    b->castle = 0;

    b->fifty = 0;
}

// Convert a Forsyth-Edwards Notation position into our internal representation.
void ParseFEN(struct Board * b, char * fen)
{
    // FEN is awkward to parse in my opinion. The format is readable for humans,
    // but it is unnatural for computers. Oh well.

    // First, the board state.
    // A capital letter signifies a white piece, a lowercase letter a black piece.
    // Pieces can be Pawns, kNights, Bishops, Rooks, Queens or Kings.
    // A rank is terminated by a '/' character.
    // Empty squares are abbreviated by listing the number of empty squares to
    //   either the next piece, or the end of the board.
    // The end of the board state is marked by a space.

    int rank = 7, file = 0, square;
    int fenidx = 0;
    char c;

    ClearBoard(b);

    while (rank >= 0 && file <= 7) {
        c = fen[fenidx];
        square = 8*rank + file;

        // Is it a series of empty squares?
        if (isdigit(c)) {
            // Yes, increment file by the number of empty squares.
            file += c - '0'; // atoi would also work.
            fenidx++;

            // Are we off the board?
            if (file >= 8) {
                rank--;
                file = 0;
            }

            continue;
        }

        // Is it an end of rank marker?
        if (c == '/') {
            // Yes, go to the next rank.
            file = 0;
            fenidx++;
            continue;
        }

        // Is it an end of board marker?
        if (c == ' ') {
            // Theoretically, we only get here if we made a mistake somewhere
            // So assert the values.
            assert(rank == 0);
            assert(file >= 8);

            fenidx++;
            break;
        }

        // Is it a piece?
        if (isalpha(c)) {
            // Yes, set the sidemask, piecemask and board as needed.

            // Due to BLACK being equal to 1, this update can be made branchlessly.
            b->colors[(islower(c)) ? BLACK : WHITE] |= 1ULL << square;

            // Sadly this can't be made elegant :(
            c = tolower(c);

            if (c == 'p') {
                b->pieces[PAWN] |= 1ULL << square;
            }
            if (c == 'n') {
                b->pieces[KNIGHT] |= 1ULL << square;
            }
            if (c == 'b') {
                b->pieces[BISHOP] |= 1ULL << square;
            }
            if (c == 'r') {
                b->pieces[ROOK] |= 1ULL << square;
            }
            if (c == 'q') {
                b->pieces[QUEEN] |= 1ULL << square;
            }
            if (c == 'k') {
                b->pieces[KING] |= 1ULL << square;
            }

            file++;

            if (file >= 8) {
                rank--;
                file = 0;
            }

            fenidx++;
            continue;
        }

        // We shouldn't get here, in an ideal world. But skip any strange garbage.
        fenidx++;
    }

    // This will now be a space separator. Skip it.
    fenidx++;

    // Now for the side to move indicator.
    // This will be either 'w' for White, or 'b' or Black. Simple enough.
    b->side = (fen[fenidx] == 'b');
    fenidx++;

    // Another space seperator. Skip it too.
    fenidx++;

    // Castling rights.
    // Capital letters are white's castle rights, lowercase letters are black's
    //   castle rights.
    // 'k' stands for Kingside, 'q' stands for Queenside.
    // '-' indicates no castling rights present.
    // The specification guarantees that the order will be KQkq, so we can just
    //   use that order without scanning.
    b->castle = 0;
    if (fen[fenidx] == 'K') {
        b->castle |= 1;
        fenidx++;
    }
    if (fen[fenidx] == 'Q') {
        b->castle |= 2;
        fenidx++;
    }
    if (fen[fenidx] == 'k') {
        b->castle |= 4;
        fenidx++;
    }
    if (fen[fenidx] == 'q') {
        b->castle |= 8;
        fenidx++;
    }
    // "But what about the case where there are no castling rights?"
    // In that situation, none of the if statements fire, leaving
    // b->castle at 0. In other words, indicating no castling rights.

    // Now for another space separator.
    fenidx++;

    // En passant target square.
    // Either '-' if there is no en passant target square, or the square itself
    //   in lowercase coordinate notation.
    if (fen[fenidx] == '-') {
        // No square, so just increment the index.
        fenidx++;
    } else {
        c = fen[fenidx];
        fenidx++;

        file = c - 'a';

        c = fen[fenidx];
        fenidx++;

        rank = c - '1';

        b->ep = 8*rank + file;
    }

    // Another space separator.
    fenidx++;

    // Fifty-move counter.
    // A number between 0 and (hopefully) 99.
    // This code breaks if we are given a fifty-move counter >= 100, but such
    // positions are illegal anyway.
    c = fen[fenidx];
    fenidx++;

    b->fifty = c - '0';

    if (fen[fenidx] != ' ') {
        b->fifty *= 10;
        c = fen[fenidx];
        b->fifty += c - '0';
    }

    // Next would be the fullmove counter, except we really don't care about it in the
    // least. So we just return.
    return;
}
