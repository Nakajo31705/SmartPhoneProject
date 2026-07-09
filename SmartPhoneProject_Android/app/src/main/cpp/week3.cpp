#include "DxLib.h"
#include "math.h"
#include "string.h"
#include <vector>


int screenW = 720;  //画面の幅
int screenH = 1280; //画面の高さ
int margin = 100;   //画面の余白(敵の生成範囲用)
bool isHit = false;
bool isWait = true;
float waitTime = 90.0;
float waitTimer = 0.0;
int playerLife = 3;
int targetEnemyCount = 15;



//////////////////////////////////////////
//  タッチ操作関係
//////////////////////////////////////////

///タッチ入力の座標を表す構造体
struct TouchPoint
{
    int x;  //X座標
    int y;  //Y座標
};

///タッチ入力の情報をまとめた構造体
struct MobileInput
{
    bool isTouching;    //現在タッチ中か

    bool tap;           //タップ
    bool swipe;         //スワイプ
    bool flick;         //フリック
    bool pinchIn;       //ピンチイン(縮小)
    bool pinchOut;      //ピンチアウト(拡大)

    int startX;         //タッチ開始X座標
    int startY;         //タッチ開始Y座標
    int currentX;       //現在のX座標
    int currentY;       //現在のY座標

    int startTime;      //タッチ開始時間
    int touchCount;     //タッチしている指の本数

    float previousPinchDistance;    //前フレームのピンチ距離
};

///2点間の距離を計算する
float GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1; //X方向の差
    float dy = y2 - y1; //Y方向の差

    //計算した距離を返す
    return sqrtf(dx * dx + dy * dy);
}

///タッチ入力の情報を初期化
/// 引数:
/// input MobileInputを受け取る
void InitMobileInput(MobileInput* input)
{
    input->isTouching = false;

    input->tap = false;
    input->swipe = false;
    input->flick = false;
    input->pinchIn = false;
    input->pinchOut = false;

    input->startX = 0;
    input->startY = 0;
    input->currentX = 0;
    input->currentY = 0;

    input->startTime = 0;
    input->touchCount = 0;

    input->previousPinchDistance = 0.0f;
}

///タッチ入力を毎フレーム更新し、ジェスチャーに更新
void UpdateMobileInput(MobileInput* input)
{
    //入力情報判定用しきい値
    const int swipeDistance = 80;
    const int flickTime = 200;
    const float pinchThreshold = 0.5f;

    //フレームごとにリセットするフラグ
    input->tap = false;
    input->swipe = false;
    input->flick = false;
    input->pinchIn = false;
    input->pinchOut = false;

    //タッチ数取得
    int touchNum = GetTouchInputNum();
    input->touchCount = touchNum;

    //指が離れたときの処理
    if(touchNum == 0)
    {
        if(input->isTouching)
        {
            int dx = input->currentX - input->startX;
            int dy = input->currentY - input->startY;

            //移動距離と時間で入力情報を判定
            float distance = GetDistance(
                    (float)input->startX,
                    (float)input->startY,
                    (float)input->currentX,
                    (float)input->currentY
            );

            //経過時間の計算
            int elapsedTime = GetNowCount() - input->startTime;

            if(distance < swipeDistance)
            {
                input->tap = true;
            }
            else
            {
                input->swipe = true;

                if(elapsedTime <= flickTime)
                {
                    input->flick = true;
                }
            }

            input->isTouching = false;
            input->previousPinchDistance = 0.0f;
            return;
        }
    }

    if(touchNum > 0)
    {
        //タッチ中の座標を更新
        int x = 0;
        int y = 0;

        GetTouchInput(0,&x, &y,NULL, NULL);

        if(!input->isTouching)
        {
            input->isTouching = true;

            input->startX = x;
            input->startY = y;
            input->currentX = x;
            input->currentY = y;

            input->startTime = GetNowCount();
        }
        else
        {
            input->currentX = x;
            input->currentY = y;
        }
    }

    //ピンチ処理(指が2本の時)
    if(touchNum >= 2)
    {
        int x1 = 0;
        int y1 = 0;
        int x2 = 0;
        int y2 = 0;

        GetTouchInput(0,&x1,&y1,NULL,NULL);
        GetTouchInput(1,&x2,&y2,NULL,NULL);

        float currentDistance = GetDistance(
                (float)x1,
                (float)y1,
                (float)x2,
                (float)y2
        );

        if(input->previousPinchDistance == 0.0f)
        {
            input->previousPinchDistance = currentDistance;
            return;
        }

        float diff = currentDistance - input->previousPinchDistance;

        if(fabsf(diff) > pinchThreshold)
        {
            if(diff > 0)
            {
                input->pinchOut = true;
            }
            else
            {
                input->pinchIn = true;
            }
        }

        input->previousPinchDistance = currentDistance;
    }
}

