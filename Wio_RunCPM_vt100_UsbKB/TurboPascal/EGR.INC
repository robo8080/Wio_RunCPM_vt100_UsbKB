{* EGR.INC *}

const
  _EGR = #$1B'%';

  { Graphics Routine }
  
  procedure Clear;
  begin
    Write(_EGR, 'S');
  end;

  procedure SetColor(r, g, b: Integer);
  begin
    Write(_EGR, r, ';', g, ';', b, 'F');
  end;

  procedure SetBaseColor(r, g, b: Integer);
  begin
    Write(_EGR, r, ';', g, ';', b, 'K');
  end;

  procedure InitGraph;
  begin
    Write(#$1B'[H'#$1B'[2J');
    SetBaseColor(0, 0, 0);
    Clear;
    SetColor(255, 255, 255);
  end;

  procedure DrawArc(x, y, r0, r1, angle0, angle1: Integer);
  begin
    Write(_EGR, x, ';', y, ';', r0, ';', r1, ';', angle0, ';', angle1, 'A');
  end;

  procedure DrawBezier(x0, y0, x1, y1, x2, y2: Integer);
  begin
    Write(_EGR, x0, ';', y0, ';', x1, ';', y1, ';', x2, ';', y2, 'B');
  end;

  procedure DrawBezier2(x0, y0, x1, y1, x2, y2, x3, y3: Integer);
  begin
    Write(_EGR, x0, ';', y0, ';', x1, ';', y1, ';', x2, ';', y2, ';', x3, ';', y3, 'B');
  end;

  procedure DrawCircle(x, y, r: Integer);
  begin
    Write(_EGR, x, ';', y, ';', r, 'C');
  end;

  procedure DrawEllipse(x, y, rx, ry: Integer);
  begin
    Write(_EGR, x, ';', y, ';', rx, ';', ry, 'E');
  end;

  procedure DrawFastHLine(x, y, w: Integer);
  begin
    Write(_EGR, x, ';', y, ';', w, 'H');
  end;

  procedure DrawEllipseArc(x, y, r0x, r1x, r0y, r1y, st, en: Integer);
  begin
    Write(_EGR, x, ';', y, ';', r0x, ';', r1x, ';', r0y, ';', r1y, ';' st, ';', en, 'I');
  end;

  procedure DrawFastVLine(x, y, h: Integer);
  begin
    Write(_EGR, x, ';', y, ';', h, 'V');
  end;

  procedure DrawLine(x0, y0, x1, y1: Integer);
  begin
    Write(_EGR, x0, ';', y0, ';', x1, ';', y1, 'L');
  end;

  procedure DrawPixel(x, y: Integer);
  begin
    Write(_EGR, x, ';', y, 'P');
  end;

  procedure DrawRect(x, y, w, h: Integer);
  begin
    Write(_EGR, x, ';', y, ';', w, ';', h, 'R');
  end;

  procedure DrawRoundRect(x, y, w, h, r: Integer);
  begin
    Write(_EGR, x, ';', y, ';', w, ';', h, ';', r, 'O');
  end;

  procedure DrawTriangle(x0, y0, x1, y1, x2, y2: Integer);
  begin
    Write(_EGR, x0, ';', y0, ';', x1, ';', x1, ';', x2, ';', y2, 'T');
  end;

  procedure FillArc(x, y, r0, r1, angle0, angle1: Integer);
  begin
    Write(_EGR, x, ';', y, ';', r0, ';', r1, ';', angle0, ';', angle1, 'a');
  end;

  procedure FillCircle(x, y, r: Integer);
  begin
    Write(_EGR, x, ';', y, ';', r, 'c');
  end;

  procedure FillEllipse(x, y, rx, ry: Integer);
  begin
    Write(_EGR, x, ';', y, ';', rx, ';', ry, 'e');
  end;

  procedure FillEllipseArc(x, y, r0x, r1x, r0y, r1y, st, en: Integer);
  begin
    Write(_EGR, x, ';', y, ';', r0x, ';', r1x, ';', r0y, ';', r1y, ';' st, ';', en, 'i');
  end;

  procedure FillRoundRect(x, y, w, h, r: Integer);
  begin
    Write(_EGR, x, ';', y, ';', w, ';', h, 'o');
  end;

  procedure FillRect(x, y, w, h: Integer);
  begin
    Write(_EGR, x, ';', y, ';', w, ';', h, 'r');
  end;

  procedure FillScreen;
  begin
    Write(_EGR, 's');
  end;

  procedure FillTriangle(x0, y0, x1, y1, x2, y2: Integer);
  begin
    Write(_EGR, x0, ';', y0, ';', x1, ';', y1, ';', x2, ';', y2, 't');
  end;

  { Misc Routine }

  procedure Beep(n, t, d: Integer);
  begin
    Write(_EGR, n, ';', t, ';', d, 'b');
  end;

  procedure HideCursor(Flg: Boolean);
  begin
    Write(_EGR, Ord(Flg), 'h');
  end;  

  procedure GenerateChar(Target: Char; D1, D2, D3, D4, D5, D6, D7, D8: Byte);
  begin
    Write(_EGR, Ord(Target), ';', D1, ';', D2, ';', D3, ';', D4, ';', D5, ';', D6, ';', D7, ';', D8, 'G');
  end;

  procedure RestoreChar(Target: Char);
  begin
    Write(_EGR, Ord(Target), 'g');
  end;

