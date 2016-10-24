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
#include "functions.h"

int ReadClock()
{
    // returns wall-clock time in msec
#ifdef WINDOWS
    return GetTickCount();
#else
    struct timeval t;

    gettimeofday(&t, NULL);

    return t.tv_sec*1000 + t.tv_usec/1000;

#endif
}

int main()
{
    InitMagics();

    struct Board b;
    struct Undo u;
    struct Move m[128];
    char str[400];
    int i, n;
    int side = FORCE;

    ParseFEN(&b, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    setvbuf(stdout, NULL, _IONBF, 0);

    while (1) {

        if (b.side == side) {
            struct PV pv;
            int start, finish, depth;

            nodes = 0;
            cuts = 0;
            finish = 0;

            start = ReadClock();

            for (depth = 1; depth <= 8; depth++) {

                int score = Search(&b, depth, -10000, +10000, 1, &pv);

                finish = ReadClock();

                printf("%d %d %d %d ", depth, score, (finish-start)/10, nodes);

                for (i = 0; i < pv.count; i++) {
                    PrintMove(&b, pv.moves[i]);
                    printf(" ");
                }

                printf("\n");
            }

            printf("# First: %d Cuts: %d\n", first, cuts);

            if (pv.count) {
                printf("move ");
                PrintMove(&b, pv.moves[0]);
                printf("\n");
            } else {
                printf("resign\n");

                continue;
            }

            MakeMove(&b, &u, pv.moves[0]);
        }

        if (fgets(str, 400, stdin) == NULL) {
            return 0;
        }

        if (!strncmp(str, "protover 2", 8)) {
            printf("feature done=0 myname=\"Dorpsgek Corkscrew\" setboard=1 usermove=1 done=1\n");
            continue;
        }

        if (!strncmp(str, "go", 2)) {
            side = b.side;
            continue;
        }

        if (!strncmp(str, "force", 5)) {
            side = FORCE;
            continue;
        }

        if (!strncmp(str, "setboard", 8)) {
            ParseFEN(&b, str+9);
            continue;
        }

        if (!strncmp(str, "epd", 3)) {
            printf("\nPosition: %s\n", str+4);
            ParseFEN(&b, str+4);
            continue;
        }

        if (!strncmp(str, "new", 3)) {
            ParseFEN(&b, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            continue;
        }

        if (!strncmp(str, "kiwipete", 8)) {
            ParseFEN(&b, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
            continue;
        }

        if (!strncmp(str, "pos4", 4)) {
            ParseFEN(&b, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
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

        if (!strncmp(str, "perft", 5)) {
            int depth;
            uint64_t correct, result;

            sscanf(str, "perft %d %llu", &depth, &correct);

            result = Perft(&b, depth);

            if (result == correct) {
                printf("Depth %d OK\n", depth);
            } else {
                printf("Depth %d FAILED: expected %llu, got %llu\n", depth, correct, result);
            }

            continue;
        }

        if (!strncmp(str, "usermove", 8)) {
            struct Move tmp, m;
            tmp.from = str[9] - 'a';
            tmp.from += 8*(str[10] - '1');
            tmp.dest = str[11] - 'a';
            tmp.dest += 8*(str[12] - '1');

            struct Sort s;
            int found = 0;

            InitSort(&b, &s);

            while (NextMove(&s, &m)) {

                printf("# ");
                PrintMove(&b, m);
                printf("\n");

                if ((m.from&63) == tmp.from && (m.dest&63) == tmp.dest) {
                    MakeMove(&b, &u, m);
                    found = 1;
                    break;
                }
            }

            if (!found)
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
            printf("en passant sq:  %d\n", b.ep);
            printf("castle rights:  %d\n", b.castle);
            continue;
        }

        if (!strncmp(str, "undo", 4)) {
            UnmakeMove(&b, &u, m[i]);
            continue;
        }

        if (!strncmp(str, "quit", 4)) {
            break;
        }
    }

    return 0;
}
