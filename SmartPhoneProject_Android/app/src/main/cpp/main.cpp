#include "DxLib.h"

///長方形を表示する構造体
struct Rect{
    int x;  //左上のX座標
    int y;  //左上のY座標
    int w;  //幅
    int h;  //高さ
};

///UIの配置を管理する構造体
struct UILayout{
    Rect topLeft;       //左上
    Rect topRight;      //右上
    Rect bottomLeft;    //左下
    Rect bottomRight;   //右下
    Rect center;        //真ん中
};

///画像サイズに応じたUIレイアウトを作成
///引数:　
/// screenW 画面幅
/// screenH 画面の高さ
UILayout CreateLayout(int screenW, int screenH){

    //四隅のパネルのサイズ
    const int panelW = 10;
    const int panelH = 200;

    //中央のパネルのサイズ
    const int centerW = 360;
    const int centerH = 160;

    //画面端からの余白
    const int margin = 20;

    //レイアウト情報
    UILayout layout;

    //左上のパネル
    layout.topLeft =
            {
                    margin,
                    margin,
                    panelW,
                    panelH
            };

    //右上のパネル
    layout.topRight =
            {
                    screenW - panelW - margin,
                    margin,
                    panelW,
                    panelH
            };

    //左下のパネル
    layout.bottomLeft =
            {
                    margin,
                    screenH - panelH - margin,
                    panelW,
                    panelH
            };

    //右下のパネル
    layout.bottomRight =
            {
                    screenW - panelW - margin,
                    screenH - panelH - margin,
                    panelW,
                    panelH
            };

    //中央のパネル
    layout.center =
            {
                    screenW / 2 - centerW / 2,
                    screenH / 2 - centerH / 2,
                    centerW,
                    centerH
            };

    //完成したパネルを返す
    return layout;
}

///UI用の長方形とラベルを描画する
/// 引数:
/// rect 描画する長方形の位置・サイズ
/// label   表示する文字
/// color   塗りつぶす色
void DrawUiRect(const Rect& rect, const char* label, int color){

    //塗りつぶし矩形
    DrawBox(
            rect.x,
            rect.y,
            rect.x + rect.w,
            rect.y + rect.h,
            color,
            TRUE
    );

    //白い枠線
    DrawBox(
            rect.x,
            rect.y,
            rect.x + rect.w,
            rect.y + rect.h,
            GetColor(255,255,255),
            FALSE
    );

    //ラベルの文字を表示
    DrawString(
            rect.x + 10,
            rect.y + 10,
            label,
            GetColor(255,255,255)
    );
}

///UILayout内の全UIパネルを描画する
/// 引数:
/// UILayoutを受け取る

void DrawUiLayout( const UILayout& layout ){
   //左上
    DrawUiRect(layout.topLeft, "", GetColor(80, 120, 200));
/*     //右上
    DrawUiRect(layout.topRight, "TopRight", GetColor(80, 160, 120));
    //左下
    DrawUiRect(layout.bottomLeft, "BottomLeft", GetColor(180, 120, 80));
    //右下
    DrawUiRect(layout.bottomRight, "BottomRight", GetColor(180, 80, 120));
    //中央
    DrawUiRect(layout.center, "Center", GetColor(120, 120, 120));
 */
 }


///ここから始まります
/*int android_main(){
    //画面サイズの設定
    SetGraphMode( 720, 1280, 32);
    //DxLib初期化
    if(DxLib_Init() == -1){
        return -1;
    }

    //描画先を裏画面に設定
    SetDrawScreen(DX_SCREEN_BACK);
    //メインループ
    while(ProcessMessage() == 0){
        ClearDrawScreen();
        int screenW = 720;
        int screenH = 1280;

        //画面サイズに応じたUIレイアウトを作成して取得
        UILayout layout = CreateLayout(screenW, screenH);

        DrawUiLayout(layout);

        //DrawString(20, 1240, "Android UI Layout Sample", GetColor(255, 255,0));

        ScreenFlip();
    }

    DxLib_End();
    return 0;

}
 */