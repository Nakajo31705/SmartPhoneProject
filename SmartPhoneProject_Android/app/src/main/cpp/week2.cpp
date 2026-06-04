#include "DxLib.h"
#include "math.h"
#include "string.h"


int screenW = 720;  //画面の幅
int screenH = 1280; //画面の高さ

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

///タッチ入力を舞フレーム更新し、ジェスチャーに更新
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
//プレイヤー関係
//////////////////////////////////

///2次元ベクトル
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
};

///プレイヤーの初期化
void InitPlayer(Player* player)
{
    player->position.x = screenW / 2;
    player->position.y = screenH / 2 + 500;
    player->radius = 35;
    player->velocity = {0.0,0.0};
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
}

//////////////////////////////////
//ステージ関係
//////////////////////////////////

///ステージ障害物の構造体
struct StageObj
{
    int left;   //左
    int right;  //右
    int top;    //上
    int bot;    //下
};

//////////////////////////////////
//エネミー関係
//////////////////////////////////

struct Enemy
{
    int x;
    int y;
    int size;
};

//////////////////////////////////
//メインループ関数
//////////////////////////////////
MobileInput input;
Player player;
bool isGameOver = false;

///ゲームリセット
void ReStart()
{
    //ゲームオーバー時画面タップで再スタート
    if(isGameOver && input.tap)
    {
        isGameOver = false;
    }
}

//ゲームのUpdate
void UpdateGame()
{
    ReStart();
    UpdateMobileInput(&input);

    //ゲームオーバー時ここより下を呼ばない
    if(isGameOver) return;
}

//ゲームのDraw
void DrawGame()
{
    /*ゲームオーバー画面
    const char* msg1 = "GameOver";
    const char* msg2 = "Tap to ReStart";

    int w1 = GetDrawStringWidth(msg1, strlen(msg1));
    int w2 = GetDrawStringWidth(msg2, strlen(msg2));

    DrawFormatString(screenW / 2 - w1 / 2, screenH / 2, GetColor(255,0,0), msg1);
    DrawFormatString(screenW / 2 - w2 / 2, screenH / 2 + 50, GetColor(255,255,255), msg2);
    isGameOver = true;
     */

    //ゲームオーバー時ここより下を呼ばない
    if(isGameOver) return;
    DrawPlayer(&player);
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