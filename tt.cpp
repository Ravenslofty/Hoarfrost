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

#include <stdint.h>
#include <string.h>
#include <vector>

#include "board.h"
#include "functions.h"

struct TTE {
    uint64_t hash;
    struct Move m;
    int16_t val;
    uint8_t flags;
    uint8_t depth;
};

std::vector< TTE > tt;

void ResizeTT(int megabytes)
{
    // Convert to bytes
    size_t s = megabytes * 1024 * 1024;
    s /= sizeof(TTE);

    tt.resize(s);
}

void ClearTT()
{
    size_t s = tt.size();

    tt.clear();
    tt.resize(s);
}

int ReadTT(struct Board * b, struct Move * m, int depth, int alpha, int beta, int ply)
{
    struct TTE entry = tt[b->hash & (tt.size()-1)];

    int val = entry.val;

    if (val >= 9500) {
        val = val - ply;
    }
    if (val <= -9500) {
        val = val + ply;
    }

    m->from = 0;
    m->dest = 0;
    m->type = 0;
    m->score = 0;

    if (entry.hash == b->hash) {

        if (entry.depth >= depth) {
            if (entry.flags == hashfEXACT) {
                return val;
            }
            if ((entry.flags == hashfALPHA) &&
                    (val <= alpha)) {
                return val;
            }
            if ((entry.flags == hashfBETA) &&
                    (val >= beta)) {
                return val;
            }
        }
        *m = entry.m;
        m->score = 0;
    }

    return 11000;
}

void WriteTT(struct Board * b, int depth, int val, int hashf, struct Move m, int ply)
{
    struct TTE entry;

    if (val >= 9500) {
        val = val + ply;
    }
    if (val <= -9500) {
        val = val - ply;
    }

    entry.hash = b->hash;
    entry.m = m;
    entry.val = val;
    entry.flags = hashf;
    entry.depth = depth;

    tt[b->hash & (tt.size()-1)] = entry;
}
