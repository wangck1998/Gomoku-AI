#include "aialphabeta.h"
#include <QVector>
#include <QPoint>
#include <QDebug>
#include "constants.h"

#define n BOARD_SIZE
#define INF 1000000
#define timelimit 22345
#define check(x, y) ((x) >= 0 && (x) < n && (y) >= 0 && (y) < n)

struct Choice
{
    Choice (int _score = 0, int _x = 0, int _y = 0)
        : score(_score), x(_x), y(_y)
    {
    }

    int score;
    int x, y;
    bool operator < (const Choice &o) const
    {
        return score > o.score;
    }
};

AIAlphaBeta::AIAlphaBeta(QObject *parent) : AIThread(parent)
{

}

void AIAlphaBeta::run()
{
    timer = 0;
    rx = n / 2, ry = n / 2;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
        {
            if (chessboard[i][j] == BLACK) board[i][j] = 0;
            else if (chessboard[i][j] == WHITE) board[i][j] = 1;
            else board[i][j] = -1;
        }

    for (max_deep = 1; timer < timelimit; max_deep++)
    {
        int tmp = dfs(0, -INF - 233, INF + 233, max_deep);
        qDebug() << "depth:" << max_deep << "expectance:" << tmp;
    }
    qDebug() << timer << "phases have been visited.";
    response(rx, ry);
}

bool AIAlphaBeta::exist(const int &x, const int &y)
{
    for (int dx = -2; dx <= 2; dx++)
        for (int dy = -2; dy <= 2; dy++)
        {
            int p = x + dx, q = y + dy;
            if (check(p, q) && chessboard[p][q] != -1) return true;
        }
    return false;
}

int AIAlphaBeta::dfs(int p, int alpha, int beta, int deep)
{
    if (gameOver()) return -INF;
    if (deep < 0) return evaluate() * (p ? -1 : 1);
    timer++;
    QVector<Choice> que;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
        {
            if (board[i][j] != -1) continue;
            if (! exist(i, j)) continue;
            que.push_back(Choice(
                              //potential(i, j, p) + potential(i, j, p ^ 1),
                              //qMax(potential(i, j, p), potential(i, j, p ^ 1)),
                              qMax(potential(i, j, p), potential(i, j, p ^ 1)) +
                              qMax(potential2(i, j, p), potential2(i, j, p ^ 1)),
                                 i, j));
        }
    qSort(que);
    while (que.size() > 10) // || que.end()->score + 50 < que.begin()->score)
        que.pop_back();

    bool firstLayer = deep == max_deep;

    Choice c;
    foreach (c, que) {
        board[c.x][c.y] = p;
        int tmp = -dfs(p ^ 1, -beta, -alpha, deep - 1);
        board[c.x][c.y] = -1;
        if (tmp >= beta)
            return beta;
        if (tmp > alpha)
        {
            alpha = tmp;
            if (firstLayer) rx = c.x, ry = c.y;
        }
    }
    return alpha;
}

int AIAlphaBeta::potential(const int &x, const int &y, const int &p)
{
    int ret = 0;
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++)
        {
            if (! dx && ! dy) continue;
            int tmp = 0;
            int c = 0;
            for (int s = 1, px = x, py = y; s < 5; s++, px += dx, py += dy)
            {
                if (! check(px, py)) break;
                if (board[px][py] == (p ^ 1))
                {
                    tmp = 0;
                    break;
                }
                if (board[px][py] == -1) tmp += 1;
                else  // board[px][py] == p
                {
                    c++;
                    tmp += (6 - s) * (6 - s);
                }
            }
            ret += tmp;
        }
    return ret;
}

const int d4[][2] = {
    {1, -1},
    {1, 0},
    {1, 1},
    {0, 1},
};

int AIAlphaBeta::potential2(const int &x, const int &y, const int &p)
{
    int ret = 0;
    for (int d = 0; d < 4; d++)
    {
        int c = 0, cs = 1;
        for (int s = 1, px = x, py = y, dx = d4[d][0], dy = d4[d][1];
             s < 5; s++, px += dx, py += dy)
        {
            if (! check(px, py)) break;
            if (board[px][py] == (p ^ 1)) break;
            cs++;
            if (board[px][py] == p) c++;
        }
        for (int s = 1, px = x, py = y, dx = -d4[d][0], dy = -d4[d][1];
             s < 5; s++, px += dx, py += dy)
        {
            if (! check(px, py)) break;
            if (board[px][py] == (p ^ 1)) break;
            cs++;
            if (board[px][py] == p) c++;
        }
        if (cs >= 5) ret += c * c * c * 5;
    }
    return ret;
}

int AIAlphaBeta::evaluate()
{
    int ret = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
        {
            int t = board[i][j];
            if (t == -1) continue;
            ret += potential(i, j, t) * (board[i][j] == 0 ? 1 : -1);
        }
    return ret;
}

bool AIAlphaBeta::gameOver()
{
    for (int i = 0; i < n; i++)
    {
        if (linkCheck(i, 0, 0, 1)) return true;
        if (linkCheck(0, i, 1, 0)) return true;
        if (linkCheck(i, 0, 1, 1)) return true;
        if (linkCheck(i, 0, -1, 1)) return true;
        if (linkCheck(0, i, 1, 1)) return true;
        if (linkCheck(n - 1, i, -1, 1)) return true;
    }
    return false;
}

bool AIAlphaBeta::linkCheck(int x, int y, int dx, int dy)
{
    int c = -1, s = 0;
    for (; check(x, y); x += dx, y += dy)
    {
        int t = board[x][y];
        if (t == c)
        {
            s++;
            if (s >= 5 && t != -1)
            {
                return true;
            }
        }
        else
        {
            c = t;
            s = 1;
        }
    }
    return false;
}


#undef n
#undef INF
#undef timelimit
#undef check