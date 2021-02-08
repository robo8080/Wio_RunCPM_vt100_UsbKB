#ifndef ROMANCONV_H
#define ROMANCONV_H

// ローマ字変換用
bool isConvert = false; // 変換中か？
uint8_t rLen = 0;       // 変換位置
char rBuf[5] = {};      // ローマ字入力バッファ

// キー変換テーブル
PROGMEM const uint8_t CONV_TBL[128] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA2, 0xA3, 0x00, 0xA5, 0xA4, 0xB0, 0xA1, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x00,
};
PROGMEM const uint8_t S_AIUEO_TBL[6] = { 0x00, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB };
PROGMEM const uint8_t S_YAYUYO_TBL[6] = { 0x00, 0xAC, 0xA8, 0xAD, 0xAA, 0xAE };

void kana_push(const char c) {
  xQueueSend(xQueue, &c, 0);
}

// 母音のインデックスを得る
int getVowelIndex(const char c) {
  switch (c) {
    case 'a':
      return (1);
    case 'i':
      return (2);
    case 'u':
      return (3);
    case 'e':
      return (4);
    case 'o':
      return (5);
    case 'n':
      return (-1);
    default:
      return (0);
  }
}

uint8_t toKana(const uint8_t ch) {

  if (!isConvert || ch < 0x20 || ch > 0x7E) {
    // 変換中でないか範囲外
    rLen = 0;
    return (ch);
  }
  uint8_t c = CONV_TBL[ch];
  if (!c) {
    // 変換対象の文字ではない
    rLen = 0;
    return (ch);
  }
  if ((c < 'a') || (c > 'z')) {
    // 変換対象だがローマ字用の文字ではない
    rLen = 0;
    kana_push(c);
    return (0);
  }

  int idx;

  switch (rLen) {
    case 0:
      // *** 1 stroke ***
      idx = getVowelIndex(c);
      if (idx > 0) {
        // アイウエオ
        kana_push(0xB1 + idx - 1);
      } else {
        rBuf[0] = c;
        rLen++;
        return (0);
      }
      break;
    case 1:
      // *** 2 stroke ***
      idx = getVowelIndex(c);
      if (idx > 0) {
        switch (rBuf[0]) {
          case 'b': // バビブベボ
          case 'h': // ハヒフヘホ
          case 'p': // パピプペポ
            kana_push(0xCA + idx - 1);
            if (rBuf[0] == 'b')
              kana_push(0xDE);
            else if (rBuf[0] == 'p')
              kana_push(0xDF);
            break;
          case 'c':
            if ( idx % 2 == 1 )
              kana_push(0xB6 + idx - 1); // カ ク コ
            else
              kana_push(0xBB + idx - 1); //  シ セ
            break;
          case 'd': // ダヂヅデド
          case 't': // タチツテト
            kana_push(0xC0 + idx - 1);
            if (rBuf[0] == 'd')
              kana_push(0xDE);
            break;
          case 'f':// ファフィフフェフォ
            kana_push(0xCC);
            if (idx != 3)
              kana_push(S_AIUEO_TBL[idx]);
            break;
          case 'g': // ガギグゲゴ
          case 'k': // カキクケコ
            kana_push(0xB6 + idx - 1);
            if (rBuf[0] == 'g')
              kana_push(0xDE);
            break;
          case 'j': // ジャジジュジェジョ
            kana_push(0xBC);
            kana_push(0xDE);
            if (idx != 2)
              kana_push(S_YAYUYO_TBL[idx]);
            break;
          case 'l': // ァィゥェォ
          case 'x': // ァィゥェォ
            kana_push(S_AIUEO_TBL[idx]);
            break;
          case 'm': // マミムメモ
            kana_push(0xCF + idx - 1);
            break;
          case 'n': // ナニヌネノ
            kana_push(0xC5 + idx - 1);
            break;
          case 'q': // クァクィククェクォ
            kana_push(0xB8);
            if (idx != 3)
              kana_push(S_AIUEO_TBL[idx]);
            break;
          case 'r': // ラリルレロ
            kana_push(0xD7 + idx - 1);
            break;
          case 's': // サシスセソ
          case 'z': // ザジズゼゾ
            kana_push(0xBB + idx - 1);
            if (rBuf[0] == 'z')
              kana_push(0xDE);
            break;
          case 'v': // ヴァヴィヴヴェヴォ
            kana_push(0xB3);
            kana_push(0xDE);
            if (idx != 3)
              kana_push(S_AIUEO_TBL[idx]);
            break;
          case 'w': // ワウィウウェヲ
            switch (idx) {
              case 1:
                kana_push(0xDC);
                break;
              case 2:
                kana_push(0xB3);
                kana_push(0xA8);
                break;
              case 3:
                kana_push(0xB3);
                break;
              case 4:
                kana_push(0xB3);
                kana_push(0xAA);
                break;
              case 5:
                kana_push(0xA6);
                break;
            }
            break;
          case 'y': // ヤイユイェヨ
            switch (idx) {
              case 1:
                kana_push(0xD4);
                break;
              case 2:
                kana_push(0xB2);
                break;
              case 3:
                kana_push(0xD5);
                break;
              case 4:
                kana_push(0xB2);
                kana_push(0xAA);
                break;
              case 5:
                kana_push(0xD6);
                break;
            }
            break;
        }
      } else {
        if ((rBuf[0] == 'n') && (idx == -1)) {
          kana_push(0xDD); // ン
        } else {
          switch (c) {
            case 'h':
            case 's':
            case 't':
            case 'w':
            case 'y':
              rBuf[1] = c;
              rLen++;
              return (0);
            default:
              break;
          }
        }
      }
      break;
    case 2:
      // *** 3 stroke ***
      idx = getVowelIndex(c);
      if (idx > 0) {
        switch (rBuf[1]) {
          case 'h':
            switch (rBuf[0]) {
              case 'c': // チャチィチュチェチョ
                kana_push(0xC1);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 't': // テャティテュテェテョ
              case 'd': // デャディデュデェデョ
                kana_push(0xC3);
                if (rBuf[0] == 'd')
                  kana_push(0xDE);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 's': // シャシシュシェショ
                kana_push(0xBC);
                if (idx != 2)
                  kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 'w': // ウァウィウウェウォ
                kana_push(0xB3);
                if (idx != 3)
                  kana_push(S_AIUEO_TBL[idx]);
                break;
              default:
                break;
            }
            break;
          case 's':
            switch (rBuf[0]) {
              case 't': // ツァツィツツェツォ
                kana_push(0xC2);
                if (idx != 3)
                  kana_push(S_AIUEO_TBL[idx]);
                break;
              default:
                break;
            }
            break;
          case 't':
            switch (rBuf[0]) {
              case 'l': // ッ
              case 'x': // ッ
                if (idx == 3)
                  kana_push(0xAF);
                break;
              default:
                break;
            }
            break;
          case 'w':
            switch (rBuf[0]) {
              case 'w': // トァトィトゥトェトォ
              case 'd': // ドァドィドゥドェドォ
                kana_push(0xC4);
                if (rBuf[0] == 'd')
                  kana_push(0xDE);
                kana_push(S_AIUEO_TBL[idx]);
                break;
              case 'f': // ファフィフゥフェフォ
                kana_push(0xCC);
                kana_push(S_AIUEO_TBL[idx]);
                break;
              case 'q': // クァクィクゥクェクォ
              case 'g': // グァグィグゥグェグォ
                kana_push(0xB8);
                if (rBuf[0] == 'g')
                  kana_push(0xDE);
                kana_push(S_AIUEO_TBL[idx]);
                break;
              case 'k': // クァ
                if (idx == 1) {
                  kana_push(0xB8);
                  kana_push(S_AIUEO_TBL[idx]);
                }
                break;
              case 's': // スァスィスゥスェスォ
                kana_push(0xBD);
                kana_push(S_AIUEO_TBL[idx]);
                break;
              default:
                break;
            }
            break;
          case 'y':
            switch (rBuf[0]) {
              case 'h': // ヒャヒィヒュヘヒョ
              case 'b': // ビャビィビュビェビョ
              case 'p': // ピャピィピュピェピョ
                kana_push(0xCB);
                if (rBuf[0] == 'd')
                  kana_push(0xDE);
                if (rBuf[0] == 'p')
                  kana_push(0xDF);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 'c': // チャチィチュチェチョ
              case 't': // チャチィチュチェチョ
              case 'd': // ヂャヂィヂュヂェヂョ
                kana_push(0xC1);
                if (rBuf[0] == 'd')
                  kana_push(0xDE);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 'f': // フャフィフュフェフョ
                kana_push(0xCC);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 'k': // キャキィキュキェキョ
              case 'g': // ギャギィギュギェギョ
                kana_push(0xB7);
                if (rBuf[0] == 'g')
                  kana_push(0xDE);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 's': // シャシィシュシェショ
              case 'j': // ジャジィジュジェジョ
              case 'z': // ジャジィジュジェジョ
                kana_push(0xBC);
                if (rBuf[0] != 's')
                  kana_push(0xDE);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 'l': // ャィュェョ
              case 'x': // ャィュェョ
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 'm': // ミャミィミュミェミョ
                kana_push(0xD0);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 'n': // ニャニィニュニェニョ
                kana_push(0xC6);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 'q': // クャクィクュクェクョ
                kana_push(0xB8);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 'r': // リャリィリュリェリョ
                kana_push(0xD8);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              case 'v': // ヴャヴィヴュヴェヴョ
                kana_push(0xB3);
                kana_push(0xDE);
                kana_push(S_YAYUYO_TBL[idx]);
                break;
              default:
                break;
            }
            break;
        }
      } else {
          switch (c) {
            case 's':
              rBuf[2] = c;
              rLen++;
              return (0);
            default:
              break;
          }
      }
      break;
    case 3:
      // *** 4 stroke ***
      idx = getVowelIndex(c);
      if ((rBuf[1] == 't') && (rBuf[2] == 's') && (idx == 3)) {
        switch (rBuf[0]) {
          case 'l': // ッ
          case 'x': // ッ
            kana_push(0xAF);
            break;
        }
      }
      break;
    default:
      break;
  }
  rLen = 0;
  return (0);
}

#endif
