/************************************************************/
/*                                                          */
/*        RunCPM + VT100 Emulator for Wio Termainal         */
/*                                                          */
/*   Wio_RunCPM_vt100_UsbKB                                 */
/*     https://github.com/robo8080/Wio_RunCPM_vt100_UsbKB   */
/*   Wio_RunCPM_vt100_CardKB                                */
/*     https://github.com/robo8080/Wio_RunCPM_vt100_CardKB  */
/*                                                          */
/*   RunCPM - Z80 CP/M 2.2 emulator                         */
/*     https://github.com/MockbaTheBorg/RunCPM              */
/*   VT100 Terminal Emulator for Wio Terminal               */
/*     https://github.com/ht-deko/vt100_wt                  */
/*                                                          */
/************************************************************/

#include <Arduino.h>
#include <SPI.h>

//------VT100_WT---------------------------------------------------------

#include <Seeed_Arduino_FreeRTOS.h>
// https://github.com/Seeed-Studio/Seeed_Arduino_FreeRTOS
#include <LovyanGFX.hpp>
// https://github.com/lovyan03/LovyanGFX
#include <SAMD51_InterruptTimer.h>
// https://github.com/Dennis-van-Gils/SAMD51_InterruptTimer

//------RunCPM-----------------------------------------------------------

#include <SdFat.h>
// https://github.com/greiman/SdFat
#include "RTC_SAMD51.h"
// https://github.com/Seeed-Studio/Seeed_Arduino_RTC
#include "DateTime.h"

//------LovyanGFX--------------------------------------------------------

static LGFX lcd;
static lgfx::Panel_ILI9341 panel;

//------Settings---------------------------------------------------------

// フォント: 53 Columns (font: 6x8) or 40 Columns (font: 8x8)
#include "fonts/font6x8sc1602b.h"             // 6x8 ドットフォント (SUNLIKE SC1602B 風)
//#include "fonts/font8x8misaki_2nd.h"          // 8x8 ドットフォント (美咲ゴシック 2nd)
// 横画面時: (53 or 40 - NORM_MARGINH_X) x (40 - NORM_MARGINH_Y)
#define NORM_MARGINH_X  1         // マージン文字数 (横)
#define NORM_MARGINH_Y  1         // マージン文字数 (縦)
// 縦画面時: (40 or 30 - NORM_MARGINV_X) x (40 - NORM_MARGINV_Y)
#define NORM_MARGINV_X  1         // マージン文字数 (横)
#define NORM_MARGINV_Y  1         // マージン文字数 (縦)

// フォント: 80 Columns (font: 4x8)
#include "fonts/font4x8yk.h"                    // 4x8 ドットフォント (@ykumano)
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
//#define USE_USBKB                 // USB Keyboard を使う
#define USE_CARDKB                // CardKB を使う

// CrdKB I2C アドレス
#define CARDKB_ADDR   0x5F

// デバッグ出力
//#define USE_DEBUGPRINT

//-----------------------------------------------------------------------

// スクリーンプレ計算用
uint16_t RSP_W;               // 実スクリーン横サイズ (px)
uint16_t RSP_H;               // 実スクリーン横サイズ (px)
uint16_t CH_W;                // フォント横サイズ (px)
uint16_t CH_H;                // フォント縦サイズ (px)
uint16_t SC_W;                // スクリーン横サイズ (char)
uint16_t SC_H;                // スクリーン縦サイズ (char)
uint16_t SCSIZE;              // スクリーン要素数 (char)
uint16_t SP_W;                // 有効スクリーン横サイズ (px)
uint16_t SP_H;                // 有効スクリーン横サイズ (px)
uint16_t MAX_CH_X;            // フォント最大横位置 (px)
uint16_t MAX_CH_Y;            // フォント最大縦位置 (px)
uint16_t MAX_SC_X;            // スクリーン最大横位置 (char)
uint16_t MAX_SC_Y;            // スクリーン最大縦位置 (char)
uint16_t MAX_SP_X;            // スクリーン最大横位置 (px)
uint16_t MAX_SP_Y;            // スクリーン最大縦位置 (px)
uint16_t MARGIN_LEFT;         // 左マージン (px)
uint16_t MARGIN_TOP;          // 上マージン (px)
uint16_t ROTATION_ANGLE = 0;  // 画面回転 (0..3)

// コマンドの長さ
PROGMEM const int CMD_LEN = 5;
// スイッチ情報
PROGMEM const int SW_PORT[5] = {WIO_5S_UP, WIO_5S_DOWN, WIO_5S_RIGHT, WIO_5S_LEFT, WIO_5S_PRESS};
bool prev_sw[5] = {false, false, false, false, false};
enum WIO_SW {SW_UP, SW_DOWN, SW_RIGHT, SW_LEFT, SW_PRESS};
PROGMEM const char SW_CMD[5][CMD_LEN] = {"\eOA", "\eOB", "\eOC", "\eOD", "\r"};
// ボタン情報
PROGMEM const int BTN_PORT[3] = {WIO_KEY_A, WIO_KEY_B, WIO_KEY_C};
bool prev_btn[3] = {false, false, false};
enum WIO_BTN {BT_A, BT_B, BT_C};
PROGMEM const char BTN_CMD[3][CMD_LEN] = {"\eOT", "\eOS", "\eOR"};
// 特殊キー情報
enum SP_KEY {KY_HOME, KY_INS, KY_DEL, KY_END, KY_PGUP, KY_PGDOWN};
PROGMEM const char KEY_CMD[7][CMD_LEN] = {"\eO1", "\eO2", "\x7F", "\eO4", "\eO5", "\eO6"};
// ローテーションテーブル
PROGMEM const int ROT_TBL[4][4] = {{0, 1, 2, 3}, {3, 2, 0, 1}, {1, 0, 3, 2}, {2, 3, 1, 0}};

/*********************************************
  ・キーボードと Wio Terminal のボタンとスイッチの対応
  +-------------+--------------+-----------+
  |  Keyboard   | Wio Terminal |  ESC SEQ  |
  +-------------+--------------+-----------+
  | [F3]/[fn] 3 | WIO_KEY_C    | [ESC] O R |
  | [F4]/[fn] 4 | WIO_KEY_B    | [ESC] O S |
  | [F5]/[fn] 5 | WIO_KEY_A    | [ESC] O T |
  | [UP]        | WIO_5S_UP    | [ESC] O A |
  | [DOWN]      | WIO_5S_DOWN  | [ESC] O B |
  | [RIGHT]     | WIO_5S_RIGHT | [ESC] O C |
  | [LEFT]      | WIO_5S_LEFT  | [ESC] O D |
  | [ENTER]     | WIO_5S_PRESS | [CR]      |
  +-------------+--------------+-----------+
  ・特殊キー
  +-------------+--------------+-----------+
  |   USB KB    |    CardKB    |  ESC SEQ  |
  +-------------+--------------+-----------+
  | [HOME]      | [Fn] ←      | [ESC] O 1 |
  | [INS]       | [Fn] 1       | [ESC] O 2 |
  | [DEL]       | [Shift] <x]  | [DEL]     |
  | [END]       | [Fn] →      | [ESC] O 4 |
  | [PAGE UP]   | [Fn] ↑      | [ESC] O 5 |
  | [PAGE DOWN] | [Fn] ↓      | [ESC] O 6 |
  | [ALT]+[ESC] | [Fn][ESC]    | [ESC] O 7 |
  +-------------+--------------+-----------+
*********************************************/

// キーボード
#if defined USE_USBKB
#undef USE_CARDKB
#endif

#if defined USE_CARDKB
#define KBD_TYPE  "CardKB"
#include <Wire.h>
#else
#define KBD_TYPE  "USB KB"
#include <KeyboardController.h>
#endif

// ヘッダファイル
#include "globals.h"

// Delays for LED blinking
#define sDELAY 50
#define DELAY 100

// PUN: device configuration
#if defined USE_PUN
File pun_dev;
int pun_open = FALSE;
#endif

// LST: device configuration
#if defined USE_LST
File lst_dev;
int lst_open = FALSE;
#endif

// 前方宣言
void resetSystem();

// Board definitions go into the "hardware" folder, if you use a board different than the
// Arduino DUE, choose/change a file from there and replace arduino/due.h here
#include "hardware/arduino/wioterm.h"

#include "host.h"
#include "abstraction_arduino.h"
#include "ram.h"
#include "console.h"
#include "cpu.h"
#include "disk.h"
#include "cpm.h"
#if defined CCP_INTERNAL
#include "ccp.h"
#endif
#include "romanconv.h"

// デバッグシリアル
#if defined USE_CARDKB
#define DebugSerial Serial
#else
#define DebugSerial Serial1
#endif

#define SERIALSPD 115200

// LED 制御用ピン
/*
  #define LED_01  D2
  #define LED_02  D3
  #define LED_03  D4
  #define LED_04  D5
*/

// スピーカー制御用ピン
#define SPK_PIN       WIO_BUZZER  // Wio Terminal

// キュー
#define QUEUE_LENGTH 100

// 文字アトリビュート用
struct TATTR {
  uint8_t Bold  : 1;      // 1
  uint8_t Faint  : 1;     // 2
  uint8_t Italic : 1;     // 3
  uint8_t Underline : 1;  // 4
  uint8_t Blink : 1;      // 5 (Slow Blink)
  uint8_t RapidBlink : 1; // 6
  uint8_t Reverse : 1;    // 7
  uint8_t Conceal : 1;    // 8
};

union ATTR {
  uint8_t value;
  struct TATTR Bits;
};

// カラーアトリビュート用 (RGB565)
PROGMEM const uint16_t aColors[] = {
  // Normal (0..7)
  0x0000, // black
  0x8000, // red
  0x0400, // green
  0x8400, // yellow
  0x0010, // blue (Dark)
  0x8010, // magenta
  0x0410, // cyan
  0xbdf7, // black
  // Bright (8..15)
  0x8410, // white
  0xf800, // red
  0x07e0, // green
  0xffe0, // yellow
  0x001f, // blue
  0xf81f, // magenta
  0x07ff, // cyan
  0xe73c  // white
};

