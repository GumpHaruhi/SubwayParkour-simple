#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <math.h>

// 玩家对象
typedef struct MyHero{
    int state;                  // 玩家状态： 1 stand  0 crouch  2 jump
    int position;               // 玩家赛道： 1 2 3
    SDL_Rect* rect;             // 玩家 body
    int health;                 // 玩家生命值
    int v;                      // 跳跃滞空速度
    bool binerjump;             // 二段跳标记
    int buff;                   // 增益： 0-none  1-金身  2-无敌  3-富翁  4-发财
    Uint32 crouchTimer;         // 下蹲延迟计时器
    Uint32 cureTimer;           // 治愈计时器
    Uint32 buffTimer;           // buff 计时器
} Player;

// 物体对象
typedef struct MyObject{
    int type;                   // 物体类别：0-coin  1-low  2-up  3-mid  4-full 5-buff 6-run
    SDL_Rect* rect;             // 物体 body
    struct MyObject* next;      // 链表结构
} Object;

// 全局变量
SDL_Window* window;                // 主窗口
SDL_Surface* screenSurface;        // 屏幕
SDL_Renderer* renderer;            // 画笔
TTF_Font* font_0 = NULL; TTF_Font* font_1 = NULL;
TTF_Font* font_2 = NULL; TTF_Font* font_3 = NULL;
TTF_Font* font_4 = NULL; TTF_Font* font_5 = NULL;
const int playerHigh = 60, playerWidth = 60; // 玩家的size
int speed;                         // 速度
int score;                         // 得分
const int grity = 4;               // 重力
const int velocity = 32;           // 跳跃初速度
const int crouchDelay = 600;       // 滑铲延迟
const int cureDelay = 5000;        // 治愈延迟
int weight_1, weight_2, weight_3, weight_4, weight_5, weight_6;  // 随机数权重
Uint32 generateTimer;              // 刷物品的计时器
int generateDelay = 1000;          // 刷物品延迟
const int buffDelay = 10000;        // buff延迟

// 函数声明
void initSDL();
SDL_Texture* loadTexture(const char* filename);
void renderTexture(SDL_Texture* texture, SDL_Rect* rect);
void renderText(const char* text, int x, int y, char flag);
void drawLine();
void drawPlayer(Player* hero);
void cleanUp();
void keyPressEvent(SDL_Event event, Player* hero, bool* is_Pause);
Player* initPlayer();
void updatePlayer(Player* hero);
void updateState(Player* hero, int index);
Object* randomObject(int index, bool have_buff);
Object* generateObject(Object* objects, bool have_buff);
void drawObject(Object* objects, int index);
bool updateObject(Object* objects, Player* hero);
bool checkCollision(SDL_Rect* rect1, SDL_Rect* rect2);
void drawTopBar(Player* hero);
void runGame(bool* is_Pause);
void updateSpeed();

// 主函数
int main(int argc,char *argv[]) {
    initSDL();
    srand((unsigned int)time(NULL));   // 随机数种子

    bool quit = false;          // 程序退出
    bool gameRuning = false;    // 游戏正在运行
    bool tag = false;           // 标记是否已经进行过游戏
    bool is_Pause = false;     // 标记游戏暂停
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    if (!gameRuning) {
                        // 空格键按下，开始游戏
                        gameRuning = true;
                    }
                }
            }
        }

        // 清空渲染器
        if(tag){
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }
        else {
            SDL_SetRenderDrawColor(renderer, 64, 32, 64, 255);
        }
        SDL_RenderClear(renderer);

        if (gameRuning) {
            runGame(&is_Pause);
            gameRuning = false;
            tag = true;
        }
        else if(tag){
            // 游戏已输，等待重开
            char numberText[20];  // 足够大以容纳整数的字符串表示
            sprintf(numberText, "YOU LOSE");
            renderText(numberText, 260, 300, 'r');
            char scoreText[20];
            sprintf(scoreText, "Score: %d", score);
            renderText(scoreText, 550, 500, 'b');
        }
        else{
            char title0Text[25];
            sprintf(title0Text, "Sub");
            renderText(title0Text, 100, 20, 'x');
            char title2Text[25];
            sprintf(title2Text, "Way");
            renderText(title2Text, 500, 280, 'x');
            char title1Text[25];
            sprintf(title1Text, "PARKOUR");
            renderText(title1Text, 280, 230, 'y');
            char numberText[25];  // 足够大以容纳整数的字符串表示
            sprintf(numberText, "TAP SPASE TO START");
            renderText(numberText, 350, 600, 's');
        }

        // 刷新屏幕
        SDL_UpdateWindowSurface(window);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }

    cleanUp();
    return 0;
}


