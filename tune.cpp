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
            int index, phase, value;
            sscanf(line, "piece %d %d %d", &index, &phase, &value);
            piecevals[index][phase] = value;
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

void DumpEval(const char * const filename)
{
    std::fstream f(filename, std::fstream::out);

    for (int i = PAWN; i <= KING; i++) {
        for (int j = 0; j <= 1; j++) {
            f << "piece " << i << " " << j << " " << piecevals[i][j] << std::endl;

            for (int k = 0; k <= 7; k++) {
                f << "pstrank " << i << " " << j << " " << k << " " << pstrank[i][j][k] << std::endl;
                f << "pstfile " << i << " " << j << " " << k << " " << pstfile[i][j][k] << std::endl;
            }
        }
    }
}

void DumpEval()
{
    DumpEval("vals.txt");
}

void DumpZeroEval()
{
    DumpEval("vals-backup.txt");

    std::fstream f("vals.txt", std::fstream::out);

    for (int i = PAWN; i <= KING; i++) {
        for (int j = 0; j <= 1; j++) {
            f << "piece " << i << " " << j << " " << 0 << std::endl;

            for (int k = 0; k <= 7; k++) {
                f << "pstrank " << i << " " << j << " " << k << " " << 0 << std::endl;
                f << "pstfile " << i << " " << j << " " << k << " " << 0 << std::endl;
            }
        }
    }
}

void DumpRandomEval()
{
    DumpEval("vals-backup.txt");

    std::fstream f("vals.txt", std::fstream::out);
    std::mt19937 mt;
    std::uniform_int_distribution<int> uid(-1000,+1000);
    auto rng = std::bind(uid, mt);


    for (int i = PAWN; i <= KING; i++) {
        for (int j = 0; j <= 1; j++) {
            f << "piece " << i << " " << j << " " << 0 << std::endl;

            for (int k = 0; k <= 7; k++) {
                f << "pstrank " << i << " " << j << " " << k << " " << 0 << std::endl;
                f << "pstfile " << i << " " << j << " " << k << " " << 0 << std::endl;
            }
        }
    }
}

void OptimiseEval()
{
    LoadEval();

    double firsterr = CalcError();
    double besterr = firsterr;
    double currerr;

    int s = 100;

    while (s >= 1) {
        while (true) {
            LoadEval(); // Get current best

            int c, p, h, f, bestc = 0, bestp = 0, besth = 0, bests = 0, bestf = 0;
            bool progress = false;

            // Find best neighbour.

            // Piece values, 2 * 5 * 2 = 20 rounds.
            for (h = 0; h <= 1; h++) {

                if (h == 0)
                    printf("== Midgame ==\n");
                else
                    printf("== Endgame ==\n");

                for (p = PAWN; p < KING; p++) {

                    printf("== Piece Value ==\n");

                    piecevals[p][h] += s;

                    currerr = CalcError();

                    if (currerr < besterr) {
                        bestc = 0;
                        bestp = p;
                        besth = h;
                        bests = s;
                        besterr = currerr;
                        progress = true;

                        printf("%.17g (%.17g)\n", besterr, firsterr-besterr);
                    }

                    piecevals[p][h] -= s + s;

                    currerr = CalcError();

                    if (currerr < besterr) {
                        bestc = 0;
                        bestp = p;
                        besth = h;
                        bests = s;
                        besterr = currerr;
                        progress = true;

                        printf("%.17g (%.17g)\n", besterr, firsterr-besterr);
                    }

                    piecevals[p][h] += s;

                    printf("== PSTs ==\n");

                    for (f = 0; f < 8; f++) {
                        pstfile[p][h][f] += s;

                        currerr = CalcError();

                        if (currerr < besterr) {
                            bestc = 1;
                            bestp = p;
                            besth = h;
                            bests = s;
                            bestf = f;
                            besterr = currerr;
                            progress = true;

                            printf("%.17g (%.17g)\n", besterr, firsterr-besterr);
                        }

                        pstfile[p][h][f] -= s + s;

                        currerr = CalcError();

                        if (currerr < besterr) {
                            bestc = 1;
                            bestp = p;
                            besth = h;
                            bests = -s;
                            bestf = f;
                            besterr = currerr;
                            progress = true;

                            printf("%.17g (%.17g)\n", besterr, firsterr-besterr);
                        }

                        pstfile[p][h][f] += s;

                        pstrank[p][h][f] += s;

                        currerr = CalcError();

                        if (currerr < besterr) {
                            bestc = 2;
                            bestp = p;
                            besth = h;
                            bests = s;
                            bestf = f;
                            besterr = currerr;
                            progress = true;

                            printf("%.17g (%.17g)\n", besterr, firsterr-besterr);
                        }

                        pstrank[p][h][f] -= s + s;

                        currerr = CalcError();

                        if (currerr < besterr) {
                            bestc = 2;
                            bestp = p;
                            besth = h;
                            bests = -s;
                            bestf = f;
                            besterr = currerr;
                            progress = true;

                            printf("%.17g (%.17g)\n", besterr, firsterr-besterr);
                        }

                        pstrank[p][h][f] += s;
                    }
                }
            }

            if (progress) {
                if (bestc == 0) {
                    printf("piece %d %d %+d\n", bestp, besth, bests);

                    piecevals[bestp][besth] += bests;
                }

                if (bestc == 1) {
                    printf("pstfile %d %d %d %+d\n", bestp, besth, bestf, bests);

                    pstfile[bestp][besth][bestf] += bests;
                }

                if (bestc == 2) {
                    printf("pstrank %d %d %d %+d\n", bestp, besth, bestf, bests);

                    pstrank[bestp][besth][bestf] += bests;
                }

                DumpEval();
            } else {
                break;
            }
        }

        char filename[16];

        printf("=== END OF ITERATION FOR S = %d ===\n", s);

        sprintf(filename, "vals-%d.txt", s);

        DumpEval(filename);

        s = s / 2;
    }

    printf("Reduced error by %.17g\n", firsterr - besterr);
}
