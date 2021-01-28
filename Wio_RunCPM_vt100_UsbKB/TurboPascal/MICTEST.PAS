program MICTEST;
{$I ARDUINO.INC}
const
  WIO_MIC = 39;
var
  v: Integer;  
begin
  pinMode(WIO_MIC, INPUT);
  repeat
    v := analogRead(WIO_MIC);
    Writeln(v);
    Delay2(200);
  until KeyPressed;
end.

