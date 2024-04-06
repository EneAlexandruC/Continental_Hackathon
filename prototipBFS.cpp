#include <vector>
#include <queue>
#include <iostream>
#define NMAX 10

typedef struct POINT
{
    int x, y;
} POINT;

// Vectori de directie in sensul acelor
int dx[] = {-1, 0, 1, 0},
    dy[] = {0, 1, 0, -1};

// Labirintul inconjurat de un border de 0-uri
int maze[][NMAX] = {
    {1, 1, 1, 0, 0, 0, 1, 0, 0, 0},
    {1, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    {1, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    {1, 0, 1, 0, 0, 0, 1, 1, 1, 1},
    {1, 0, 1, 0, 0, 0, 1, 0, 1, 0},
    {1, 1, 1, 0, 2, 1, 1, 0, 1, 0},
    {1, 0, 1, 0, 0, 0, 1, 0, 1, 0},
    {1, 0, 1, 0, 0, 0, 1, 0, 1, 0},
    {1, 0, 1, 0, 1, 1, 1, 0, 0, 0},
    {1, 0, 1, 1, 1, 0, 0, 0, 0, 0}};

int solvedMaze[NMAX][NMAX];

void solveMaze(POINT start)
{
    std::queue<POINT> q;
    std::vector<std::vector<int>> visited(NMAX, std::vector<int>(NMAX, 0));
    POINT coord;
    bool solutieGasita = false;

    q.push(start);
    visited[start.x][start.y] = 1;

    while (!q.empty() && solutieGasita == false)
    {
        coord = q.front();
        for (int i = 0; i < 4 && solutieGasita == false; i++)
        {
            if ((coord.x + dx[i] < 0 || coord.x + dx[i] >= NMAX) || (coord.y + dy[i] < 0 || coord.y + dy[i] >= NMAX)) // Verificare pentru out of borders
                continue;

            if (maze[coord.x + dx[i]][coord.y + dy[i]] >= 1 && visited[coord.x + dx[i]][coord.y + dy[i]] == 0) // Verificare daca celula este disponibila si vizitata
            {
                visited[coord.x + dx[i]][coord.y + dy[i]] = 1;                                   // Bag noua coordonata in coada si
                q.push({coord.x + dx[i], coord.y + dy[i]});                                      // dau update la solvedMaze
                solvedMaze[coord.x + dx[i]][coord.y + dy[i]] = solvedMaze[coord.x][coord.y] + 1; // cu pasul la care se afla robotul

                if (maze[coord.x + dx[i]][coord.y + dy[i]] > 1)
                {
                    solutieGasita = true;
                }
            }
        }
        q.pop();
    }
}

int main()
{
    POINT start = {0, 0};

    solveMaze(start);

    for (int i = 0; i < NMAX; i++, std::cout << std::endl)
        for (int j = 0; j < NMAX; j++)
            std::cout << solvedMaze[i][j] << "    ";
}