struct TCOLOR {
  uint8_t Foreground : 4;
  uint8_t Background : 4;
};
union COLOR {
  uint8_t value;
  struct TCOLOR Color;
};

// 環境設定用
struct TMODE {
  bool Reserved2 : 1;  // 2
  bool Reserved4 : 1;  // 4:
  bool Reserved12 : 1; // 12:
  bool CrLf : 1;       // 20: LNM (Line feed new line mode)
  bool Reserved33 : 1; // 33:
  bool UndelineCursor : 1; // 34: WYULCURM (Undeline Cursor Mode)
  bool ADM3A : 1; // 99: ADM-3A (/ TeleVideo TS803) Mode
  uint8_t Reverse : 1;
};

union MODE {
  uint8_t value;
  struct TMODE Flgs;
};

struct TMODE_EX {
  bool Reserved1 : 1;     // 1 DECCKM (Cursor Keys Mode)
  bool Reserved2 : 1;     // 2 DECANM (ANSI/VT52 Mode)
  bool Cols132 : 1;       // 3 DECCOLM (Column Mode)
  bool Reserved4 : 1;     // 4 DECSCLM (Scrolling Mode)
  bool ScreenReverse : 1; // 5 DECSCNM (Screen Mode)
  bool Reserved6 : 1;     // 6 DECOM (Origin Mode)
  bool WrapLine  : 1;     // 7 DECAWM (Autowrap Mode)
  bool Reserved8 : 1;     // 8 DECARM (Auto Repeat Mode)
  bool Reserved9 : 1;     // 9 DECINLM (Interlace Mode)
  uint16_t Reverse : 7;
};

union MODE_EX {
  uint16_t value;
  struct TMODE_EX Flgs;
};

// 色
PROGMEM const uint8_t clBlack   = 0;
PROGMEM const uint8_t clRed     = 1;
PROGMEM const uint8_t clGreen   = 2;
PROGMEM const uint8_t clYellow  = 3;
PROGMEM const uint8_t clBlue    = 4;
PROGMEM const uint8_t clMagenta = 5;
PROGMEM const uint8_t clCyan    = 6;
PROGMEM const uint8_t clWhite   = 7;

// デフォルト
PROGMEM const uint8_t defaultMode = 0b00001000;
PROGMEM const uint16_t defaultModeEx = 0b0000000001000100;
PROGMEM const union ATTR defaultAttr = {0b00000000};
PROGMEM const union COLOR defaultColor = {(BACK_COLOR << 4) | FORE_COLOR};

// スクロール有効行
uint16_t M_TOP    = 0;        // スクロール行上限
uint16_t M_BOTTOM;            // スクロール行下限

// フォント先頭アドレス
uint8_t* ofontTop;
uint8_t* fontTop;

// バッファ
uint8_t screen[80 * 30];            // スクリーンバッファ
uint8_t attrib[80 * 30];            // 文字アトリビュートバッファ
uint8_t colors[80 * 30];            // カラーアトリビュートバッファ
uint8_t tabs[80];                   // タブ位置バッファ
unsigned char fontbuf[256 * 8 + 3]; // フォントバッファ

// 状態
PROGMEM enum class em {NONE,  ES, CSI, CSI2, LSC, G0S, G1S, SVA, LC1, LC2, EGR};
em escMode = em::NONE;         // エスケープシーケンスモード
bool isShowCursor = false;     // カーソル表示中か？
bool canShowCursor = false;    // カーソル表示可能か？
bool lastShowCursor = false;   // 前回のカーソル表示状態
bool hasParam = false;         // <ESC> [ がパラメータを持っているか？
bool isNegative = false;       // パラメータにマイナス符号が付いているか？
bool isDECPrivateMode = false; // DEC Private Mode (<ESC> [ ?)
bool isGradientBold = false;   // グラデーションボールド
union MODE mode = {defaultMode};
union MODE_EX mode_ex = {defaultModeEx};

// キー
int key;
PROGMEM const uint8 KEY_TBL[48] =  // キー変換テーブル
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x1c, 0x1f, 0x1b, 0x1d, 0x00,
  0x00, 0x11, 0x17, 0x05, 0x12, 0x14, 0x19, 0x15, 0x09, 0x0f, 0x10, 0x00,
  0x00, 0x00, 0x01, 0x13, 0x04, 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0c, 0x00,
  0x00, 0x00, 0x1a, 0x18, 0x03, 0x16, 0x02, 0x0e, 0x0d, 0x00, 0x00, 0x00
};

// 前回位置情報
int16_t p_XP = 0;
int16_t p_YP = 0;

// カレント情報
int16_t XP = 0;
int16_t YP = 0;
union ATTR cAttr   = defaultAttr;
union COLOR cColor = defaultColor;

// バックアップ情報
int16_t prev_XP = 0;
int16_t prev_YP = 0;
union ATTR bAttr   = defaultAttr;
union COLOR bColor = defaultColor;

// CSI / EGR パラメータ
int16_t nVals = 0;
int16_t vals[10] = {};

// カーソル描画用
bool needCursorUpdate = false;
bool hideCursor = false;
PROGMEM const unsigned long TIMER_PERIOD = 200000; // 200ms

// キーボード制御用
#if !defined USE_CARDKB
USBHost usb;
KeyboardController keyboard(usb);
#endif

// マクロ
#define swap(a, b) { uint16_t t = a; a = b; b = t; }
#define LOBYTE(x) ((x) & 0xff)
#define HIBYTE(x) (((x) >> 8) & 0xff)

// -----------------------------------------------------------------------------

// 輝度を落とした色(太字表示に使用)の生成
static inline uint16_t RGB565dark(uint16_t col) {
  uint16_t r = 0;
  uint16_t g = 0;
  uint16_t b = 0;
  uint16_t res = 0;

#if 1
  // 輝度75%
  r = ((col & 0xf800) >> 11) * 3 / 4;
  g = ((col & 0x07e0) >> 5) * 3 / 4;
  b =  (col & 0x001f) * 3 / 4;
#else
  // 輝度66%
  r = ((col & 0xf800) >> 11) * 2 / 3;
  g = ((col & 0x07e0) >> 5) * 2 / 3;
  b =  (col & 0x001f) * 2 / 3;
#endif
  res = (r << 11) | (g << 5) | b;

  return res;
}

// システムリセット (CP/M のコールドブート)
void resetSystem() {
  NVIC_SystemReset();
}

#if !defined USE_CARDKB
// イベント: キーを押した
void keyPressed() {
  //printKey();
}

// イベント: キーを離した
void keyReleased() {
  printKey();
}
#endif

// キー押下処理
void printKey() {

  char c = 0;
#if defined USE_CARDKB
  if (Wire.requestFrom(CARDKB_ADDR, 1))
    c = Wire.read();
  needCursorUpdate = (c > 0x00) && (c < 0x80);
#else
  c = keyboard.getKey();
  int key = keyboard.getOemKey();
  int mod = keyboard.getModifiers();
  needCursorUpdate = (c > 0x00) && (c < 0x80);
  if ((key == 0x29) && (mod == 4))
    needCursorUpdate = false;
#endif

  if (needCursorUpdate) {
    if (!toKana(c))
      return;
    xQueueSend(xQueue, &c, 0);
  } else {
    needCursorUpdate = true;
#if defined USE_CARDKB
    switch (c) {
      case 0x80:          // Fn-Esc
        isConvert = (mask8bit & 0x80) ? !isConvert : false;
        rLen = 0;
        needCursorUpdate = false;
        break;
      case 0x81:          // Fn-1 (Ctrl+!)
        printSpecialKey(KEY_CMD[KY_INS]);
        break;
      case 0x83:          // Fn-3 (Ctrl+#)
        printSpecialKey(BTN_CMD[BT_C]);
        break;
      case 0x84:          // Fn-4 (Ctrl+$)
        printSpecialKey(BTN_CMD[BT_B]);
        break;
      case 0x85:          // Fn-5 (Ctrl+%)
        printSpecialKey(BTN_CMD[BT_A]);
        break;
      case 0x82:          // Fn-2 (Ctrl+@)
      case 0x86:          // Fn-6 (Ctrl+^)
      case 0x87:          // Fn-7 (Ctrl+\)
      case 0x88:          // Fn-8 (Ctrl+_)
      case 0x89:          // Fn-9 (Ctrl+[)
      case 0x8a:          // Fn-0 (Ctrl+])
      case 0x8d ... 0x96: // Fn-Q..P
      case 0x9a ... 0xa2: // Fn-A..L
      case 0xa6 ... 0xac: // Fn-Z..M
        c = KEY_TBL[c - 0x80];
        xQueueSend(xQueue, &c, 0);
        break;
      case 0xb4:          // Left
        printSpecialKey(SW_CMD[SW_LEFT]);
        break;
      case 0xb5:          // Up
        printSpecialKey(SW_CMD[SW_UP]);
        break;
      case 0xb6:          // Down
        printSpecialKey(SW_CMD[SW_DOWN]);
        break;
      case 0xb7:          // Right
        printSpecialKey(SW_CMD[SW_RIGHT]);
        break;
      case 0x98:          // Fn-Left
        printSpecialKey(KEY_CMD[KY_HOME]);
        break;
      case 0x99:          // Fn-Up
        printSpecialKey(KEY_CMD[KY_PGUP]);
        break;
      case 0xA4:          // Fn-Down
        printSpecialKey(KEY_CMD[KY_PGDOWN]);
        break;
      case 0xA5:          // Fn-Right
        printSpecialKey(KEY_CMD[KY_END]);
        break;
      default:
        needCursorUpdate = false;
    }
#else
    switch (key) {
      case 0x29:          // Alt-Esc
        if (mod == 4) {
          isConvert = (mask8bit & 0x80) ? !isConvert : false;
          rLen = 0;
        }
        needCursorUpdate = false;
        break;
      case 60: // F3 (Wio Button #3)
        printSpecialKey(BTN_CMD[BT_C]);
        break;
      case 61: // F4 (Wio Button #2)
        printSpecialKey(BTN_CMD[BT_B]);
        break;
      case 62: // F5 (Wio Button #1)
        printSpecialKey(BTN_CMD[BT_A]);
        break;
      case 73: // Insert
        printSpecialKey(KEY_CMD[KY_INS]);
        break;
      case 74: // Home
        printSpecialKey(KEY_CMD[KY_HOME]);
        break;
      case 75: // Page Up
        printSpecialKey(KEY_CMD[KY_PGUP]);
        break;
      case 76: // DEL
        printSpecialKey(KEY_CMD[KY_DEL]);
        break;
      case 77: // End
        printSpecialKey(KEY_CMD[KY_END]);
        break;
      case 78: // Page Down
        printSpecialKey(KEY_CMD[KY_PGDOWN]);
        break;
      case 79: // RIGHT (Wio Switch #3)
        printSpecialKey(SW_CMD[SW_RIGHT]);
        break;
      case 80: // LEFT (Wio Switch #4)
        printSpecialKey(SW_CMD[SW_LEFT]);
        break;
      case 81: // DOWN (Wio Switch #2)
        printSpecialKey(SW_CMD[SW_DOWN]);
        break;
      case 82: // UP (Wio Switch #1)
        printSpecialKey(SW_CMD[SW_UP]);
        break;
      default:
        needCursorUpdate = false;
    }
#endif
  }
}