/***********         初始化         ************/
// 初始化SDL
void initSDL(){
    SDL_Init(SDL_INIT_VIDEO);

    IMG_Init(IMG_INIT_PNG);
    if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG){
        printf("fail to init IMG: %s\n", IMG_GetError());
    }

    // 加载字体
    TTF_Init();
    // 获取可执行文件的基本路径
    char basePath[256];
    SDL_strlcpy(basePath, SDL_GetBasePath(), sizeof(basePath));
    // 构建相对路径
    char relativePath0[256];
    SDL_snprintf(relativePath0, sizeof(relativePath0), "%s%s", basePath, "font_0.ttf");
    font_0 = TTF_OpenFont(relativePath0, 28);

    char relativePath2[256];
    SDL_snprintf(relativePath2, sizeof(relativePath2), "%s%s", basePath, "font_0.ttf");
    font_2 = TTF_OpenFont(relativePath2, 50);

    char relativePath3[256];
    SDL_snprintf(relativePath3, sizeof(relativePath3), "%s%s", basePath, "font_0.ttf");
    font_3 = TTF_OpenFont(relativePath3, 300);

    char relativePath1[256];
    SDL_snprintf(relativePath1, sizeof(relativePath1), "%s%s", basePath, "font_1.ttf");
    font_1 = TTF_OpenFont(relativePath1, 120);

    char relativePath4[256];
    SDL_snprintf(relativePath4, sizeof(relativePath4), "%s%s", basePath, "font_1.ttf");
    font_4 = TTF_OpenFont(relativePath4, 250);

    char relativePath5[256];
    SDL_snprintf(relativePath5, sizeof(relativePath5), "%s%s", basePath, "font_0.ttf");
    font_5 = TTF_OpenFont(relativePath5, 150);

    window = SDL_CreateWindow("Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 800, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    screenSurface = SDL_GetWindowSurface( window );
}

// 释放资源
void cleanUp() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

/***********          游戏运行主逻辑           ***********/
void runGame(bool* is_Pause){
    score = 0;
    speed = 10;
    weight_1 = 5;
    weight_2 = 35;
    weight_3 = 50;
    weight_4 = 65;
    weight_5 = 80;
    weight_6 = 95;
    Player * player = initPlayer();
    Object* objects = NULL;
    generateDelay = 1000;
    generateTimer = 2000;

    bool gameover = false;
    SDL_Event event;
    int counter = 0;

    while (!gameover) {
        while (SDL_PollEvent(&event) != 0) {
            // 退出游戏
            if (event.type == SDL_QUIT) {
                gameover = true;
            }
                // 处理玩家输入，例如按键事件
            else if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP){
                keyPressEvent(event, player, is_Pause);
            }
        }

        // 游戏暂停逻辑
        if(!(*is_Pause)){
            // 刷出新的物品
            if(SDL_GetTicks() - generateTimer > generateDelay){
                objects = generateObject(objects, player->buff == 4);
                generateTimer = SDL_GetTicks();
            }
            counter++;
            if(counter >= 70){ score++; counter = 0; }

            // 更新玩家状态
            updatePlayer(player);

            // 判断碰撞
            if(updateObject(objects, player)){
                gameover = true;
            }

            // 强化难度
            updateSpeed();
        }

        // 清空渲染器
        if(player->buff == 4){
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // 设置颜色为灰色
        }
        else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // 设置颜色为白色
        }
        SDL_RenderClear(renderer);
        SDL_RenderFillRect(renderer, NULL); // 绘制一个填充整个窗口的矩形

        // 绘图
        drawLine();
        drawPlayer(player);
        drawObject(objects, player->buff);
        drawTopBar(player);
        if(*is_Pause){
            char pauseText[10];  // 足够大以容纳整数的字符串表示
            sprintf(pauseText, "PAUSE");
            renderText(pauseText, 155, 250, 'p');
        }

        // 更新屏幕
        SDL_UpdateWindowSurface(window);
        SDL_RenderPresent(renderer);
        SDL_Delay(30);
    }
}

