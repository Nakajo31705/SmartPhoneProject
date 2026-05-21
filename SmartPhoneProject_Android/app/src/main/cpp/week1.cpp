#include "DxLib.h"
#include "math.h"

int screenW = 720;  //画面の幅
int screenH = 1280; //画面の高さ

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

///プレイヤーの構造体
struct Player
{
    int x;
    int y;
    int size;
};

///プレイヤーのサイズを初期化
void InitPlayer(Player* player)
{
    player->x = screenW / 2 - 50;
    player->y = screenH - 200;
    player->size = 100;
}

///プレイヤーを表示(Box)
void PlayerDraw(const Player* player)
{
    DrawBox(
            player->x,
            player->y,
            player->x + player->size,
            player->y + player->size,
            GetColor(0,0,255),
            TRUE
            );
}

void PlayerControl(Player* player, MobileInput* input)
{
    if(input->tap)
    {
        player->x = input->currentX;
        player->y = input->currentY;
    }
}

///描画処理
void Draw()
{
    //レーンの表示
    DrawLine(100, 0, 100, screenH, GetColor(255,255,255), 0);
    DrawLine(screenW / 2, 0, screenW / 2, screenH, GetColor(255,255,255), 0);
    DrawLine(screenW - 100, 0, screenW - 100, screenH, GetColor(255,255,255), 0);
}

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

    Player player;
    MobileInput input;

    InitPlayer(&player);
    InitMobileInput(&input);

    //メインループ
    while(ProcessMessage() == 0)
    {
        ClearDrawScreen();

        Draw();
        UpdateMobileInput(&input);

        PlayerDraw(&player);
        PlayerControl(&player, &input);
        ScreenFlip();
    }

    DxLib_End();
    return 0;
}