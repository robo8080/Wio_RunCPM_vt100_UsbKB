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
          if (v > 3) then v := 3;
          Mode := v;
        end
      else
        Mode := 0;  
    end;  
  if (Mode div 2) = 1 then
    Write(#$1b'[99h')  { with TeleVideo Ext    }
   else
    Write(#$1b'[99l'); { without TeleVideo Ext }
  if Odd(Mode) then
    Write(#$1b'[?3l')  { 80 Cols }
   else
    Write(#$1b'[?3h'); { 53 Cols }
end.
