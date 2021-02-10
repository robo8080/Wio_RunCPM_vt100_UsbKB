program ANKTEST;
const
  LINESTR = '+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+';
  TBL: string[16] = '0123456789ABCDEF';
var
  c, r, n: Integer;
begin
  ClrScr;
  Writeln(LINESTR);
  write('| |');
  for c := 0 to 15 do
    write(TBL[c + 1], '|');
  Writeln;
  Writeln(LINESTR);
  for r := 0 to 15 do
  begin
    write('|', TBL[r + 1], '|');
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
