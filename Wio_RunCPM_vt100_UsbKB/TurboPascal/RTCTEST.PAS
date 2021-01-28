program RTCTEST;
{$I ARDUINO.INC}
var
  Year: Integer;
  Month, Day, Hour, Minute, Second: Byte;
begin
  getDate(Year, Month, Day);
  getTime(Hour, Minute, Second);
  Write('Year (' , Year , '): '); Readln(Year);
  Write('Month (', Month, '): '); Readln(Month);
  Write('Day ('  , Day  , '): '); Readln(Day);
  Writeln;
  Write('Hour ('  , Hour  , '): '); Readln(Hour);
  Write('Minute (', Minute, '): '); Readln(Minute);
  Write('Second (', Second, '): '); Readln(Second);
  Writeln;
  setDate(Year, Month, Day);
  setTime(Hour, Minute, Second);
  repeat
    getDate(Year, Month, Day);
    getTime(Hour, Minute, Second);
    Writeln(Year, '/', Month, '/', Day, ' ', Hour, ':', Minute, ':', Second);
    Delay2(10000);
  until KeyPressed;
end.
