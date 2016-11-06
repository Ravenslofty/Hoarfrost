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

#include <string.h>

#include <fstream>
#include <functional>
#include <random>
#include <utility>
#include <vector>

#include "board.h"
#include "functions.h"

const double K = 2.1f;

typedef std::pair<struct Board, double> Test;

std::vector<Test> positions;

void LoadTests()
{
    std::fstream f("tune.epd", std::fstream::in);

    positions.clear();

    char fen[128], result[32];
    double Ri;
    struct Board b;

    while (f.good()) {
        f.getline(fen, 128);
        ParseFEN(&b, fen+9);

        f.getline(result, 32);
        sscanf(result, "result %lf", &Ri);

        Test t(b, Ri);

        positions.push_back(t);
    }
}

void LoadEval()
{
    std::fstream f("vals.txt", std::fstream::in);

    char line[128];

    while (f.good()) {
        f.getline(line, 128);

        if (!strncmp(line, "piece", 5)) {
            int index, value;
            sscanf(line, "piece %d %d", &index, &value);
            piecevals[index] = value;
            continue;
        }

        if (!strncmp(line, "pstrank", 7)) {
            int piece, phase, rank, value;
            sscanf(line, "pstrank %d %d %d %d", &piece, &phase, &rank, &value);
            pstrank[piece][phase][rank] = value;
            continue;
    }

        if (!strncmp(line, "pstfile", 7)) {
            int piece, phase, file, value;
            sscanf(line, "pstfile %d %d %d %d", &piece, &phase, &file, &value);
            pstfile[piece][phase][file] = value;
            continue;
        }
    }
}

double CalcError()
{
    double sum_e = 0.0, e, sigmoid;

    for (Test t: positions) {
        int qi = Quies(&t.first, -10000, +10000);

        if (t.first.side == BLACK)
            qi = -qi;

        sigmoid = 1 / (1 + pow(10.0, -(K * qi)/400));
        e = (t.second - sigmoid);
        if (e < 0.0)
            e = -e;
        sum_e += e;
    }

    e = sum_e / positions.size();

    printf("%.17g\n", e);

    return e;
}

void DumpEval()
{
    std::fstream f("vals.txt", std::fstream::out);

    for (int i = PAWN; i <= KING; i++) {
        f << "piece " << i << " " << piecevals[i] << std::endl;

        for (int j = 0; j <= 1; j++) {
            for (int k = 0; k <= 7; k++) {
                f << "pstrank " << i << " " << j << " " << k << " " << pstrank[i][j][k] << std::endl;
                f << "pstfile " << i << " " << j << " " << k << " " << pstfile[i][j][k] << std::endl;
            }
        }
    }
}

void OptimiseEval()
{
    std::mt19937 mt;
    std::uniform_int_distribution<int> uid(-10,+10);
    auto rng = std::bind(uid, mt);

    LoadEval();

    double firsterr = CalcError();
    double besterr = firsterr;

    for (int iteration = 0; iteration < 1000; iteration++) {
        LoadEval(); // Get current best

        std::uniform_int_distribution<int> choice(0,2);

        int c = choice(mt);

        if (c == 0) {
            std::uniform_int_distribution<int> victim(KNIGHT, QUEEN);

            int p = victim(mt);

            piecevals[p] += rng();
        } else if (c == 1) {
            std::uniform_int_distribution<int> piece(PAWN, KING);
            std::uniform_int_distribution<int> phase(0, 1);
            std::uniform_int_distribution<int> file(0, 7);

            int p = piece(mt);
            int h = phase(mt);
            int f = file(mt);

            int s = rng();

            printf("pstfile %d %d %d %+d\n", p, h, f, s);

            pstfile[p][h][f] += s;
        } else if (c == 2) {
            std::uniform_int_distribution<int> piece(PAWN, KING);
            std::uniform_int_distribution<int> phase(0, 1);
            std::uniform_int_distribution<int> rank(0, 7);

            int p = piece(mt);
            int h = phase(mt);
            int r = rank(mt);

            int s = rng();

            printf("pstrank %d %d %d %+d\n", p, h, r, s);

            pstrank[p][h][r] += s;
        }

        double newerr = CalcError();

        if (newerr < besterr) {
            besterr = newerr;

            DumpEval();
        }
    }

    printf("Reduced error by %.17g\n", firsterr - besterr);
}
