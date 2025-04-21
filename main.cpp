#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <stack>

using namespace sf;
using namespace std;

const int CELL_SIZE = 40;
const int ROWS = 7;
const int COLS = 7;

struct Cell
{
    bool top = true, right = true, bottom = true, left = true;
    bool visited = false;
};

vector<vector<Cell>> maze(ROWS, vector<Cell>(COLS));
vector<vector<Cell>> discoveredMaze(ROWS, vector<Cell>(COLS));

enum Direction
{
    NORTH = 0,
    EAST,
    SOUTH,
    WEST
};

int dx[] = {0, 1, 0, -1};
int dy[] = {-1, 0, 1, 0};

int simX = 1, simY = 1;
Direction simDir = SOUTH;
bool autoMode = false;

Direction leftOf(Direction d) { return static_cast<Direction>((d + 3) % 4); }
Direction rightOf(Direction d) { return static_cast<Direction>((d + 1) % 4); }
Direction behind(Direction d) { return static_cast<Direction>((d + 2) % 4); }

bool wallAt(int x, int y, Direction dir)
{
    if (x < 0 || y < 0 || x >= COLS || y >= ROWS)
        return true;
    if (dir == NORTH)
        return maze[y][x].top;
    if (dir == EAST)
        return maze[y][x].right;
    if (dir == SOUTH)
        return maze[y][x].bottom;
    if (dir == WEST)
        return maze[y][x].left;
    return true;
}

void getData(int x, int y, Direction dir)
{
    discoveredMaze[y][x].top = wallAt(x, y, NORTH);
    discoveredMaze[y][x].right = wallAt(x, y, EAST);
    discoveredMaze[y][x].bottom = wallAt(x, y, SOUTH);
    discoveredMaze[y][x].left = wallAt(x, y, WEST);
}

void simulateStep()
{
    if (simX == COLS - 2 && simY == ROWS - 2)
    {
        cout << "Maze Completed!" << endl;
        return;
    }

    getData(simX, simY, simDir);

    Direction left = leftOf(simDir);
    Direction right = rightOf(simDir);
    Direction back = behind(simDir);

    int lx = simX + dx[left];
    int ly = simY + dy[left];
    int fx = simX + dx[simDir];
    int fy = simY + dy[simDir];
    int rx = simX + dx[right];
    int ry = simY + dy[right];
    int bx = simX + dx[back];
    int by = simY + dy[back];

    if (!wallAt(simX, simY, left) && !discoveredMaze[ly][lx].visited)
    {
        simDir = left;
    }
    else if (!wallAt(simX, simY, simDir) && !discoveredMaze[fy][fx].visited)
    {
        // keep direction
    }
    else if (!wallAt(simX, simY, right) && !discoveredMaze[ry][rx].visited)
    {
        simDir = right;
    }
    else if (!wallAt(simX, simY, back))
    {
        simDir = back;
    }
    else
    {
        // stuck
        return;
    }

    simX += dx[simDir];
    simY += dy[simDir];
    discoveredMaze[simY][simX].visited = true;
}

