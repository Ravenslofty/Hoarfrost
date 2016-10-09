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

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "board.h"
#include "m42.h"

int main()
{
    m42_init();

    struct Board b;
    struct Undo u;
    struct Move m[128];
    char str[400];
    int i, n;

    ParseFEN(&b, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    while (1) {
        printf("> ");
        if (fgets(str, 400, stdin) == NULL) {
            return 0;
        }

        if (!strncmp(str, "setboard", 8)) {
            ParseFEN(&b, str+9);
            continue;
        }

        if (!strncmp(str, "divide", 6)) {
            int depth;
            clock_t start, stop;
            uint64_t nodes;
            float time;

            sscanf(str, "divide %d", &depth);

            start = clock();
            nodes = Divide(&b, depth);
            stop = clock();

            time = (float)(((float)(stop) - (float)(start)) / CLOCKS_PER_SEC);

            printf("Nodes: %llu in %.3f seconds\n", nodes, time);
            continue;
        }

        if (!strncmp(str, "usermove", 8)) {
            struct Move tmp;
            tmp.from = str[9] - 'a';
            tmp.from += 8*(str[10] - '1');
            tmp.dest = str[11] - 'a';
            tmp.dest += 8*(str[12] - '1');

            n = GenerateQuiets(&b, m);
            for (i = 0; i < n; i++) {
                if (m[i].from == tmp.from && m[i].dest == tmp.dest) {
                    MakeMove(&b, &u, m[i]);
                    break;
                }
            }

            n = GenerateCaptures(&b, m);
            for (i = 0; i < n; i++) {
                if (m[i].from == tmp.from && m[i].dest == tmp.dest) {
                    MakeMove(&b, &u, m[i]);
                    break;
                }
            }

            if (i == n)
                printf("Illegal move\n");

			continue;
        }

        if (!strncmp(str, "debug", 5)) {
            printf("pieces[PAWN]:   %016llX\n", b.pieces[PAWN]);
            printf("pieces[KNIGHT]: %016llX\n", b.pieces[KNIGHT]);
            printf("pieces[BISHOP]: %016llX\n", b.pieces[BISHOP]);
            printf("pieces[ROOK]:   %016llX\n", b.pieces[ROOK]);
            printf("pieces[QUEEN]:  %016llX\n", b.pieces[QUEEN]);
            printf("pieces[KING]:   %016llX\n", b.pieces[KING]);
            printf("\n");
            printf("colors[WHITE]:  %016llX\n", b.colors[WHITE]);
            printf("colors[BLACK]:  %016llX\n", b.colors[BLACK]);
            printf("\n");
            printf("side to move:   %d\n", b.side);
            continue;
        }

        if (!strncmp(str, "undo", 4)) {
            UnmakeMove(&b, &u, m[i]);
            continue;
        }
    }

    return 0;
}
