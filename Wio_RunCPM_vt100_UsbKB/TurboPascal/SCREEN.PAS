program SCREEN;
var
  Mode, v, i: Integer;
begin
  if (ParamCount = 0) then
    Mode := 0
  else
    begin
      Val(ParamStr(1), v, i);
      if i = 0 then
        begin
          if (v < 0) then v := 0;
          if (v > 7) then v := 7;
          Mode := v;
        end
      else
        Mode := 0;  
    end;  

  { +---+---------+-------------+-------+ }
  { |   |bit2     |bit1         |bit0   | }
  { +---+---------+-------------+-------+ }
  { | 0 |Block    |TeleVideo OFF|53 Cols| }
  { +---+---------+-------------+-------+ }
  { | 1 |Underline|TeleVideo ON |80 Cols| }
  { +---+---------+-------------+-------+ }

  if (Mode and 4) = 4 then
    Write(#$1b'[34h')  { Underline Cursor Mode }
  else
    Write(#$1b'[34l'); { Block Cursor Mode     }
  if (Mode and 2) = 2 then
    Write(#$1b'[99h')  { with TeleVideo Ext    }
  else
    Write(#$1b'[99l'); { without TeleVideo Ext }
  if (Mode and 1) = 1 then
    Write(#$1b'[?3l')  { 80 Cols }
  else
    Write(#$1b'[?3h'); { 53 Cols }
end.

