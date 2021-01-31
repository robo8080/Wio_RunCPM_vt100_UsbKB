program ANKTEST;
const
  LINESTR = '+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+';
var
  ch: Char;
  c, r, n: Integer;

  function IntToHex(v: Byte): Char;
  begin
    if v < 10 then
      IntToHex := Chr(Ord('0') + v)
    else
      IntToHex := Chr(Ord('A') + v - 10);
  end; { IntToHex }

begin
  ClrScr;
  Bdos(230, $FF);
  Writeln;
  Writeln(LINESTR);
  write('| |');
  for c := 0 to 15 do
    write(IntToHex(c), '|');
  Writeln;
  Writeln(LINESTR);
  for r := 0 to 15 do
  begin
    write('|', IntToHex(r), '|');
    for c := 0 to 15 do
    begin
      n := r * 16 + c;
      if (n < $20) or (n = $7F) then
        n := $20;
      write(Chr(n), '|');
    end;
    Writeln;
  end;
  Writeln(LINESTR);
end.