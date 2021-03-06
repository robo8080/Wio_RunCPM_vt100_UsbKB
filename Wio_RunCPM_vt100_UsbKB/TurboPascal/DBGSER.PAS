program DBGSER;
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

  { +----+--------------------------------+ }
  { |bit7| (Reserved)                     | }
  { +----+--------------------------------+ }
  { |bit6| (Reserved)                     | }
  { +----+--------------------------------+ }
  { |bit5| Output Invalid Escape Sequence | }
  { +----+--------------------------------+ }
  { |bit4| Output Raw                     | }
  { +----+--------------------------------+ }
  { |bit3| (Reserved)                     | }
  { +-------------------------------------+ }
  { |bit2| (Reserved)                     | }
  { +----+--------------------------------+ }
  { |bit1| (Reserved)                     | }
  { +----+--------------------------------+ }
  { |bit0| Allow Input                    | }
  { +----+--------------------------------+ }

  setDebugSerial(Mode);
end.

