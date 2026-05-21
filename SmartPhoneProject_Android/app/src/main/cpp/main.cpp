#include "DxLib.h"

struct Rect{
    int x;
    int y;
    int w;
    int h;
};

struct UILayout{
    Rect topLeft;
    Rect topRight;
    Rect bottomLeft;
    Rect bottomRight;
    Rect center;
};

UILayout CreateLayout(int screenW, int screenH){
    const int panelW = 180;
    const int panelH = 90;

    const int centerW = 360;
    const int centerH = 160;

    const int margin = 20;

    UILayout layout;

    layout.topLeft =
            {
                    margin,
                    margin,
                    panelW,
                    panelH
            };

    layout.topRight =
            {
                    screenW - panelW - margin,
                    margin,
                    panelW,
                    panelH
            };

    layout.bottomLeft =
            {
                    margin,
                    screenH - panelH - margin,
                    panelW,
                    panelH
            };

    layout.bottomRight =
            {
                    screenW - panelW - margin,
                    screenH - panelH - margin,
                    panelW,
                    panelH
            };

    layout.center =
            {
                    screenW / 2 - centerW / 2,
                    screenH / 2 - centerH / 2,
                    centerW,
                    centerH
            };

    return layout;
}

void DrawUiRect(const Rect& rect, const char* label, int color){
    DrawBox(
            rect.x,
            rect.y,
            rect.x + rect.w,
            rect.y + rect.h,
            color,
            TRUE
    );

    DrawBox(
            rect.x,
            rect.y,
            rect.x + rect.w,
            rect.y + rect.h,
            GetColor(255,255,255),
            FALSE
    );

    DrawString(
            rect.x + 10,
            rect.y + 10,
            label,
            GetColor(255,255,255)
    );
}

void DrawUiLayout( const UILayout& layout ){
    DrawUiRect(layout.topLeft, "TopLeft", GetColor(80, 120, 200));
    DrawUiRect(layout.topRight, "TopRight", GetColor(80, 160, 120));
    DrawUiRect(layout.bottomLeft, "BottomLeft", GetColor(180, 120, 80));
    DrawUiRect(layout.bottomRight, "BottomRight", GetColor(180, 80, 120));
    DrawUiRect(layout.center, "Center", GetColor(120, 120, 120));
}

int android_main(){
    SetGraphMode( 720, 1280, 32);
    //DxLib初期化
    if(DxLib_Init() == -1){
        return -1;
    }

    SetDrawScreen(DX_SCREEN_BACK);
    //メインループ
    while(ProcessMessage() == 0){
        ClearDrawScreen();
        int screenW = 720;
        int screenH = 1280;
        //

        UILayout layout = CreateLayout(screenW, screenH);

        DrawUiLayout(layout);

        //
        DrawString(20, 1240, "Android UI Layout Sample", GetColor(255, 255,0));

        ScreenFlip();
    }

    DxLib_End();
    return 0;
}