//////////////////////////////////
//構造体
//////////////////////////////////

///プレイヤーの2次元ベクトル
struct Vec2
{
    float x,y;
};

///プレイヤーの構造体
struct Player
{
    Vec2 position;  //座標
    Vec2 velocity;  //速度
    float radius;   //半径
    float friction; //減速率
};

///ステージ障害物の構造体
struct StageObj
{
    int left;   //左
    int right;  //右
    int top;    //上
    int bot;    //下
};

//敵の構造体
struct Enemy
{
    float x;
    float y;
    float radius;
    bool isAlive;
    int speed;
};

//////////////////////////////////
//エネミー関係
//////////////////////////////////

const int enemiesMax = 100;
Enemy enemies[enemiesMax];
int spawnTimer = 240;
int spawnInterval = 240;

void InitEnemy()
{
    for (int i = 0; i < enemiesMax; i++)
    {
        enemies[i].y = -50;
    }
}

///エネミーの生成処理
void SpawnEnemy()
{
    for (int i = 0; i < enemiesMax; i++)
    {
        if(enemies[i].isAlive)
            continue;

        enemies[i].isAlive = true;

        enemies[i].x = GetRand(screenW - margin * 2) + margin;
        enemies[i].y = -100;
        enemies[i].radius = 32;
        enemies[i].speed = 2;

        break;
    }
}

///エネミーの生成タイマー
void EnemyTimer()
{
    spawnTimer++;
    if(spawnTimer > spawnInterval)
    {
        spawnTimer = 0;
        SpawnEnemy();
    }
}

///エネミーの動作処理
void EnemyMove()
{
    for(int i = 0; i < enemiesMax; i++)
    {
        if(!enemies[i].isAlive)
            continue;
        enemies[i].y += enemies[i].speed;
    }
}

void EnemyDead()
{
    for(int i = 0; i < enemiesMax; i++)
    {
        if(enemies[i].y > screenH - 100)
        {
            enemies[i].isAlive = false;
            enemies[i].y = 0;
            playerLife--;
        }
    }
}

///エネミーの描画処理
void DrawEnemy()
{
    for(int i = 0; i < enemiesMax; i++)
    {
        if(!enemies[i].isAlive)
            continue;

        DrawCircle(
                enemies[i].x,
                enemies[i].y,
                enemies[i].radius,
                GetColor(255, 0, 0),
                TRUE,
                1
        );
    }
}

//////////////////////////////////
//プレイヤー関係
//////////////////////////////////

///プレイヤーの初期化
void InitPlayer(Player* player)
{
    player->position.x = screenW / 2;
    player->position.y = screenH / 2 + 500;
    player->radius = 35;
    player->velocity = {0.0,0.0};
    player->friction = 0.98;
}

///プレイヤーをリセットするときの関数
void ResetPlayer(Player *player)
{
    if(!isWait)
    {
        waitTimer++;
        if(waitTimer > waitTime || isHit)
        {
            isWait = true;
            waitTimer = 0;
            isHit = false;
            InitPlayer(player);
        }
    }
}

///プレイヤーの動作処理
void PlayerControl(Player* player, MobileInput* input)
{
    if(isWait)
    {
        if(input->swipe)
        {
            float dx = input->currentX - input->startX;
            float dy = input->currentY - input->startY;

            player->velocity.x = dx / 8;
            player->velocity.y = dy / 8;
            isWait = false;
        }
    }
}

///プレイヤーの移動処理
void PlayerMove(Player* player)
{
    //移動処理
    player->position.x += player->velocity.x;
    player->position.y += player->velocity.y;
}

///敵との接触判定を取得する関数
void CheckEnemyCollision(Player* player)
{
    for(int i = 0; i < enemiesMax; i++)
    {
        if(!enemies[i].isAlive)
            continue;

        float dx = player->position.x - enemies[i].x;
        float dy = player->position.y - enemies[i].y;

        float distance = sqrtf(dx * dx + dy * dy);

        //敵とプレイヤーの半径の合計よりdistanceが小さいなら接触
        if(distance < player->radius + enemies[i].radius) {
            isHit = true;
            targetEnemyCount--;
            enemies[i].isAlive = false;
            ResetPlayer(player);
        }
    }
}

