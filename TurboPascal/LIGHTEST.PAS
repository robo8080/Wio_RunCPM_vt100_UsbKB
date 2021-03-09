program LIGHTEST;
{$I ARDUINO.INC}
const
  WIO_LIGHT = 27;
var
  light: Integer;  
begin
  pinMode(WIO_LIGHT, INPUT);
  repeat
    light := analogRead(WIO_LIGHT);
    Writeln('Light value: ', light);
    Delay2(200);
  until KeyPressed;
end.
