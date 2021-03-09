program SCREEN;
{$I ARDUINO.INC}
var
  Mode, v, i: Integer;
begin
  Mode := 0;
  if ParamCount > 0 then
  begin
    Val(ParamStr(1), v, i);
    if (i = 0) and (v >= 0) then
      Mode := v;  
  end;    

  { +---+----+----+---------+----------+-------+ }
  { |   |bit4|bit3|bit2     |bit1      |bit0   | }
  { +---+----+----+---------+----------+-------+ }
  { | 0 |Screen   |  Block  |ADM-3A OFF|53 Cols| }
  { +---+ Rotation+---------+----------+-------+ }
  { | 1 |  (0..3) |Underline|ADM-3A ON |80 Cols| }
  { +---+---------+---------+----------+-------+ }

  if (Mode and 4) = 4 then
    Write(#$1b'[34h')  { Underline Cursor Mode }
  else
    Write(#$1b'[34l'); { Block Cursor Mode     }
  if (Mode and 2) = 2 then
    Write(#$1b'[99h')  { with ADM-3A Ext       }
  else
    Write(#$1b'[99l'); { without ADM-3A Ext    }
  if (Mode and 1) = 1 then
    Write(#$1b'[?3l')  { 80 Cols               }
  else
    Write(#$1b'[?3h'); { 53 Cols               }
  v := (Mode shr 3) and 3;
  if getRotation <> v then
    setRotation(v);
end.