///プレイヤーの表示
void DrawPlayer(Player* player)
{
    DrawCircle(
            player->position.x,
            player->position.y,
            player->radius,
            GetColor(255,255,255),
            TRUE,
            1
    );

    DrawFormatString(screenW / 2 - 30,0, GetColor(255,255,255), "HP:%d", playerLife);
    DrawFormatString(screenW / 2 - 60 ,20, GetColor(255,255,255), "目標撃墜数:%d", targetEnemyCount);
}

//////////////////////////////////
//ステージオブジェクト関数
//////////////////////////////////

void ObjInit(StageObj* obj)
{
    obj->left = 100;
    obj->right = (screenW - margin * 2) + margin;
    obj->top = screenH - 100 + 32;
    obj->bot = screenH;
}

void PlayerObj(StageObj* obj)
{
    DrawBox(
            obj->left,
            obj->top,
            obj->right,
            obj->bot,
            GetColor(255,255,255),
            TRUE,
            0
            );
}

//////////////////////////////////
//メインループ関数
//////////////////////////////////
MobileInput input;
Player player;
StageObj obj;
bool isGameOver = false;
bool isGameClear = false;

///ゲームリセット
void ReStart()
{
    //ゲームオーバーorゲームクリア時画面タップで再スタート
    if(isGameOver && input.tap || isGameClear && input.tap)
    {
        isGameOver = false;
        isGameClear = false;
        InitPlayer(&player);
        SpawnEnemy();
    }
}

///ゲームの状態を確認する関数
void GameManager()
{
    if(targetEnemyCount == 0)
    {
        isGameClear = true;
        targetEnemyCount = 15;
        InitPlayer(&player);
        InitEnemy();

    }
    if(playerLife == 0)
    {
        isGameOver = true;
        playerLife = 3;
        InitEnemy();
    }
}

///ゲームのUpdate
void UpdateGame()
{
    UpdateMobileInput(&input);
    ReStart();

    //ゲームオーバーorゲームクリア時ここより下を呼ばない
    if(isGameOver || isGameClear) return;

    GameManager();
    ResetPlayer(&player);
    PlayerControl(&player, &input);
    PlayerMove(&player);
    CheckEnemyCollision(&player);
    EnemyDead();
    EnemyTimer();
    EnemyMove();
}

///ゲームのDraw
void DrawGame()
{
    if(isGameOver)
    {
        //ゲームオーバー画面
        const char* msg1 = "GameOver";
        const char* msg2 = "Tap to ReStart";

        int w1 = GetDrawStringWidth(msg1, strlen(msg1));
        int w2 = GetDrawStringWidth(msg2, strlen(msg2));

        DrawFormatString(screenW / 2 - w1 / 2, screenH / 2, GetColor(255,0,0), msg1);
        DrawFormatString(screenW / 2 - w2 / 2, screenH / 2 + 50, GetColor(255,255,255), msg2);
    }

    if(isGameClear)
    {
        //ゲームオーバー画面
        const char* msg1 = "GameClear";
        const char* msg2 = "Tap to ReStart";

        int w1 = GetDrawStringWidth(msg1, strlen(msg1));
        int w2 = GetDrawStringWidth(msg2, strlen(msg2));

        DrawFormatString(screenW / 2 - w1 / 2, screenH / 2, GetColor(0,255,255), msg1);
        DrawFormatString(screenW / 2 - w2 / 2, screenH / 2 + 50, GetColor(255,255,255), msg2);
    }

    //ゲームオーバーorゲームクリア時ここより下を呼ばない
    if(isGameOver || isGameClear) return;
    DrawPlayer(&player);
    DrawEnemy();
    PlayerObj(&obj);
}

///ここから始まります。
int android_main()
{
    //画面サイズの設定
    SetGraphMode(screenW,screenH,32);

    //DxLibの初期化
    if(DxLib_Init() == -1)
    {
        return -1;
    }

    //裏画面に表示するようにする
    SetDrawScreen(DX_SCREEN_BACK);

    //初期化関数を呼ぶところ
    InitPlayer(&player);
    InitMobileInput(&input);
    ObjInit(&obj);

    //メインループ
    while(ProcessMessage() == 0)
    {
        UpdateGame();
        ClearDrawScreen();
        DrawGame();
        ScreenFlip();
    }

    DxLib_End();
    return 0;
}
