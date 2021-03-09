program BEEPTEST;
{$I ARDUINO.INC}
{$I EGR.INC}
var
  i: Integer;
begin
  for i:=255 downto 1 do
    Beep(1, i, 800 div (i + 100));
  Delay2(500);
  Beep(2, 50, 100);
  Delay2(500);
  Beep(2, 50, 100);
  Delay2(500);
  Beep(2, 50, 100);
  Delay2(1500);
  Beep(3, 50, 100);
  Delay2(1500);
  Beep(5, 55, 100);
end.