/**********         渲染         ***********/
// 渲染文本
void renderText(const char* text, int x, int y, char flag){
    SDL_Color textColor;
    SDL_Surface* textSurface;
    if(flag == 'r'){   // 红色大字
        textColor.r = 255; textColor.g = 0;
        textColor.b = 0; textColor.a = 255;
        textSurface = TTF_RenderText_Solid(font_1, text, textColor);
    }
    else if(flag == 'b'){   // 红色小字
        textColor.r = 255; textColor.g = 0;
        textColor.b = 0; textColor.a = 255;
        textSurface = TTF_RenderText_Solid(font_0, text, textColor);
    }
    else if(flag == 's'){   // 开始界面字
        textColor.r = 100; textColor.g = 180;
        textColor.b = 180; textColor.a = 255;
        textSurface = TTF_RenderText_Solid(font_2, text, textColor);
    }
    else if(flag == 'p'){    // 暂停游戏字
        textColor.r = 127; textColor.g = 127;
        textColor.b = 127; textColor.a = 255;
        textSurface = TTF_RenderText_Solid(font_3, text, textColor);
    }
    else if(flag == 'x'){
        textColor.r = 255; textColor.g = 255;
        textColor.b = 255; textColor.a = 255;
        textSurface = TTF_RenderText_Solid(font_4, text, textColor);
    }
    else if(flag == 'y'){
        textColor.r = 127; textColor.g = 127;
        textColor.b = 127; textColor.a = 255;
        textSurface = TTF_RenderText_Solid(font_5, text, textColor);
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect destRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &destRect);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

/************        绘图         ***********/
// 画出赛道
void drawLine(){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(renderer, 0, 240, 1200, 240);
    SDL_RenderDrawLine(renderer, 0, 470, 1200, 470);
    SDL_RenderDrawLine(renderer, 0, 700, 1200, 700);
}

// 画出玩家
void drawPlayer(Player* hero){
    //SDL_RenderCopy(renderer, image, NULL, getPlayerRect(hero));
    if(hero->buff == 1){
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, hero->rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        for (int i = 0; i < 360; ++i) {
            double angle = i * M_PI / 180.0;
            SDL_Rect* rct = hero->rect;
            int centerX = rct->x + 30, centerY = rct->y + 30;
            int circleX = centerX + 45 * cos(angle);
            int circleY = centerY + 45 * sin(angle);
            SDL_RenderDrawPoint(renderer, circleX, circleY);
        }
    }
    else if(hero->buff == 2){
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_RenderFillRect(renderer, hero->rect);
    }
    else if(hero->health == 2){
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, hero->rect);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, hero->rect);
    }

    if(hero->buff == 4){
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 设置颜色为金色
        SDL_Rect rect;
        rect.x = 70; rect.y = 0;
        rect.w = 20; rect.h = 800;
        SDL_RenderFillRect(renderer, &rect);
    }
}

// 画出物品
void drawObject(Object* objects, int index){
    Object* p = objects;
    while(p != NULL){
        // 调色
        if(p->type == 0 && index != 3) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        }
        else if(p->type == 0 && index == 3){
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        }
        else if(p->type == 5) { SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); }
        else { SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); }

        SDL_RenderFillRect(renderer, p->rect);
        p = p->next;
    }
}

// 画顶栏
void drawTopBar(Player* hero){
    // 画出分数
    char numberText[100];  // 足够大以容纳整数的字符串表示
    sprintf(numberText, "Score: %d", score);
    renderText(numberText, 1000, 50, 'b');
    // 画出buff
    char titleText[10];
    sprintf(titleText, "Buff: ");
    renderText(titleText, 700, 50, 'b');

    char buffText[50];
    if(hero->buff == 1){
        sprintf(buffText, "Extra Shield");
        renderText(buffText, 770, 50, 'b');
    }
    else if(hero->buff == 2){
        sprintf(buffText, "Invincible time");
        renderText(buffText, 770, 50, 'b');
    }
    else if(hero->buff == 3){
        sprintf(buffText, "Coin Gain");
        renderText(buffText, 770, 50, 'b');
    }
    else {
        sprintf(buffText, "None");
        renderText(buffText, 770, 50, 'b');
    }
}

