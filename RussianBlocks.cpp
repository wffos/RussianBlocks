#include <windows.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <map>
#include <algorithm>
#include <conio.h>

using namespace std;

// 游戏区域的宽度和高度
const int WIDTH = 10;
const int HEIGHT = 20;
const int MAXLINE = 3; // 消除多少行后开始博弈

// 定义方块的形状结构体
struct WffBlock
{
    int shape[4][4];
    int x, y;
};

// 各种方块形状的定义
WffBlock g_blockTypes[7] =
    {
        {{{0, 1, 0, 0},
          {0, 1, 0, 0},
          {0, 1, 0, 0},
          {0, 1, 0, 0}},
         0,
         0}, // I型
        {{{0, 0, 0, 0},
          {0, 1, 1, 0},
          {0, 1, 1, 0},
          {0, 0, 0, 0}},
         0,
         0}, // O型
        {{{0, 0, 0, 0},
          {0, 1, 1, 0},
          {0, 1, 0, 0},
          {0, 1, 0, 0}},
         0,
         0}, // L型
        {{{0, 0, 0, 0},
          {0, 1, 1, 0},
          {0, 0, 1, 0},
          {0, 0, 1, 0}},
         0,
         0}, // J型
        {{{0, 0, 0, 0},
          {0, 1, 0, 0},
          {0, 1, 1, 0},
          {0, 0, 1, 0}},
         0,
         0}, // S型
        {{{0, 0, 0, 0},
          {0, 0, 1, 0},
          {0, 1, 1, 0},
          {0, 1, 0, 0}},
         0,
         0}, // Z型
        {{{0, 0, 0, 0},
          {0, 1, 0, 0},
          {0, 1, 0, 0},
          {0, 1, 1, 0}},
         0,
         0} // T型
};

// 方块权重
map<int, int> g_blockWeights =
    {
        {0, 0},
        {1, 0},
        {2, 0},
        {3, 0},
        {4, 0},
        {5, 0},
        {6, 0}};
// 游戏背景
int g_gameBackimg[HEIGHT][WIDTH] = {0};
// 要出现的方块
vector<int> g_nextBlocks;
bool g_startBadMatch = false;
// 生成随机的方块（这里可以根据博弈算法来调整生成策略）
WffBlock CreateBlock(int clearedLines)
{
    // 消除行数 权重增加（权重开不开博弈都得算，为了后面开博弈直接出效果）
    g_blockWeights[g_nextBlocks[0]] += clearedLines;

    if (!g_startBadMatch)
    {
        // 正常无博弈
        srand(static_cast<unsigned int>(time(nullptr)));
        int randomIndex = rand() % (sizeof(g_blockTypes) / sizeof(g_blockTypes[0]));
        // 更新下一个方块序列，选择最不利的下下个方块
        g_nextBlocks.erase(g_nextBlocks.begin());
        g_nextBlocks.push_back(randomIndex);
        WffBlock currentBlock = g_blockTypes[g_nextBlocks[0]];
        currentBlock.x = WIDTH / 2 - 2;
        currentBlock.y = 0;
        return currentBlock;
    }
    else
    {
        int maxWeight = -1;
        // 找出不利方块
        vector<std::pair<int, int>> sortWeights(g_blockWeights.begin(), g_blockWeights.end());
        sort(sortWeights.begin(), sortWeights.end(),
             [](const std::pair<int, int> &a, const std::pair<int, int> &b)
             {
                 return a.second < b.second;
             });
        vector<int> badCandidates;
        vector<int> CandidatesWeights; // 前二（支持并列，不然会重复刷新）
        for (size_t i = 0; i < sortWeights.size(); i++)
        {
            if (CandidatesWeights.end() == std::find(CandidatesWeights.begin(), CandidatesWeights.end(), sortWeights[i].second))
            {
                CandidatesWeights.push_back(sortWeights[i].second);
            }
            if (CandidatesWeights.size() >= 2)
            {
                break;
            }
        }
        for (size_t i = 0; i < sortWeights.size(); i++)
        {
            if (CandidatesWeights.end() != std::find(CandidatesWeights.begin(), CandidatesWeights.end(), sortWeights[i].second))
            {
                //todo 可以适当限制一下候选方块数量，这样博弈效果更明显
                badCandidates.push_back(sortWeights[i].first);
            }
        }

        // 从不利方块中随机选一个（消除重复因素，打消玩家疑惑）
        int randomIndex = -1;
        if (!badCandidates.empty())
        {
            srand(static_cast<unsigned int>(time(nullptr)));
            randomIndex = rand() % badCandidates.size();
        }
        if (randomIndex == -1)
        {
            // 如果没选到合适的 随机选一个
            srand(static_cast<unsigned int>(time(nullptr)));
            randomIndex = rand() % (sizeof(g_blockTypes) / sizeof(g_blockTypes[0]));
        }

        // 更新下一个方块序列，选择最不利的下下个方块
        g_nextBlocks.erase(g_nextBlocks.begin());
        g_nextBlocks.push_back(randomIndex);
        WffBlock currentBlock = g_blockTypes[g_nextBlocks[0]];
        currentBlock.x = WIDTH / 2 - 2;
        currentBlock.y = 0;
        return currentBlock;
    }
}

