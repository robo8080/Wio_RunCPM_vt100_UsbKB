program FONTTEST;
{$I EGR.INC}
var
  c: Char;
begin
  { for 6x8 font }
  Clear;

  GenerateChar('A', $00, $7C, $7C, $7C, $7C, $7C, $7C, $7C);
  GenerateChar('B', $00, $FC, $80, $80, $80, $80, $80, $80);
  GenerateChar('C', $00, $FC, $00, $1C, $1C, $1C, $1C, $1C);
  GenerateChar('D', $00, $E0, $70, $78, $78, $78, $78, $78);
  GenerateChar('E', $7C, $7C, $7C, $60, $60, $64, $68, $68);
  GenerateChar('F', $80, $FC, $FC, $00, $00, $CC, $28, $08);
  GenerateChar('G', $00, $FC, $FC, $00, $00, $90, $58, $54);
  GenerateChar('H', $78, $F8, $F8, $18, $18, $58, $D8, $58);
  GenerateChar('I', $68, $68, $68, $64, $60, $60, $7C, $00);
  GenerateChar('J', $0C, $08, $28, $C0, $00, $00, $FC, $00);
  GenerateChar('K', $90, $10, $10, $10, $00, $00, $FC, $00);
  GenerateChar('L', $58, $58, $58, $58, $18, $18, $F8, $00);

  Writeln;
  Writeln(' ABCD');
  Writeln(' EFGH');
  Writeln(' IJKL');
  Writeln;
  Readln;

  for c := 'A' to 'L' do
    RestoreChar(c);
end.