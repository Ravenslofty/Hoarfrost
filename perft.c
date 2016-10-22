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

uint64_t Perft(struct Board * b, int depth)
{
    struct Sort s;
    struct Move m;
    struct Undo u;
    struct Board c;
    int movecount, i, j;
    uint64_t nodes = 0, tmp;

    if (depth == 0) {
        return 1;
    }

    InitSort(b, &s);

    while (NextMove(&s, &m)) {

        MakeMove(b, &u, m);

        if (IsIllegal(b)) {
            UnmakeMove(b, &u, m);
            continue;
        }

        nodes += tmp = Perft(b, depth - 1);

        UnmakeMove(b, &u, m);
    }

    return nodes;
}

uint64_t Divide(struct Board * b, int depth)
{
    struct Sort s;
    struct Move m;
    struct Undo u;
    struct Board c;
    int movecount, i, j;
    uint64_t nodes = 0, tmp;

    if (depth == 0) {
        return 1;
    }

    InitSort(b, &s);

    while (NextMove(&s, &m)) {

        MakeMove(b, &u, m);

        if (IsIllegal(b)) {
            UnmakeMove(b, &u, m);
            continue;
        }

        PrintMove(b, m);

        nodes += tmp = Perft(b, depth - 1);

        UnmakeMove(b, &u, m);

        printf(" %llu\n", tmp);
    }

    return nodes;
}