// 绘制游戏区域
void DrawRect(WffBlock currentBlock)
{
    system("cls");
    cout << "author : wangfengfan" << endl;
    cout << "how to play : w - spin   a - move left   s - move down    d - move right" << endl;
    cout << "--------------------" << endl;
    for (int y = 0; y < HEIGHT; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
            if (currentBlock.x <= x && x < currentBlock.x + 4 && currentBlock.y <= y && y < currentBlock.y + 4)
            {
                if (currentBlock.shape[y - currentBlock.y][x - currentBlock.x] == 0)
                {
                    if (g_gameBackimg[y][x] == 0) // 边缘可能遮挡
                        cout << "  ";
                    else
                        cout << "[]";
                }
                else
                    cout << "[]";
            }
            else
            {
                if (g_gameBackimg[y][x] == 0)
                    cout << "  ";
                else
                    cout << "[]";
            }
        }
        cout << endl;
    }
    cout << "--------------------" << endl;
    WffBlock nextBlock = g_blockTypes[g_nextBlocks[1]];
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {

            if (nextBlock.shape[y][x] == 0)
                cout << "  ";
            else
                cout << "[]";
        }
        cout << endl;
    }
    cout << "--------------------" << endl;

}

// 固定方块位置
void LockBlock(WffBlock currentBlock)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (currentBlock.shape[i][j])
                g_gameBackimg[currentBlock.y + i][currentBlock.x + j] = 1;
        }
    }
}

// 检查方块是否可以移动到指定位置（x, y）
bool CanMove(WffBlock block, int newX, int newY)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (block.shape[i][j])
            {
                int targetX = block.x + j + newX;
                int targetY = block.y + i + newY;
                if (targetX < 0 || targetX >= WIDTH || targetY >= HEIGHT || (targetY >= 0 && g_gameBackimg[targetY][targetX]))
                    return false;
            }
        }
    }
    return true;
}

// 移动方块
void MoveBlock(WffBlock &block, int dx, int dy)
{
    if (CanMove(block, dx, dy))
    {
        block.x += dx;
        block.y += dy;
        DrawRect(block);
    }
}

// 旋转方块
void SpinBlock(WffBlock &block)
{
    WffBlock rotated = block;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            rotated.shape[i][j] = block.shape[3 - j][i];
        }
    }
    if (CanMove(rotated, 0, 0))
    {
        block = rotated;
        DrawRect(block);
    }
}

// 检查能否消除
int CheckLines()
{
    int linesCleared = 0;
    for (int i = HEIGHT - 1; i >= 0; --i)
    {
        bool lineFull = true;
        for (int j = 0; j < WIDTH; ++j)
        {
            if (g_gameBackimg[i][j] == 0)
            {
                lineFull = false;
                break;
            }
        }
        if (lineFull)
        {
            linesCleared++;
            for (int k = i; k > 0; --k)
            {
                for (int l = 0; l < WIDTH; ++l)
                {
                    g_gameBackimg[k][l] = g_gameBackimg[k - 1][l];
                }
            }
            i++;
        }
    }
    return linesCleared;
}

// 游戏主循环
void gameLoop()
{
    int totalLine = 0;

    // 当前、下一个、下下一个
    for (int i = 0; i < 3; ++i)
    {
        srand(static_cast<unsigned int>(time(nullptr)) + i);
        int randomBlockID = rand() % 7;
        g_nextBlocks.push_back(randomBlockID);
    }

    WffBlock currentBlock = CreateBlock(0); // 生成随机方块

    while (true)
    {
        for (int i = 0; i < 10; ++i)
        {
            // 获取键盘输入控制
            if (_kbhit())
            {
                char key = _getch();
                switch (key)
                {
                case 'a':
                case 'A':
                    MoveBlock(currentBlock, -1, 0);
                    break;
                case 'd':
                case 'D':
                    MoveBlock(currentBlock, 1, 0);
                    break;
                case 's':
                case 'S':
                    MoveBlock(currentBlock, 0, 1);
                    break;
                case 'w':
                case 'W':
                    SpinBlock(currentBlock);
                    break;
                }
            }
        }

        if (CanMove(currentBlock, 0, 1))
        {
            MoveBlock(currentBlock, 0, 1);
        }
        else
        {
            // 清空输入缓冲区
            while (_kbhit())
            {
                _getch();
            }
            // 固定当前方块，检查能否消除，生成新的方块
            LockBlock(currentBlock);
            int clearedLines = CheckLines();
            totalLine += clearedLines;
            if (totalLine > MAXLINE)
            {
                g_startBadMatch = true;
            }
            currentBlock = CreateBlock(clearedLines);
            if (!CanMove(currentBlock, 0, 0))
            {
                break;
            }
        }
        Sleep(500);
    }

    cout << "game over!" << endl;
}

int main()
{
    gameLoop();
    return 0;
}