// 特殊キーの送信
void printSpecialKey(const char *str) {
  while (*str) xQueueSend(xQueue, (const char *)str++, 0);
}

// 指定位置の文字の更新表示
void sc_updateChar(uint16_t x, uint16_t y) {
  uint16_t idx = SC_W * y + x;
  uint16_t buflen = CH_W * CH_H;
  uint16_t buf[buflen];
  uint8_t c    = screen[idx];        // キャラクタの取得
  uint8_t* ptr = &fontTop[c * CH_H]; // フォントデータ先頭アドレス
  union ATTR a;
  union COLOR l;
  a.value = attrib[idx];             // 文字アトリビュートの取得
  l.value = colors[idx];             // カラーアトリビュートの取得
  uint16_t fore = aColors[l.Color.Foreground | (a.Bits.Blink << 3)];
  uint16_t back = aColors[l.Color.Background | (a.Bits.Blink << 3)];
  uint16_t foreDark = RGB565dark(fore);

  if (a.Bits.Reverse) swap(fore, back);
  if (mode_ex.Flgs.ScreenReverse) swap(fore, back);
  lcd.setAddrWindow(MARGIN_LEFT + x * CH_W , MARGIN_TOP + y * CH_H, CH_W, CH_H);
  uint16_t cnt = 0;
  for (int i = 0; i < CH_H; i++) {
    bool prev = (a.Bits.Underline && (i == MAX_CH_Y));
    for (int j = 0; j < CH_W; j++) {
      bool pset = (a.Bits.Conceal) ? false : ((*ptr) & (0x80 >> j));
      if (isGradientBold) {
        if (pset)
          buf[cnt] = fore;
        else
          buf[cnt] = (prev) ? foreDark : back;
      } else {
        buf[cnt] = (pset || prev) ? fore : back;
      }
      if (a.Bits.Bold)
        prev = pset;
      cnt++;
    }
    ptr++;
  }
  lcd.pushPixels(buf, buflen, true);
}

// カーソルの描画
void drawCursor(uint16_t x, uint16_t y) {
  if (mode.Flgs.UndelineCursor) {
    uint16_t buflen = CH_W;
    uint16_t buf[buflen];

    lcd.setAddrWindow(MARGIN_LEFT + x * CH_W, MARGIN_TOP + y * CH_H + CH_H - 1, CH_W, 1);
    for (uint16_t i = 0; i < buflen; i++)
      buf[i] = aColors[CURSOR_COLOR];
    lcd.pushPixels(buf, buflen, true);
  } else {
    uint16_t buflen = CH_W * CH_H;
    uint16_t buf[buflen];

    lcd.setAddrWindow(MARGIN_LEFT + x * CH_W, MARGIN_TOP + y * CH_H, CH_W, CH_H);
    for (uint16_t i = 0; i < buflen; i++)
      buf[i] = aColors[CURSOR_COLOR];
    lcd.pushPixels(buf, buflen, true);
  }
}

// カーソルの表示
void dispCursor(bool forceupdate) {
  if (escMode != em::NONE)
    return;
  if (hideCursor)
    return;
  if (!forceupdate)
    isShowCursor = !isShowCursor;
  if (isShowCursor)
    drawCursor(XP, YP);
  if (lastShowCursor || (forceupdate && isShowCursor))
    sc_updateChar(p_XP, p_YP);
  if (isShowCursor) {
    p_XP = XP;
    p_YP = YP;
  }
  if (!forceupdate)
    canShowCursor = false;
  lastShowCursor = isShowCursor;
  needCursorUpdate = false;
}

// 指定行をLCD画面に反映
// 引数
//  ln:行番号（0～29)
void sc_updateLine(uint16_t ln) {
  uint8_t c;
  uint8_t dt;
  uint16_t buf[2][SP_W];
  uint16_t cnt, idx;
  union ATTR a;
  union COLOR l;

  for (uint16_t i = 0; i < CH_H; i++) {            // 1文字高さ分ループ
    cnt = 0;
    for (uint16_t clm = 0; clm < SC_W; clm++) {    // 横文字数分ループ
      idx = ln * SC_W + clm;
      c  = screen[idx];                            // キャラクタの取得
      a.value = attrib[idx];                       // 文字アトリビュートの取得
      l.value = colors[idx];                       // カラーアトリビュートの取得
      uint16_t fore = aColors[l.Color.Foreground | (a.Bits.Blink << 3)];
      uint16_t back = aColors[l.Color.Background | (a.Bits.Blink << 3)];
      uint16_t foreDark = RGB565dark(fore);
      if (a.Bits.Reverse) swap(fore, back);
      if (mode_ex.Flgs.ScreenReverse) swap(fore, back);
      dt = fontTop[c * CH_H + i];                  // 文字内i行データの取得
      bool prev = (a.Bits.Underline && (i == MAX_CH_Y));
      for (uint16_t j = 0; j < CH_W; j++) {
        bool pset = (a.Bits.Conceal) ? false : (dt & (0x80 >> j));
        if (isGradientBold) {
          if (pset)
            buf[i & 1][cnt] = fore;
          else
            buf[i & 1][cnt] = (prev) ? foreDark : back;
        } else {
          buf[i & 1][cnt] = (pset || prev) ? fore : back;
        }
        if (a.Bits.Bold)
          prev = pset;
        cnt++;
      }
    }
    lcd.pushPixels(buf[i & 1], SP_W, true);
  }
}

// カーソルをホーム位置へ移動
void setCursorToHome() {
  XP = 0;
  YP = 0;
}

// カーソル位置と属性の初期化
void initCursorAndAttribute() {
  cAttr.value = defaultAttr.value;
  cColor.value = defaultColor.value;
  memset(tabs, 0x00, SC_W);
  for (int i = 0; i < SC_W; i += 8)
    tabs[i] = 1;
  setTopAndBottomMargins(1, SC_H);
  //mode.value = defaultMode;
  //mode_ex.value = defaultModeEx;
}

// 一行スクロール
// (DECSTBM の影響を受ける)
void scroll() {
  if (mode.Flgs.CrLf) XP = 0;
  YP++;
  if (YP > M_BOTTOM) {
    uint16_t n = SCSIZE - SC_W - ((M_TOP + MAX_SC_Y - M_BOTTOM) * SC_W);
    uint16_t idx = SC_W * M_BOTTOM;
    uint16_t idx2;
    uint16_t idx3 = M_TOP * SC_W;
    memmove(&screen[idx3], &screen[idx3 + SC_W], n);
    memmove(&attrib[idx3], &attrib[idx3 + SC_W], n);
    memmove(&colors[idx3], &colors[idx3 + SC_W], n);
    for (int x = 0; x < SC_W; x++) {
      idx2 = idx + x;
      screen[idx2] = 0;
      attrib[idx2] = defaultAttr.value;
      colors[idx2] = defaultColor.value;
    }
    lcd.setAddrWindow(MARGIN_LEFT, M_TOP * CH_H + MARGIN_TOP, SP_W, ((M_BOTTOM + 1) * CH_H) - (M_TOP * CH_H));
    for (int y = M_TOP; y <= M_BOTTOM; y++)
      sc_updateLine(y);
    YP = M_BOTTOM;
  }
}

// パラメータのクリア
void clearParams(em m) {
  escMode = m;
  isDECPrivateMode = false;
  nVals = 0;
  memset(vals, 0, sizeof(vals));
  hasParam = false;
  isNegative = false;
}

