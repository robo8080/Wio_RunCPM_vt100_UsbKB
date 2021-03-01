#ifndef RUNCPM_SETTINGS_H
#define RUNCPM_SETTINGS_H

// フォント: 53 Columns (font: 6x8) or 40 Columns (font: 8x8)
#include "../fonts/font6x8sc1602b.h"             // 6x8 ドットフォント (SUNLIKE SC1602B 風)
//#include "../fonts/font8x8misaki_2nd.h"          // 8x8 ドットフォント (美咲ゴシック 2nd)
// 横画面時: (53 or 40 - NORM_MARGINH_X) x (40 - NORM_MARGINH_Y)
#define NORM_MARGINH_X  1         // マージン文字数 (横)
#define NORM_MARGINH_Y  1         // マージン文字数 (縦)
// 縦画面時: (40 or 30 - NORM_MARGINV_X) x (40 - NORM_MARGINV_Y)
#define NORM_MARGINV_X  1         // マージン文字数 (横)
#define NORM_MARGINV_Y  1         // マージン文字数 (縦)

// フォント: 80 Columns (font: 4x8)
#include "../fonts/font4x8yk.h"                    // 4x8 ドットフォント (@ykumano)
// 横画面時: (80 - WIDE_MARGINH_X) x (40 - WIDE_MARGINH_Y)
#define WIDE_MARGINH_X  0         // マージン文字数 (横)
#define WIDE_MARGINH_Y  1         // マージン文字数 (縦)
// 縦画面時: (60 - WIDE_MARGINV_X) x (40 - WIDE_MARGINV_Y)
#define WIDE_MARGINV_X  1         // マージン文字数 (横)
#define WIDE_MARGINV_Y  1         // マージン文字数 (縦)

// 色
#define FORE_COLOR    clWhite     // 初期前景色
#define BACK_COLOR    clBlue      // 初期背景色
#define CURSOR_COLOR  clWhite     // カーソル色

// エスケープシーケンス
#define USE_EGR                   // EGR 拡張

// キーボードタイプ
#define USE_USBKB                 // USB キーボードを使う

// デバッグ出力 (エスケープシーケンス)
//#define USE_DEBUGESCSEQ           // 無効なエスケープシーケンスをデバッグ出力する

// デバッグ出力 (生データ)
//#define USE_OUTPUTRAW             // 送られてきた生データをデバッグ出力する

#endif
