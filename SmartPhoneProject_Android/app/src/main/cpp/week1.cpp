#include "DxLib.h"
#include "math.h"
#include "string.h"


int screenW = 720;  //画面の幅
int screenH = 1280; //画面の高さ

///エネミー生成座標の構造体
struct SpawnPoint
{
    int x;
    int y;
};

///生成座標を登録
SpawnPoint spawn[3] =
        {
                {screenW / 2, 0},
                {100, 0},
                {screenW - 100, 0},

        };

///プレイヤーが移動可能な座標の構造体
struct MovePoint
{
    int x;
    int y;
};

///プレイヤーの移動可能な座標を登録
MovePoint points[3] =
        {
                {screenW / 2, screenH - 200},
                {100, screenH - 200},
                {screenW - 100, screenH -200}
        };

///当たり判定の構造体
struct HitBox
{
    int x;
    int y;
    int width;
    int height;
};

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

//////////////////////////////////////////////
//  エネミー関係
//////////////////////////////////////////////

///エネミーの構造体
struct Enemy
{
    int x;
    int y;
    int width;
    int height;
    bool active;
    HitBox hitbox;
};

const int Max_Enemies = 2;
Enemy enemies[Max_Enemies];

///エネミーを生成
void SpawnEnemy()
{
    for(int i = 0; i < Max_Enemies; i++)
    {
        if(!enemies[i].active)
        {
            int index = rand() % 3;

            enemies[i].x = spawn[index].x;
            enemies[i].y = spawn[index].y + 200;
            enemies[i].width = 50;
            enemies[i].height = 50;

            enemies[i].active = true;

            enemies[i].hitbox.x = spawn[index].x;
            enemies[i].hitbox.y = spawn[index].y + 200;
            enemies[i].hitbox.width = 50;
            enemies[i].hitbox.height = 50;

            break;
        }
    }
}

void DrawEnemy()
{
    for(int i = 0; i < Max_Enemies; i++)
    {
        if(!enemies[i].active) continue;

        DrawBox(
                enemies[i].x - enemies[i].width,
                enemies[i].y - enemies[i].height,
                enemies[i].x + enemies[i].width,
                enemies[i].y + enemies[i].height,
                GetColor(255,0,0),
                true
                );

        //HitBoxのデバッグ
        /*
        DrawBox(
                enemies[i].hitbox.x - enemies[i].width,
                enemies[i].hitbox.y - enemies[i].height,
                enemies[i].hitbox.x + enemies[i].width,
                enemies[i].hitbox.y + enemies[i].height,
                GetColor(0,255,0),
                FALSE
        );
        */
    }
}

void EnemyUpdate()
{
    for(int i = 0; i < Max_Enemies; i++)
    {
        //
        if(!enemies[i].active) continue;

        //下方向に移動
        enemies[i].y += 6.0f;
        enemies[i].hitbox.y += 6.0f;

        if(enemies[i].y > 1350)
        {
            enemies[i].active = false;
            SpawnEnemy();
        }
    }
}

///////////////////////////////////////////////
//  プレイヤー関係
///////////////////////////////////////////////

///プレイヤーの構造体
struct Player
{
    int x;
    int y;
    int size;

    HitBox hitbox;
};

///プレイヤーのサイズと当たり判定を初期化
void InitPlayer(Player* player)
{
    //サイズの初期化
    player->x = screenW / 2;
    player->y = screenH - 200;
    player->size = 50;

    //当たり判定の初期化
    player->hitbox.x = player->x;
    player->hitbox.y = player->y;
    player->hitbox.width = player->size;
    player->hitbox.height = player->size;
}

///プレイヤーを表示(Box)
void PlayerDraw(const Player* player)
{
    DrawBox(
            player->x - player->size,
            player->y - player->size,
            player->x + player->size,
            player->y + player->size,
            GetColor(0,0,255),
            TRUE
            );

    //HitBoxのデバッグ
    /*DrawBox(
            player->hitbox.x - player->size,
            player->hitbox.y - player->size,
            player->hitbox.x + player->size,
            player->hitbox.y + player->size,
            GetColor(255,0,0),
            FALSE
    );
     */

}

///プレイヤーの移動処理
void PlayerControl(Player* player, MobileInput* input)
{
    //タップした時
    if(input->tap)
    {
        player->x = input->currentX;
        int nearestIndex = 0;
        int nearestDistance = abs(player->x - points[0].x);

        for(int i = 0; i < 3; i++)
        {
            //目標地点がタップ位置からどれくらい離れているか計算
            int distance = abs(player->x - points[i].x);
            if(distance < nearestDistance)
            {
                nearestDistance = distance;
                nearestIndex = i;
            }
        }

        //一番近い移動ポイントに移動
        player->x = points[nearestIndex].x;
        player->y = points[nearestIndex].y;

        //移動時に当たり判定も同時に移動する
        player->hitbox.x = points[nearestIndex].x;
        player->hitbox.y = points[nearestIndex].y;
        player->hitbox.width = player->size;
        player->hitbox.height = player->size;
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

///当たり判定の関数
bool CheckHit(const Player& player, const Enemy& enemy)
{
    return (
            player.hitbox.x < enemy.hitbox.x + enemy.hitbox.width &&
            player.hitbox.x + player.hitbox.width > enemy.hitbox.x &&
            player.hitbox.y < enemy.hitbox.y + enemy.hitbox.height &&
            player.hitbox.y + player.hitbox.height > enemy.hitbox.y
            );
}

Player player;
MobileInput input;
MovePoint point;

bool isGameOver = false;

///ゲームリセット
void ReStart()
{
    if(isGameOver && input.tap)
    {
        isGameOver = false;
        InitPlayer(&player);
        for(int i = 0; i < Max_Enemies; i++)
        {
            enemies[i].active = false;
            SpawnEnemy();
        }
    }
}

//ゲームのUpdate
void UpdateGame()
{
    ReStart();
    UpdateMobileInput(&input);

    if(isGameOver) return;

    PlayerControl(&player, &input);
    EnemyUpdate();
    SpawnEnemy();
}

//ゲームのDraw
void DrawGame()
{
    for(int i = 0; i < Max_Enemies; i++)
    {
        if(CheckHit(player, enemies[i]))
        {
            const char* msg1 = "GameOver";
            const char* msg2 = "Tap to ReStart";

            int w1 = GetDrawStringWidth(msg1, strlen(msg1));
            int w2 = GetDrawStringWidth(msg2, strlen(msg2));

            DrawFormatString(screenW / 2 - w1 / 2, screenH / 2, GetColor(255,0,0), msg1);
            DrawFormatString(screenW / 2 - w2 / 2, screenH / 2 + 50, GetColor(255,255,255), msg2);
            isGameOver = true;
        }
    }

    if(isGameOver) return;
    Draw();
    DrawEnemy();
    PlayerDraw(&player);



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