// 文字描画
void printChar(char c) {
  // [ESC] キー
  if (c == 0x1b) {
    escMode = em::ES;   // エスケープシーケンス開始
    return;
  }
  // エスケープシーケンス
  if (escMode == em::ES) {
    switch (c) {
      case '[':
        // Control Sequence Introducer (CSI) シーケンス へ
        clearParams(em::CSI);
        break;
      case '#':
        // Line Size Command  シーケンス へ
        clearParams(em::LSC);
        break;
      case '(':
        // G0 セット シーケンス へ
        clearParams(em::G0S);
        break;
      case ')':
        // G1 セット シーケンス へ
        clearParams(em::G1S);
        break;
      case 'G':
        if (mode.Flgs.ADM3A) {
          // Set Video Attributes (ADM-3A): 属性変更 シーケンス へ
          clearParams(em::SVA);
        } else {
          unknownSequence(escMode, c);
        }
        break;
#if defined USE_EGR
      case '%':
        // EGR セット シーケンス へ
        clearParams(em::EGR);
        break;
#endif
      default:
        // <ESC> xxx: エスケープシーケンス
        switch (c) {
          case '7':
            // DECSC (Save Cursor): カーソル位置と属性を保存
            saveCursor();
            break;
          case '8':
            // DECRC (Restore Cursor): 保存したカーソル位置と属性を復帰
            restoreCursor();
            break;
          case '=':
            if (mode.Flgs.ADM3A) {
              // Load Cursor Row and Column (ADM-3A): カーソル位置指定 シーケンス へ
              clearParams(em::LC1);
              return;
            } else {
              // DECKPAM (Keypad Application Mode): アプリケーションキーパッドモードにセット
              keypadApplicationMode();
              break;
            }
          case '>':
            // DECKPNM (Keypad Numeric Mode): 数値キーパッドモードにセット
            keypadNumericMode();
            break;
          case 'D':
            // IND (Index): カーソルを一行下へ移動
            index(1);
            break;
          case 'E':
            // NEL (Next Line): 改行、カーソルを次行の最初へ移動
            nextLine();
            break;
          case 'H':
            // HTS (Horizontal Tabulation Set): 現在の桁位置にタブストップを設定
            horizontalTabulationSet();
            break;
          case 'M':
            // RI (Reverse Index): カーソルを一行上へ移動
            reverseIndex(1);
            break;
          case 'Z':
            // DECID (Identify): 端末IDシーケンスを送信
            identify();
            break;
          case 'c':
            // RIS (Reset To Initial State): リセット
            resetToInitialState();
            break;
          case 'T':
            if (mode.Flgs.ADM3A) {
              eraseInLine(0);
              break;
            }
          default:
            // 未確認のシーケンス
            unknownSequence(escMode, c);
            break;
        }
        clearParams(em::NONE);
        break;
    }
    return;
  }

  // "[" Control Sequence Introducer (CSI) シーケンス
  int16_t v1 = 0;
  int16_t v2 = 0;

  if (escMode == em::CSI) {
    escMode = em::CSI2;
    isDECPrivateMode = (c == '?');
    if (isDECPrivateMode) return;
  }

  if (escMode == em::CSI2) {
    if (isdigit(c)) {
      // [パラメータ]
      vals[nVals] = vals[nVals] * 10 + (c - '0');
      hasParam = true;
    } else if (c == ';') {
      // [セパレータ]
      nVals++;
      hasParam = false;
    } else {
      if (hasParam) nVals++;
      switch (c) {
        case 'A':
          // CUU (Cursor Up): カーソルをPl行上へ移動
          v1 = (nVals == 0) ? 1 : vals[0];
          reverseIndex(v1);
          break;
        case 'B':
          // CUD (Cursor Down): カーソルをPl行下へ移動
          v1 = (nVals == 0) ? 1 : vals[0];
          cursorDown(v1);
          break;
        case 'C':
          // CUF (Cursor Forward): カーソルをPc桁右へ移動
          v1 = (nVals == 0) ? 1 : vals[0];
          cursorForward(v1);
          break;
        case 'D':
          // CUB (Cursor Backward): カーソルをPc桁左へ移動
          v1 = (nVals == 0) ? 1 : vals[0];
          cursorBackward(v1);
          break;
        case 'H':
        // CUP (Cursor Position): カーソルをPl行Pc桁へ移動
        case 'f':
          // HVP (Horizontal and Vertical Position): カーソルをPl行Pc桁へ移動
          v1 = (nVals == 0) ? 1 : vals[0];
          v2 = (nVals <= 1) ? 1 : vals[1];
          cursorPosition(v1, v2);
          break;
        case 'J':
          // ED (Erase In Display): 画面を消去
          v1 = (nVals == 0) ? 0 : vals[0];
          eraseInDisplay(v1);
          break;
        case 'K':
          // EL (Erase In Line) 行を消去
          v1 = (nVals == 0) ? 0 : vals[0];
          eraseInLine(v1);
          break;
        case 'L':
          // IL (Insert Line): カーソルのある行の前に Ps 行空行を挿入
          v1 = (nVals == 0) ? 1 : vals[0];
          insertLine(v1);
          break;
        case 'M':
          // DL (Delete Line): カーソルのある行から Ps 行を削除
          v1 = (nVals == 0) ? 1 : vals[0];
          deleteLine(v1);
          break;
        case 'c':
          // DA (Device Attributes): 装置オプションのレポート
          v1 = (nVals == 0) ? 0 : vals[0];
          deviceAttributes(v1);
          break;
        case 'g':
          // TBC (Tabulation Clear): タブストップをクリア
          v1 = (nVals == 0) ? 0 : vals[0];
          tabulationClear(v1);
          break;
        case 'h':
          if (isDECPrivateMode) {
            // DECSET (DEC Set Mode): モードのセット
            decSetMode(vals, nVals);
          } else {
            // SM (Set Mode): モードのセット
            setMode(vals, nVals);
          }
          break;
        case 'l':
          if (isDECPrivateMode) {
            // DECRST (DEC Reset Mode): モードのリセット
            decResetMode(vals, nVals);
          } else {
            // RM (Reset Mode): モードのリセット
            resetMode(vals, nVals);
          }
          break;
        case 'm':
          // SGR (Select Graphic Rendition): 文字修飾の設定
          if (nVals == 0) {
            nVals = 1;
            vals[0] = 0;
          }
          selectGraphicRendition(vals, nVals);
          break;
        case 'n':
          // DSR (Device Status Report): 端末状態のリポート
          v1 = (nVals == 0) ? 0 : vals[0];
          deviceStatusReport(v1);
          break;
        case 'q':
          // DECLL (Load LEDS): LED の設定
          v1 = (nVals == 0) ? 0 : vals[0];
          loadLEDs(v1);
          break;
        case 'r':
          // DECSTBM (Set Top and Bottom Margins): スクロール範囲をPt行からPb行に設定
          v1 = (nVals == 0) ? 1 : vals[0];
          v2 = (nVals <= 1) ? SC_H : vals[1];
          setTopAndBottomMargins(v1, v2);
          break;
        case 'y':
          // DECTST (Invoke Confidence Test): テスト診断を行う
          if ((nVals > 1) && (vals[0] = 2))
            invokeConfidenceTests(vals[1]);
          break;
        default:
          // 未確認のシーケンス
          unknownSequence(escMode, c);
          break;
      }
      clearParams(em::NONE);
    }
    return;
  }

  // "#" Line Size Command  シーケンス
  if (escMode == em::LSC) {
    switch (c) {
      case '3':
        // DECDHL (Double Height Line): カーソル行を倍高、倍幅、トップハーフへ変更
        doubleHeightLine_TopHalf();
        break;
      case '4':
        // DECDHL (Double Height Line): カーソル行を倍高、倍幅、ボトムハーフへ変更
        doubleHeightLine_BotomHalf();
        break;
      case '5':
        // DECSWL (Single-width Line): カーソル行を単高、単幅へ変更
        singleWidthLine();
        break;
      case '6':
        // DECDWL (Double-Width Line): カーソル行を単高、倍幅へ変更
        doubleWidthLine();
        break;
      case '8':
        // DECALN (Screen Alignment Display): 画面を文字‘E’で埋める
        screenAlignmentDisplay();
        break;
      default:
        // 未確認のシーケンス
        unknownSequence(escMode, c);
        break;
    }
    clearParams(em::NONE);
    return;
  }

  // "(" G0 セットシーケンス
  if (escMode == em::G0S) {
    // SCS (Select Character Set): G0 文字コードの設定
    setG0charset(c);
    clearParams(em::NONE);
    return;
  }

  // ")" G1 セットシーケンス
  if (escMode == em::G1S) {
    // SCS (Select Character Set): G1 文字コードの設定
    setG1charset(c);
    clearParams(em::NONE);
    return;
  }

#if defined USE_EGR
  // "%" EGR シーケンス
  if (escMode == em::EGR) {
    if (isdigit(c) || c == '-') {
      // [パラメータ]
      if (c != '-')
        vals[nVals] = vals[nVals] * 10 + (c - '0');
      else
        isNegative = true;
      hasParam = true;
    } else if (c == ';') {
      // [セパレータ]
      if (isNegative) vals[nVals] = -vals[nVals];
      nVals++;
      hasParam = false;
      isNegative = false;
    } else {
      if (hasParam) {
        if (isNegative) vals[nVals] = -vals[nVals];
        nVals++;
      }
      switch (c) {
        case 'A':
          // drawArc
          lcd.drawArc(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
          break;
        case 'a':
          // fillArc
          lcd.fillArc(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
          break;
        case 'B':
          // drawBezier
          if (nVals == 8)
            lcd.drawBezier(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7]);
          else
            lcd.drawBezier(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
          break;
        case 'b':
          // playBeep
          switch (nVals) {
            case 2:
              vals[2] = 583;
            case 1:
              vals[1] = 12;
            case 0:
              vals[0] = 1;
          }
          playBeep(vals[0], vals[1], vals[2]);
          break;
        case 'C':
          // drawCircle
          lcd.drawCircle(vals[0], vals[1], vals[2]);
          break;
        case 'c':
          // fillCircle
          lcd.fillCircle(vals[0], vals[1], vals[2]);
          break;
        case 'E':
          // drawEllipse
          lcd.drawEllipse(vals[0], vals[1], vals[2], vals[3]);
          break;
        case 'e':
          // fillEllipse
          lcd.fillEllipse(vals[0], vals[1], vals[2], vals[3]);
          break;
        case 'F':
          // setColor
          lcd.setColor(lcd.color565(vals[0], vals[1], vals[2]));
          break;
        case 'H':
          // drawFastHLine
          lcd.drawFastHLine(vals[0], vals[1], vals[2]);
          break;
        case 'h':
          // カーソル表示/非表示
          textCursorEnableMode((nVals == 0) || (vals[0] == 0));
          break;
        case 'G':
          // フォント書き換え
          if (nVals > 0)
            generateChar(vals);
          break;
        case 'g':
          // フォント復帰
          if (nVals > 0)
            restoreChar(vals);
          break;
        case 'K':
          // setBaseColor
          lcd.setBaseColor(lcd.color565(vals[0], vals[1], vals[2]));
          break;
        case 'L':
          // drawLine
          lcd.drawLine(vals[0], vals[1], vals[2], vals[3]);
          break;
        case 'O':
          // drawRoundRect
          lcd.drawRoundRect(vals[0], vals[1], vals[2], vals[3], vals[4]);
          break;
        case 'o':
          // fillRoundRect
          lcd.fillRoundRect(vals[0], vals[1], vals[2], vals[3], vals[4]);
          break;
        case 'P':
          // drawPixel
          lcd.drawPixel(vals[0], vals[1]);
          break;
        case 'R':
          // drawRect
          lcd.drawRect(vals[0], vals[1], vals[2], vals[3]);
          break;
        case 'r':
          // fillRect
          lcd.fillRect(vals[0], vals[1], vals[2], vals[3]);
          break;
        case 'S':
          // clear
          lcd.clear();
          break;
        case 's':
          // fillScreen
          lcd.fillScreen();
          break;
        case 'T':
          // drawTriangle
          lcd.drawTriangle(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
          break;
        case 't':
          // fillTriangle
          lcd.fillTriangle(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
          break;
        case 'V':
          // drawFastVLine
          lcd.drawFastVLine(vals[0], vals[1], vals[2]);
          break;
        default:
          // 未確認のシーケンス
          unknownSequence(escMode, c);
          break;
      }
      clearParams(em::NONE);
    }
    return;
  }
#endif

  if (mode.Flgs.ADM3A) {
    // LC シーケンス 1 (ADM-3A)
    if (escMode == em::LC1) {
      escMode = em::LC2;
      vals[0] = c - ' ' + 1; // 行位置
      return;
    }

    // LC シーケンス 2 (ADM-3A)
    if (escMode == em::LC2) {
      vals[1] = c - ' ' + 1; // 桁位置
      cursorPosition(vals[0], vals[1]); // 指定位置にカーソルを移動
      clearParams(em::NONE);
      return;
    }

    // SVA シーケンス (ADM-3A)
    if (escMode == em::SVA) {
      setVideoAttributes(c);
      clearParams(em::NONE);
      return;
    }

    // バックスペース (BS / ^H): カーソルを一桁左へ移動
    if (c == 0x08) {
      cursorBackward(1);
      return;
    }

    // ラインフィード (LF / ^J): カーソルを一行下へ移動
    if (c == 0x0a) {
      scroll();
      return;
    }

    // 垂直タブ (VT / ^K): カーソルを一行上へ移動
    if (c == 0x0b) {
      cursorUp(1);
      return;
    }

    // フォームフィード (FF / ^L): カーソルを一桁右へ移動
    if (c == 0x0c) {
      cursorForward(1);
      return;
    }

    // 置換 (SUB / ^Z): 画面消去
    if (c == 0x1a) {
      eraseInDisplay(2);
      return;
    }

    // レコード分離 (RS / ^^): カーソルをホーム位置へ移動
    if (c == 0x1e) {
      setCursorToHome();
      return;
    }

    // 抹消 (DEL / ^?)
    if (c == 0x7f) {
      return;
    }

  } else {
    // バックスペース (BS)
    if ((c == 0x08) || (c == 0x7f)) {
      cursorBackward(1);
      uint16_t idx = YP * SC_W + XP;
      screen[idx] = 0;
      attrib[idx] = 0;
      colors[idx] = cColor.value;
      sc_updateChar(XP, YP);
      return;
    }

    // 改行 (LF / VT / FF)
    if ((c == 0x0a) || (c == 0x0b) || (c == 0x0c)) {
      scroll();
      return;
    }
  }

  // ベル (BEL)
  if (c == 0x07) {
    playBeep(1, 12, 583);
    return;
  }

  // タブ (TAB)
  if (c == 0x09) {
    int16_t idx = -1;
    for (int16_t i = XP + 1; i < SC_W; i++) {
      if (tabs[i]) {
        idx = i;
        break;
      }
    }
    XP = (idx == -1) ? MAX_SC_X : idx;
    return;
  }

  // 復帰 (CR)
  if (c == 0x0d) {
    XP = 0;
    return;
  }

  // 通常文字
  if (XP < SC_W) {
    uint16_t idx = YP * SC_W + XP;
    screen[idx] = c;
    attrib[idx] = cAttr.value;
    colors[idx] = cColor.value;
    sc_updateChar(XP, YP);
  }

  // X 位置 + 1
  XP ++;

  // 折り返し行
  if (XP >= SC_W) {
    if (mode_ex.Flgs.WrapLine)
      scroll();
    else
      XP = MAX_SC_X;
  }
}

// 文字列描画
void printString(const char *str) {
  while (*str) printChar(*str++);
}

// エスケープシーケンス
// -----------------------------------------------------------------------------

// DECSC (Save Cursor): カーソル位置と属性を保存
void saveCursor() {
  prev_XP = XP;
  prev_YP = YP;
  bAttr.value = cAttr.value;
  bColor.value = cColor.value;
}

// DECRC (Restore Cursor): 保存したカーソル位置と属性を復帰
void restoreCursor() {
  XP = prev_XP;
  YP = prev_YP;
  cAttr.value = bAttr.value;
  cColor.value = bColor.value;
}

// DECKPAM (Keypad Application Mode): アプリケーションキーパッドモードにセット
void keypadApplicationMode() {
#if defined USE_DEBUGPRINT
  DebugSerial.println(F("Unimplement: keypadApplicationMode"));
#endif
}

// DECKPNM (Keypad Numeric Mode): 数値キーパッドモードにセット
void keypadNumericMode() {
#if defined USE_DEBUGPRINT
  DebugSerial.println(F("Unimplement: keypadNumericMode"));
#endif
}

// IND (Index): カーソルを一行下へ移動
// (DECSTBM の影響を受ける)
void index(int16_t v) {
  cursorDown(v);
}

// NEL (Next Line): 改行、カーソルを次行の最初へ移動
// (DECSTBM の影響を受ける)
void nextLine() {
  scroll();
}

// HTS (Horizontal Tabulation Set): 現在の桁位置にタブストップを設定
void horizontalTabulationSet() {
  tabs[XP] = 1;
}

// RI (Reverse Index): カーソルを一行上へ移動
// (DECSTBM の影響を受ける)
void reverseIndex(int16_t v) {
  cursorUp(v);
}

// DECID (Identify): 端末IDシーケンスを送信
void identify() {
  deviceAttributes(0); // same as DA (Device Attributes)
}

// RIS (Reset To Initial State) リセット
void resetToInitialState() {
  lcd.fillScreen((uint16_t)aColors[defaultColor.Color.Background]);
  initCursorAndAttribute();
  eraseInDisplay(2);
}

// "[" Control Sequence Introducer (CSI) シーケンス
// -----------------------------------------------------------------------------

// CUU (Cursor Up): カーソルをPl行上へ移動
// (DECSTBM の影響を受ける)
void cursorUp(int16_t v) {
  YP -= v;
  if (YP <= M_TOP) YP = M_TOP;
}

// CUD (Cursor Down): カーソルをPl行下へ移動
// (DECSTBM の影響を受ける)
void cursorDown(int16_t v) {
  YP += v;
  if (YP >= M_BOTTOM) YP = M_BOTTOM;
}

// CUF (Cursor Forward): カーソルをPc桁右へ移動
void cursorForward(int16_t v) {
  XP += v;
  if (XP >= SC_W) XP = MAX_SC_X;
}

// CUB (Cursor Backward): カーソルをPc桁左へ移動
void cursorBackward(int16_t v) {
  XP -= v;
  if (XP <= 0) XP = 0;
}

// CUP (Cursor Position): カーソルをPl行Pc桁へ移動
// HVP (Horizontal and Vertical Position): カーソルをPl行Pc桁へ移動
void cursorPosition(uint8_t y, uint8_t x) {
  YP = y - 1;
  if (YP >= SC_H) YP = MAX_SC_Y;
  XP = x - 1;
  if (XP >= SC_W) XP = MAX_SC_X;
}

// 画面を再描画
void refreshScreen() {
  lcd.setAddrWindow(MARGIN_LEFT, MARGIN_TOP, SP_W, SP_H);
  for (int i = 0; i < SC_H; i++)
    sc_updateLine(i);
}

// ED (Erase In Display): 画面を消去
void eraseInDisplay(uint8_t m) {
  uint8_t sl = 0, el = 0;
  uint16_t idx = 0, n = 0;

  switch (m) {
    case 0:
      // カーソルから画面の終わりまでを消去
      sl = YP;
      el = MAX_SC_Y;
      idx = YP * SC_W + XP;
      n   = SCSIZE - (YP * SC_W + XP);
      break;
    case 1:
      // 画面の始めからカーソルまでを消去
      sl = 0;
      el = YP;
      idx = 0;
      n = YP * SC_W + XP + 1;
      break;
    case 2:
      // 画面全体を消去
      sl = 0;
      el = MAX_SC_Y;
      idx = 0;
      n = SCSIZE;
      break;
  }

  if (m <= 2) {
    memset(&screen[idx], 0x00, n);
    memset(&attrib[idx], defaultAttr.value, n);
    memset(&colors[idx], defaultColor.value, n);
    if (m == 2)
      lcd.clear(aColors[defaultColor.Color.Background]);
    else {
      lcd.setAddrWindow(MARGIN_LEFT, sl * CH_H + MARGIN_TOP, SP_W, ((el + 1) * CH_H) - (sl * CH_H));
      for (int i = sl; i <= el; i++)
        sc_updateLine(i);
    }
  }
}

// EL (Erase In Line): 行を消去
void eraseInLine(uint8_t m) {
  uint16_t slp = 0, elp = 0;

  switch (m) {
    case 0:
      // カーソルから行の終わりまでを消去
      slp = YP * SC_W + XP;
      elp = YP * SC_W + MAX_SC_X;
      break;
    case 1:
      // 行の始めからカーソルまでを消去
      slp = YP * SC_W;
      elp = YP * SC_W + XP;
      break;
    case 2:
      // 行全体を消去
      slp = YP * SC_W;
      elp = YP * SC_W + MAX_SC_X;
      break;
  }

  if (m <= 2) {
    uint16_t n = elp - slp + 1;
    memset(&screen[slp], 0x00, n);
    memset(&attrib[slp], defaultAttr.value, n);
    memset(&colors[slp], defaultColor.value, n);
    lcd.setAddrWindow(MARGIN_LEFT, YP * CH_H + MARGIN_TOP, SP_W, ((YP + 1) * CH_H) - (YP * CH_H));
    sc_updateLine(YP);
  }
}

// IL (Insert Line): カーソルのある行の前に Ps 行空行を挿入
// (DECSTBM の影響を受ける)
void insertLine(uint8_t v) {
  int16_t rows = v;
  if (rows == 0) return;
  if (rows > ((M_BOTTOM + 1) - YP)) rows = (M_BOTTOM + 1) - YP;
  int16_t idx = SC_W * YP;
  int16_t n = SC_W * rows;
  int16_t idx2 = idx + n;
  int16_t move_rows = (M_BOTTOM + 1) - YP - rows;
  int16_t n2 = SC_W * move_rows;

  if (move_rows > 0) {
    memmove(&screen[idx2], &screen[idx], n2);
    memmove(&attrib[idx2], &attrib[idx], n2);
    memmove(&colors[idx2], &colors[idx], n2);
  }
  memset(&screen[idx], 0x00, n);
  memset(&attrib[idx], defaultAttr.value, n);
  memset(&colors[idx], defaultColor.value, n);
  lcd.setAddrWindow(MARGIN_LEFT, YP * CH_H + MARGIN_TOP, SP_W, ((M_BOTTOM + 1) * CH_H) - (YP * CH_H));
  for (int y = YP; y <= M_BOTTOM; y++)
    sc_updateLine(y);
}

// DL (Delete Line): カーソルのある行から Ps 行を削除
// (DECSTBM の影響を受ける)
void deleteLine(uint8_t v) {
  int16_t rows = v;
  if (rows == 0) return;
  if (rows > ((M_BOTTOM + 1) - YP)) rows = (M_BOTTOM + 1) - YP;
  int16_t idx = SC_W * YP;
  int16_t n = SC_W * rows;
  int16_t idx2 = idx + n;
  int16_t move_rows = (M_BOTTOM + 1) - YP - rows;
  int16_t n2 = SC_W * move_rows;
  int16_t idx3 = (M_BOTTOM + 1) * SC_W - n;

  if (move_rows > 0) {
    memmove(&screen[idx], &screen[idx2], n2);
    memmove(&attrib[idx], &attrib[idx2], n2);
    memmove(&colors[idx], &colors[idx2], n2);
  }
  memset(&screen[idx3], 0x00, n);
  memset(&attrib[idx3], defaultAttr.value, n);
  memset(&colors[idx3], defaultColor.value, n);
  lcd.setAddrWindow(MARGIN_LEFT, YP * CH_H + MARGIN_TOP, SP_W, ((M_BOTTOM + 1) * CH_H) - (YP * CH_H));
  for (int y = YP; y <= M_BOTTOM; y++)
    sc_updateLine(y);
}

// CPR (Cursor Position Report): カーソル位置のレポート
void cursorPositionReport(uint16_t y, uint16_t x) {
  String s = "\e[" + String(y, DEC) + ";" + String(x, DEC) + "R";
  for (int i = 0; i < s.length(); i++)
    xQueueSend(xQueue, (char *)s.charAt(i), 0);
}

// DA (Device Attributes): 装置オプションのレポート
// オプションのレポート
void deviceAttributes(uint8_t m) {
  String s = "\e[?1;0c";
  for (int i = 0; i < s.length(); i++)
    xQueueSend(xQueue, (char *)s.charAt(i), 0);
}

// TBC (Tabulation Clear): タブストップをクリア
void tabulationClear(uint8_t m) {
  switch (m) {
    case 0:
      // 現在位置のタブストップをクリア
      tabs[XP] = 0;
      break;
    case 3:
      // すべてのタブストップをクリア
      memset(tabs, 0x00, SC_W);
      break;
  }
}

// LNM (Line Feed / New Line Mode): 改行モード
void lineMode(bool m) {
  mode.Flgs.CrLf = m;
}

// WYULCURM (Underline Cursor Mode)
void underlinecursorMode(bool m) {
  mode.Flgs.UndelineCursor = m;
}

// : ADM3A モード
void adm3aMode(bool m) {
  mode.Flgs.ADM3A = m;
}

// DECCOLM (Select 80 or 132 Columns per Page): カラムサイズ変更
void columnMode(bool m) {
  mode_ex.Flgs.Cols132 = m;
  initScreenEx(ROTATION_ANGLE);
}

// DECSCNM (Screen Mode): 画面反転モード
void screenMode(bool m) {
  mode_ex.Flgs.ScreenReverse = m;
  refreshScreen();
}

// DECAWM (Auto Wrap Mode): 自動折り返しモード
void autoWrapMode(bool m) {
  mode_ex.Flgs.WrapLine = m;
}

// DECTCEM (Text Cursor Enable Mode): テキストカーソル有効モード
void textCursorEnableMode(bool m) {
  hideCursor = !m;
  if (hideCursor) {
    sc_updateChar(p_XP, p_YP);
    p_XP = XP;
    p_YP = YP;
  }
}

// SM (Set Mode): モードのセット
void setMode(int16_t *vals, int16_t nVals) {
  for (int16_t i = 0; i < nVals; i++) {
    switch (vals[i]) {
      case 20:
        // LNM (Line Feed / New Line Mode)
        lineMode(true);
        break;
      case 34:
        // WYULCURM (Underline Cursor Mode)
        underlinecursorMode(true);
        break;
      case 99:
        // ADM-3A (ADM-3A Mode)
        adm3aMode(true);
        break;
      default:
#if defined USE_DEBUGPRINT
        DebugSerial.print(F("Unimplement: setMode "));
        DebugSerial.println(String(vals[i], DEC));
#endif
        break;
    }
  }
}

// DECSET (DEC Set Mode): モードのセット
void decSetMode(int16_t *vals, int16_t nVals) {
  for (int16_t i = 0; i < nVals; i++) {
    switch (vals[i]) {
      case 3:
        // DECCOLM (Select 80 or 132 Columns per Page): カラムサイズ変更
        columnMode(true); // 53 文字モードへ (本来は 132 文字モード)
        break;
      case 5:
        // DECSCNM (Screen Mode): 画面反転モード
        screenMode(true);
        break;
      case 7:
        // DECAWM (Auto Wrap Mode): 自動折り返しモード
        autoWrapMode(true);
        break;
      case 25:
        // DECTCEM (Text Cursor Enable Mode): テキストカーソル有効モード
        textCursorEnableMode(true);
        break;
      default:
#if defined USE_DEBUGPRINT
        DebugSerial.print(F("Unimplement: decSetMode "));
        DebugSerial.println(String(vals[i], DEC));
#endif
        break;
    }
  }
}

// RM (Reset Mode): モードのリセット
void resetMode(int16_t *vals, int16_t nVals) {
  for (int16_t i = 0; i < nVals; i++) {
    switch (vals[i]) {
      case 20:
        // LNM (Line Feed / New Line Mode)
        lineMode(false);
        break;
      case 34:
        // WYULCURM (Underline Cursor Mode)
        underlinecursorMode(false);
        break;
      case 99:
        // ADM-3A (ADM-3A Mode)
        adm3aMode(false);
        break;
      default:
#if defined USE_DEBUGPRINT
        DebugSerial.print(F("Unimplement: resetMode "));
        DebugSerial.println(String(vals[i], DEC));
#endif
        break;
    }
  }
}

// DECRST (DEC Reset Mode): モードのリセット
void decResetMode(int16_t *vals, int16_t nVals) {
  for (int16_t i = 0; i < nVals; i++) {
    switch (vals[i]) {
      case 3:
        // DECCOLM (Select 80 or 132 Columns per Page): カラムサイズ変更
        columnMode(false); // 80 文字モードへ
        break;
      case 5:
        // DECSCNM (Screen Mode): 画面反転モード
        screenMode(false);
        break;
      case 7:
        // DECAWM (Auto Wrap Mode): 自動折り返しモード
        autoWrapMode(false);
        break;
      case 25:
        // DECTCEM (Text Cursor Enable Mode): テキストカーソル有効モード
        textCursorEnableMode(false);
        break;
      default:
#if defined USE_DEBUGPRINT
        DebugSerial.print(F("Unimplement: decResetMode "));
        DebugSerial.println(String(vals[i], DEC));
#endif
        break;
    }
  }
}

// SGR (Select Graphic Rendition): 文字修飾の設定
void selectGraphicRendition(int16_t *vals, int16_t nVals) {
  uint8_t seq = 0;
  uint16_t r, g, b, cIdx;
  bool isFore = true;
  for (int16_t i = 0; i < nVals; i++) {
    int16_t v = vals[i];
    switch (seq) {
      case 0:
        switch (v) {
          case 0:
            // 属性クリア
            cAttr.value = 0;
            cColor.value = defaultColor.value;
            break;
          case 1:
            // 太字
            cAttr.Bits.Bold = 1;
            break;
          case 4:
            // アンダーライン
            cAttr.Bits.Underline = 1;
            break;
          case 5:
            // 点滅 (明色表現)
            cAttr.Bits.Blink = 1;
            break;
          case 7:
            // 反転
            cAttr.Bits.Reverse = 1;
            break;
          case 21:
            // 二重下線 or 太字オフ
            cAttr.Bits.Bold = 0;
            break;
          case 22:
            // 太字オフ
            cAttr.Bits.Bold = 0;
            break;
          case 24:
            // アンダーラインオフ
            cAttr.Bits.Underline = 0;
            break;
          case 25:
            // 点滅 (明色表現) オフ
            cAttr.Bits.Blink = 0;
            break;
          case 27:
            // 反転オフ
            cAttr.Bits.Reverse = 0;
            break;
          case 38:
            seq = 1;
            isFore = true;
            break;
          case 39:
            // 前景色をデフォルトに戻す
            cColor.Color.Foreground = defaultColor.Color.Foreground;
            break;
          case 48:
            seq = 1;
            isFore = false;
            break;
          case 49:
            // 背景色をデフォルトに戻す
            cColor.Color.Background = defaultColor.Color.Background;
            break;
          default:
            if (v >= 30 && v <= 37) {
              // 前景色
              cColor.Color.Foreground = v - 30;
            } else if (v >= 40 && v <= 47) {
              // 背景色
              cColor.Color.Background = v - 40;
            }
            break;
        }
        break;
      case 1:
        switch (v) {
          case 2:
            // RGB
            seq = 3;
            break;
          case 5:
            // Color Index
            seq = 2;
            break;
          default:
            seq = 0;
            break;
        }
        break;
      case 2:
        // Index Color
        if (v < 256) {
          if (v < 16) {
            // ANSI カラー (16 色のインデックスカラーが使われる)
            cIdx = v;
          } else if (v < 232) {
            // 6x6x6 RGB カラー (8 色のインデックスカラー中で最も近い色が使われる)
            b = ( (v - 16)       % 6) / 3;
            g = (((v - 16) /  6) % 6) / 3;
            r = (((v - 16) / 36) % 6) / 3;
            cIdx = (b << 2) | (g << 1) | r;
          } else {
            // 24 色グレースケールカラー (2 色のグレースケールカラーが使われる)
            if (v < 244)
              cIdx = clBlack;
            else
              cIdx = clWhite;
          }
          if (isFore)
            cColor.Color.Foreground = cIdx;
          else
            cColor.Color.Background = cIdx;
        }
        seq = 0;
        break;
      case 3:
        // RGB - R
        seq = 4;
        break;
      case 4:
        // RGB - G
        seq = 5;
        break;
      case 5:
        // RGB - B
        // RGB (8 色のインデックスカラー中で最も近い色が使われる)
        r = map(vals[i - 2], 0, 255, 0, 1);
        g = map(vals[i - 1], 0, 255, 0, 1);
        b = map(vals[i - 0], 0, 255, 0, 1);
        cIdx = (b << 2) | (g << 1) | r;
        if (isFore)
          cColor.Color.Foreground = cIdx;
        else
          cColor.Color.Background = cIdx;
        seq = 0;
        break;
      default:
        seq = 0;
        break;
    }
  }
}

// DSR (Device Status Report): 端末状態のリポート
void deviceStatusReport(uint8_t m) {
  switch (m) {
    case 5:
      {
        String s = "\e[0n";
        for (int i = 0; i < s.length(); i++)
          xQueueSend(xQueue, (char *)s.charAt(i), 0);
      }
      break;
    case 6:
      cursorPositionReport(XP, YP); // CPR (Cursor Position Report)
      break;
  }
}

// DECLL (Load LEDS): LED の設定
void loadLEDs(uint8_t m) {
  switch (m) {
    case 0:
      // すべての LED をオフ
      /*
            digitalWrite(LED_01, LOW);
            digitalWrite(LED_02, LOW);
            digitalWrite(LED_03, LOW);
            digitalWrite(LED_04, LOW);
      */
      break;
    case 1:
      // LED1 をオン
      //      digitalWrite(LED_01, HIGH);
      break;
    case 2:
      // LED2 をオン
      //      digitalWrite(LED_02, HIGH);
      break;
    case 3:
      // LED3 をオン
      //      digitalWrite(LED_03, HIGH);
      break;
    case 4:
      // LED4 をオン
      //      digitalWrite(LED_04, HIGH);
      break;
  }
}

// DECSTBM (Set Top and Bottom Margins): スクロール範囲をPt行からPb行に設定
void setTopAndBottomMargins(int16_t s, int16_t e) {
  if (e <= s) return;
  M_TOP    = s - 1;
  if (M_TOP < 0) M_TOP = 0;
  if (M_TOP > MAX_SC_Y) M_TOP = MAX_SC_Y;
  M_BOTTOM = e - 1;
  if (M_BOTTOM < 0) M_BOTTOM = 0;
  if (M_BOTTOM > MAX_SC_Y) M_BOTTOM = MAX_SC_Y;
  setCursorToHome();
}

// DECTST (Invoke Confidence Test): テスト診断を行う
void invokeConfidenceTests(uint8_t m) {
  resetSystem();
}

// "]" Operating System Command (OSC) シーケンス
// -----------------------------------------------------------------------------

// "#" Line Size Command  シーケンス
// -----------------------------------------------------------------------------

// DECDHL (Double Height Line): カーソル行を倍高、倍幅、トップハーフへ変更
void doubleHeightLine_TopHalf() {
#if defined USE_DEBUGPRINT
  DebugSerial.println(F("Unimplement: doubleHeightLine_TopHalf"));
#endif
}

// DECDHL (Double Height Line): カーソル行を倍高、倍幅、ボトムハーフへ変更
void doubleHeightLine_BotomHalf() {
#if defined USE_DEBUGPRINT
  DebugSerial.println(F("Unimplement: doubleHeightLine_BotomHalf"));
#endif
}

// DECSWL (Single-width Line): カーソル行を単高、単幅へ変更
void singleWidthLine() {
#if defined USE_DEBUGPRINT
  DebugSerial.println(F("Unimplement: singleWidthLine"));
#endif
}

// DECDWL (Double-Width Line): カーソル行を単高、倍幅へ変更
void doubleWidthLine() {
#if defined USE_DEBUGPRINT
  DebugSerial.println(F("Unimplement: doubleWidthLine"));
#endif
}

// DECALN (Screen Alignment Display): 画面を文字‘E’で埋める
void screenAlignmentDisplay() {
  memset(screen, 0x45, SCSIZE);
  memset(attrib, defaultAttr.value, SCSIZE);
  memset(colors, defaultColor.value, SCSIZE);
  lcd.setAddrWindow(MARGIN_LEFT, MARGIN_TOP, SP_W, SP_H);
  for (int y = 0; y < SC_H; y++)
    sc_updateLine(y);
}

// "(" G0 Sets Sequence
// -----------------------------------------------------------------------------

// G0 文字コードの設定
void setG0charset(char c) {
#if defined USE_DEBUGPRINT
  DebugSerial.println(F("Unimplement: setG0charset"));
#endif
}

// "(" G1 Sets Sequence
// -----------------------------------------------------------------------------

// G1 文字コードの設定
void setG1charset(char c) {
#if defined USE_DEBUGPRINT
  DebugSerial.println(F("Unimplement: setG1charset"));
#endif
}

// "G" SVA Sets Sequence
// -----------------------------------------------------------------------------
void setVideoAttributes(char c) {
  uint8_t v = c - '0';
  cAttr.value = 0;                     // Normal Video
  if (v & 1) cAttr.Bits.Conceal   = 1; // Invisible
  if (v & 2) cAttr.Bits.Blink     = 1; // Blink
  if (v & 4) cAttr.Bits.Reverse   = 1; // Reverse
  if (v & 8) cAttr.Bits.Underline = 1; // Underline
}

// Unknown Sequence
// -----------------------------------------------------------------------------

// 不明なシーケンス
void unknownSequence(em m, char c) {
  String s = (m != em::NONE) ? "[ESC]" : "";
  switch (m) {
    case em::CSI:
      s = s + " [";
      break;
    case em::LSC:
      s = s + " #";
      break;
    case em::G0S:
      s = s + " (";
      break;
    case em::G1S:
      s = s + " )";
      break;
    case em::CSI2:
      s = s + " [";
      if (isDECPrivateMode)
        s = s + "?";
      break;
    case em::EGR:
      s = s + " %";
      break;
  }
#if defined USE_DEBUGPRINT
  DebugSerial.print(F("Unknown: "));
  DebugSerial.print(s);
  DebugSerial.print(F(" "));
  DebugSerial.print(c);
#endif
}

// タイマーハンドラ
void handle_timer() {
  canShowCursor = true;
  randomizeR(); // Z80 の Refresh Register に乱数を書き込む
}

// Play Tone
void playTone(int Pin, int Tone, int Duration) {
  for (long i = 0; i < Duration * 1000L; i += Tone * 2) {
    digitalWrite(Pin, HIGH);
    delayMicroseconds(Tone);
    digitalWrite(Pin, LOW);
    delayMicroseconds(Tone);
  }
}

// Play Beep
void playBeep(const uint16_t Number, const uint8_t ToneNo, const uint16_t Duration) {
  double freq = ((ToneNo == 12) && (Duration == 583)) ? 4000 : 256000.0 / (90 + 4 * ToneNo);
  if (freq <  230.6) freq =  230.6;
  if (freq > 2844.4) freq = 2844.4;
  double timeHigh = 1000000L / (2 * freq);
  for (uint16_t i = 0; i < Number; i++) {
    playTone(SPK_PIN, timeHigh, Duration);
    if (i < (Number - 1)) delay(300);
  }
  delay(20);
}

// フォント書き換え
void generateChar(int16_t *arr) {
  for (int i = 0; i < 8; i++)
    fontTop[LOBYTE(arr[0]) * 8 + i] = LOBYTE(arr[i + 1]);
}

// フォント復帰
void restoreChar(int16_t *arr) {
  for (int i = 0; i < 8; i++)
    fontTop[LOBYTE(arr[0]) * 8 + i] = ofontTop[LOBYTE(arr[0]) * 8 + i];
}

// スクリーン情報の初期化
void initScreen(uint8_t r) {
  // r:  0=0°, 1=90°, 2=180°, 0=270°
  uint8_t r2 = (r + 5) % 4;
  // r2: 0=270°, 1=0°, 2=90°, 3=180°

  TC.stopTimer();
  lcd.setRotation(r2);

  if (!mode_ex.Flgs.Cols132) {
    // 80 cols
    CH_W           = font4x8tt[0];            // フォント横サイズ
    CH_H           = font4x8tt[1];            // フォント縦サイズ
    isGradientBold = true;                    // グラデーションボールドの使用
    ofontTop       = (uint8_t*)font4x8tt + 3; // フォントバッファ先頭アドレス
    memcpy(fontbuf, font4x8tt, sizeof(font4x8tt));
  } else {
    // 53/40 cols
    CH_W           = font6x8tt[0];            // フォント横サイズ
    CH_H           = font6x8tt[1];            // フォント縦サイズ
    isGradientBold = false;                   // グラデーションボールドの使用
    ofontTop       = (uint8_t*)font6x8tt + 3; // フォントバッファ先頭アドレス
    memcpy(fontbuf, font6x8tt, sizeof(font6x8tt));
  }
  fontTop     = (uint8_t*)fontbuf + 3;  // フォントバッファ先頭アドレス
  RSP_W       = (r2 % 2 == 1) ? panel.panel_height : panel.panel_width;
  RSP_H       = (r2 % 2 == 1) ? panel.panel_width : panel.panel_height;
  SC_W        = RSP_W / CH_W;
  SC_H        = RSP_H / CH_H;
  if (!mode_ex.Flgs.Cols132) {
    // 80 cols
    SC_W -= (r2 % 2 == 1) ? WIDE_MARGINH_X : WIDE_MARGINV_X;
    SC_H -= (r2 % 2 == 1) ? WIDE_MARGINH_Y : WIDE_MARGINV_Y;
  } else {
    // 53/40 cols
    SC_W -= (r2 % 2 == 1) ? NORM_MARGINH_X : NORM_MARGINV_X;
    SC_H -= (r2 % 2 == 1) ? NORM_MARGINH_Y : NORM_MARGINV_Y;
  }
  SCSIZE      = SC_W * SC_H;            // スクリーン要素数 (char)
  SP_W        = SC_W * CH_W;            // 有効ピクセルスクリーン横サイズ
  SP_H        = SC_H * CH_H;            // 有効ピクセルスクリーン縦サイズ
  MAX_CH_X    = CH_W - 1;               // フォント最大横位置
  MAX_CH_Y    = CH_H - 1;               // フォント最大縦位置
  MAX_SC_X    = SC_W - 1;               // キャラクタスクリーン最大横位置
  MAX_SC_Y    = SC_H - 1;               // キャラクタスクリーン最大縦位置
  MAX_SP_X    = SP_W - 1;               // ピクセルスクリーン最大横位置
  MAX_SP_Y    = SP_H - 1;               // ピクセルスクリーン最大縦位置
  MARGIN_LEFT = (RSP_W - SP_W) / 2;     // 左マージン
  MARGIN_TOP  = (RSP_H - SP_H) / 2;     // 上マージン
  M_BOTTOM = MAX_SC_Y;

  TC.restartTimer(TIMER_PERIOD);
}

// スクリーン情報の初期化
void initScreenEx(uint8_t r) {
  setCursorToHome();
  initScreen(r);
  resetToInitialState();
}

// セットアップ
void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
#if defined USE_CARDKB
  Wire.begin();    // Define(SDA, SCL)
#endif
  DebugSerial.begin(SERIALSPD);
  delay(500);
  xQueue = xQueueCreate( QUEUE_LENGTH, sizeof( uint8_t ) );

  // LED の初期化
  /*
    pinMode(LED_01, OUTPUT);
    pinMode(LED_02, OUTPUT);
    pinMode(LED_03, OUTPUT);
    pinMode(LED_04, OUTPUT);
    digitalWrite(LED_01, LOW);
    digitalWrite(LED_02, LOW);
    digitalWrite(LED_03, LOW);
    digitalWrite(LED_04, LOW);
  */

  // LCD の初期化
  lcd.init();
  lcd.startWrite();
  lcd.setColorDepth(16);
  initScreenEx(ROTATION_ANGLE);

  // RTC の初期化
  rtc.begin();

  // スイッチの初期化
  for (int i = 0; i < 5; i++)
    pinMode(SW_PORT[i], INPUT_PULLUP);

  // ボタンの初期化
  for (int i = 0; i < 3; i++)
    pinMode(BTN_PORT[i], INPUT_PULLUP);

  // カーソル用タイマーの設定
  TC.startTimer(TIMER_PERIOD, handle_timer);

  // ブザーの初期化
  pinMode(SPK_PIN, OUTPUT);

#if !defined USE_CARDKB
  // キーボードの初期化
#if defined USE_DEBUGPRINT
  if (usb.Init())
    DebugSerial.println(F("USB host did not start."));
#endif
  delay(20);
  digitalWrite(PIN_USB_HOST_ENABLE, LOW);
  digitalWrite(OUTPUT_CTR_5V, HIGH);
#endif

#if defined DEBUGLOG
  _sys_deletefile((uint8 *)LogName);
#endif

  _clrscr();
  if (SC_W < 45) {
    _puts("CP/M 2.2 Emu v" VERSION " by Marcelo Dantas\r\n");
    _puts("Arduino r/w support by Krzysztof Klis\r\n");
    _puts("     Built " __DATE__ " - " __TIME__ "\r\n");
    _puts("--------------------------------------\r\n");
    _puts("CCP: " CCPname " CCP Address: 0x");
    _puthex16(CCPaddr);
    _puts("\r\nBRD: ");
    _puts(BOARD);
    _puts("(" KBD_TYPE "/");
    _puts((char*)String(F_CPU / 1000000L, DEC).c_str());
    _puts("MHz)\r\n");
  } else {
    _puts("CP/M 2.2 Emulator v" VERSION " by Marcelo Dantas\r\n");
    _puts("Arduino read/write support by Krzysztof Klis\r\n");
    _puts("      Built " __DATE__ " - " __TIME__ "\r\n");
    _puts("--------------------------------------------\r\n");
    _puts("CCP: " CCPname "    CCP Address: 0x");
    _puthex16(CCPaddr);
    _puts("\r\nBOARD: ");
    _puts(BOARD);
    _puts(" (" KBD_TYPE "/");
    _puts((char*)String(F_CPU / 1000000L, DEC).c_str());
    _puts("MHz)\r\n");
  }

#if defined board_esp32
  _puts("Initializing SPI.\r\n");
  SPI.begin(SPIINIT);
#endif
  _puts("Initializing SD card.\r\n");
  if (SD.begin(SDINIT)) {

    // AUTOEXEC
    if (SD.exists("/A/0/AUTOEXEC.SUB"))
      printSpecialKey("SUBMIT AUTOEXEC\r\n");

    if (VersionCCP >= 0x10 || SD.exists(CCPname)) {
      while (true) {
        _puts(CCPHEAD);
        _PatchCPM();
        Status = 0;
#if !defined CCP_INTERNAL
        if (!_RamLoad((char *)CCPname, CCPaddr)) {
          _puts("Unable to load the CCP.\r\nCPU halted.\r\n");
          break;
        }
        Z80reset();
        SET_LOW_REGISTER(BC, _RamRead(0x0004));
        PC = CCPaddr;
        Z80run();
#else
        _ccp();
#endif
        if (Status == 1)
          break;
#if defined USE_PUN
        if (pun_dev)
          _sys_fflush(pun_dev);
#endif
#if defined USE_LST
        if (lst_dev)
          _sys_fflush(lst_dev);
#endif
      }
    } else {
      _puts("Unable to load CP/M CCP.\r\nCPU halted.\r\n");
    }
  } else {
    _puts("Unable to initialize SD card.\r\nCPU halted.\r\n");
  }
}

// ループ
void loop() {
  digitalWrite(LED, HIGH ^ LEDinv);
  delay(DELAY);
  digitalWrite(LED, LOW ^ LEDinv);
  delay(DELAY);
  digitalWrite(LED, HIGH ^ LEDinv);
  delay(DELAY);
  digitalWrite(LED, LOW ^ LEDinv);
  delay(DELAY * 4);
}

// ループ
void loop2() {
  // RTC
  now = rtc.now();

#if defined USE_CARDKB
  printKey();
#else
  // USB
  usb.Task();
#endif

  // スイッチ
  for (int i = 0; i < 5; i++) {
    if (digitalRead(SW_PORT[i]) == LOW) {
      prev_sw[i] = true;
    } else {
      if (prev_sw[i]) {
        int v = (i == SW_PRESS) ? SW_PRESS : ROT_TBL[ROTATION_ANGLE][i];
        printSpecialKey(SW_CMD[v]);
        needCursorUpdate = true;
      }
      prev_sw[i] = false;
    }
  }

  // ボタン
  for (int i = 0; i < 3; i++) {
    if (digitalRead(BTN_PORT[i]) == LOW) {
      prev_btn[i] = true;
    } else {
      if (prev_btn[i]) {
        printSpecialKey(BTN_CMD[i]);
        needCursorUpdate = true;
      }
      prev_btn[i] = false;
    }
  }

  // カーソル表示処理
  if (canShowCursor || needCursorUpdate) {
    dispCursor(needCursorUpdate);
    needCursorUpdate = false;
  }
}