/*********           交互处理          **********/
// 处理键盘交互事件
void keyPressEvent(SDL_Event event, Player* hero, bool* is_Pause){
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            // jump
            case SDLK_UP:
                if(!(*is_Pause) && !hero->binerjump){
                    if(hero->state != 2){
                        hero->state = 2;
                        hero->v = (-1) * velocity;
                    }
                    else if(hero->v > 0){
                        hero->v = (-1) * hero->v;
                        hero->binerjump = true;
                    }
                }
                break;

            //crouch
            case SDLK_DOWN:
                if(!(*is_Pause) && hero->state != 0){
                    hero->state = 0;
                    hero->crouchTimer = SDL_GetTicks();
                    hero->binerjump = false;
                }
                break;

            // move down
            case SDLK_LEFT:
                if(!(*is_Pause) && hero->position != 3){
                    hero->position++;
                    hero->rect->y += 230;
                }
                break;

            // move up
            case SDLK_RIGHT:
                if(!(*is_Pause) && hero->position != 1){
                    hero->position--;
                    hero->rect->y -= 230;
                }
                break;

            // pause
            case SDLK_SPACE:
                *is_Pause = !(*is_Pause);
                break;
            default:
                break;
        }
    }
}

// 初始化玩家
Player* initPlayer(){
    Player* hero = (Player*)malloc(sizeof(Player));
    hero->state = 1;
    hero->position = 2;
    hero->rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
    hero->rect->x = 1000;
    hero->rect->y = 410;
    hero->rect->w = playerWidth;
    hero->rect->h = playerHigh;
    hero->health = 2;
    hero->v = 0;
    hero->binerjump = false;
    hero->buff = 0;
    return hero;
}

// 更新玩家位置
void updateState(Player* hero, int index){
    switch(hero->state) {
        // stand
        case 1:
            hero->rect->y = index;
            hero->rect->h = playerHigh;
            break;

        // jump
        case 2:
            hero->rect->y += hero->v;
            if(hero->v == velocity){
                hero->v = 0;
                hero->state = 1;
                hero->rect->y = index;
                hero->binerjump = false;
                break;
            }
            hero->v += grity;
            hero->rect->h = playerHigh;
            break;

        // crouch
        case 0:
            hero->rect->y = index + 30;
            hero->rect->h = playerHigh - 30;
            if(SDL_GetTicks() - hero->crouchTimer > crouchDelay){
                hero->state = 1;
                hero->rect->y = index;
                hero->rect->h = playerHigh;
            }
            break;
        default:
            printf("strange state: %d\n", hero->state);
    }
}

// 更新玩家
void updatePlayer(Player* hero){
    // 状态更新
    switch (hero->position) {
        // 1st road
        case 1:
            updateState(hero, 180);
            break;
        // 2cd road
        case 2:
            updateState(hero, 410);
            break;
        // 3rd road
        case 3:
            updateState(hero, 640);
            break;
        default:
            printf("strange position: %d\n", hero->position);
            break;
    }

    // 治愈更新
    if(hero->health == 1 && SDL_GetTicks() - hero->cureTimer > cureDelay){
        hero->health = 2;
    }

    // buff更新
    if(hero->buff != 0 && SDL_GetTicks() - hero->buffTimer > buffDelay){
        hero->buff = 0;
    }
}