void drawMaze(RenderWindow &window)
{
    for (int y = 0; y < ROWS; ++y)
    {
        for (int x = 0; x < COLS; ++x)
        {
            RectangleShape cell(Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);
            cell.setFillColor(discoveredMaze[y][x].visited ? Color(220, 220, 255) : Color::White);
            cell.setOutlineColor(Color::Black);
            cell.setOutlineThickness(1);
            window.draw(cell);

            if (maze[y][x].top)
            {
                RectangleShape wall(Vector2f(CELL_SIZE, 2));
                wall.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                wall.setFillColor(Color::Black);
                window.draw(wall);
            }
            if (maze[y][x].right)
            {
                RectangleShape wall(Vector2f(2, CELL_SIZE));
                wall.setPosition((x + 1) * CELL_SIZE - 2, y * CELL_SIZE);
                wall.setFillColor(Color::Black);
                window.draw(wall);
            }
            if (maze[y][x].bottom)
            {
                RectangleShape wall(Vector2f(CELL_SIZE, 2));
                wall.setPosition(x * CELL_SIZE, (y + 1) * CELL_SIZE - 2);
                wall.setFillColor(Color::Black);
                window.draw(wall);
            }
            if (maze[y][x].left)
            {
                RectangleShape wall(Vector2f(2, CELL_SIZE));
                wall.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                wall.setFillColor(Color::Black);
                window.draw(wall);
            }
        }
    }

    CircleShape start(CELL_SIZE / 4);
    start.setFillColor(Color::Green);
    start.setPosition(1 * CELL_SIZE + CELL_SIZE / 4, 1 * CELL_SIZE + CELL_SIZE / 4);
    window.draw(start);

    CircleShape goal(CELL_SIZE / 4);
    goal.setFillColor(Color::Blue);
    goal.setPosition((COLS - 2) * CELL_SIZE + CELL_SIZE / 4, (ROWS - 2) * CELL_SIZE + CELL_SIZE / 4);
    window.draw(goal);

    CircleShape robot(CELL_SIZE / 4);
    robot.setFillColor(Color::Red);
    robot.setPosition(simX * CELL_SIZE + CELL_SIZE / 4, simY * CELL_SIZE + CELL_SIZE / 4);
    window.draw(robot);
}

bool inBounds(int x, int y)
{
    return x >= 0 && y >= 0 && x < COLS && y < ROWS;
}

void generateMazeDFS(int x, int y, vector<vector<bool>> &visited)
{
    visited[y][x] = true;

    vector<Direction> dirs = {NORTH, EAST, SOUTH, WEST};
    random_shuffle(dirs.begin(), dirs.end());

    for (Direction d : dirs)
    {
        int nx = x + dx[d];
        int ny = y + dy[d];

        if (inBounds(nx, ny) && !visited[ny][nx])
        {
            // remove wall between (x,y) and (nx,ny)
            if (d == NORTH)
            {
                maze[y][x].top = false;
                maze[ny][nx].bottom = false;
            }
            else if (d == EAST)
            {
                maze[y][x].right = false;
                maze[ny][nx].left = false;
            }
            else if (d == SOUTH)
            {
                maze[y][x].bottom = false;
                maze[ny][nx].top = false;
            }
            else if (d == WEST)
            {
                maze[y][x].left = false;
                maze[ny][nx].right = false;
            }
            generateMazeDFS(nx, ny, visited);
        }
    }
}

void generateRandomMaze()
{
    maze = vector<vector<Cell>>(ROWS, vector<Cell>(COLS));
    vector<vector<bool>> visited(ROWS, vector<bool>(COLS, false));

    for (int i = 0; i < COLS; ++i)
    {
        maze[0][i].top = true;
        maze[ROWS - 1][i].bottom = true;
    }
    for (int i = 0; i < ROWS; ++i)
    {
        maze[i][0].left = true;
        maze[i][COLS - 1].right = true;
    }

    generateMazeDFS(1, 1, visited);
}

void resetSim()
{
    simX = 1;
    simY = 1;
    simDir = SOUTH;
    autoMode = false;
    discoveredMaze = vector<vector<Cell>>(ROWS, vector<Cell>(COLS));
    discoveredMaze[simY][simX].visited = true;
}

int main()
{
    srand(time(0));
    RenderWindow window(VideoMode(COLS * CELL_SIZE, ROWS * CELL_SIZE), "Micromouse Simulator");
    Clock clock;

    generateRandomMaze();
    resetSim();

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Space)
                    simulateStep();
                else if (event.key.code == Keyboard::A)
                    autoMode = !autoMode;
                else if (event.key.code == Keyboard::R)
                {
                    generateRandomMaze();
                    resetSim();
                }
            }
        }

        if (autoMode && clock.getElapsedTime().asMilliseconds() > 300)
        {
            simulateStep();
            clock.restart();
        }

        window.clear(Color::White);
        drawMaze(window);
        window.display();
    }

    return 0;
}