// 随机刷出物体
Object* randomObject(int index, bool have_buff){
    Object* widget = (Object*)malloc(sizeof(Object));
    // 生成随机数
    int randomNum = rand() % 100;

    // none
    if(randomNum >=0 && randomNum <weight_1){
        free(widget);
        widget = NULL;
    }
    else{
        widget->next = NULL;
        widget->rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
        // coin
        if(have_buff || (randomNum >= weight_1 && randomNum < weight_2)){
            widget->type = 0;
            widget->rect->w = 30; widget->rect->h = 30;
            widget->rect->x = -40; widget->rect->y = index+20;
        }
        // low
        else if(randomNum >= weight_2 && randomNum < weight_3){
            widget->type = 1;
            widget->rect->w = 20; widget->rect->h = 50;
            widget->rect->x = -40; widget->rect->y = index;
        }
        // up
        else if(randomNum >= weight_3 && randomNum < weight_4){
            widget->type = 2;
            widget->rect->w = 20; widget->rect->h = 180;
            widget->rect->x = -40; widget->rect->y = index-170;
        }
        // mid
        else if(randomNum >= weight_4 && randomNum < weight_5){
            if(rand() % 10 < 3){ widget->type = 6; }
            else { widget->type = 3; }
            widget->rect->w = 50; widget->rect->h = 20;
            widget->rect->x = -60; widget->rect->y = index-10;
        }
        // full
        else if(randomNum >= weight_5 && randomNum < weight_6){
            widget->type = 4;
            widget->rect->w = 60; widget->rect->h = 200;
            widget->rect->x = -120; widget->rect->y = index-140;
        }
        // buff
        else{
            widget->type = 5;
            widget->rect->w = 30; widget->rect->h = 30;
            widget->rect->x = -40; widget->rect->y = index+20;
        }
    }

    return widget;
}

// 物体刷出函数
Object* generateObject(Object* objects, bool have_buff){
    // 刷出三个物品，可能为NULL
    Object* widget_1 = randomObject(180, have_buff);
    Object* widget_2 = randomObject(410, have_buff);
    Object* widget_3 = randomObject(640, have_buff);

    // 保证不会堵死
    if(widget_1 && widget_2 && widget_3 && widget_1->type == 4 && widget_2->type == 4 && widget_3->type == 4){
        free(widget_2->rect);
        free(widget_2);
        widget_2 = NULL;
    }

    if(widget_1){
        widget_1->next = objects;
        objects = widget_1;
    }
    if(widget_2){
        widget_2->next = objects;
        objects = widget_2;
    }
    if(widget_3){
        widget_3->next = objects;
        objects = widget_3;
    }
    return objects;
}

// 刷新物体状态
bool updateObject(Object* objects, Player* hero){
    Object* p = objects;
    Object* pre = NULL;
    while(p != NULL){
        // 检测碰撞
        if(checkCollision(p->rect, hero->rect)){
            if(p->type == 0){
                score = hero->buff == 3 ? score+3 : score+1;
            }
            else if(p->type == 5){
                // do buff
                int randomBuff = rand() % 4 + 1;
                hero->buff = randomBuff;
                hero->buffTimer = SDL_GetTicks();
            }
            else{
                if(hero->buff == 1){
                    hero->buff = 0;
                }
                else if(hero->buff != 2){
                    hero->health--;
                    if(hero->health == 0){
                        return true;
                    }
                    hero->cureTimer = SDL_GetTicks();
                }
            }

            if(pre == NULL){
                pre = p;
                p = p->next;
                free(pre->rect); free(pre);
                pre = NULL;
                objects = p;
            }
            else{
                pre->next = p->next;
                free(p->rect); free(p);
                p = pre->next;
            }
            continue;
        }

        // 移出屏幕的需要消除掉
        if(p->rect->x >= 1200){
            if(pre == NULL){
                pre = p;
                p = p->next;
                free(pre->rect); free(pre);
                pre = NULL;
                objects = p;
            }
            else{
                pre->next = p->next;
                free(p->rect); free(p);
                p = pre->next;
            }
        }
        else{
            if(p->type == 6){ p->rect->x += 2*speed; }
            else { p->rect->x += speed; }
            pre = p;
            p = p->next;
        }
    }
    return false;
}

// 碰撞检测
bool checkCollision(SDL_Rect* rect1, SDL_Rect* rect2){
    return rect1->x < rect2->x + rect2->w &&
           rect1->x + rect1->w > rect2->x &&
           rect1->y < rect2->y + rect2->h &&
           rect1->y + rect1->h > rect2->y;
}

// 难度递增
void updateSpeed(){
    int level = score / 50;
    if(level == 0){
        speed = 10;
    }
    else if(level == 1){
        speed = 12;
    }
    else if(level == 2){
        speed = 15;
    }
    else if(level == 3){
        speed = 20;
        generateDelay = 900;
    }
    else if(level == 4){
        speed = 24;
        generateDelay = 700;
    }
    else if(level == 5){
        generateDelay = 600;
    }
}

