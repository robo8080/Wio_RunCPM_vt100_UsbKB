program J2KTEST;
{$I ARDUINO.INC}
const
  WIO_KEY_A     = 28;
  WIO_KEY_B     = 29;
  WIO_KEY_C     = 30;
  WIO_5S_UP     = 31;
  WIO_5S_LEFT   = 32;
  WIO_5S_RIGHT  = 33;
  WIO_5S_DOWN   = 34;
  WIO_5S_PRESS  = 35;
var
  b: Byte;
  procedure Disp; 
  begin
    if (b and $01) = $01 then
      Writeln(' - User Defined Button A');
    if (b and $02) = $02 then
      Writeln(' - User Defined Button B');
    if (b and $04) = $04 then
      Writeln(' - User Defined Button C');
    if (b and $08) = $08 then
      Writeln(' - 5-Way Switch Up');
    if (b and $10) = $10 then
      Writeln(' - 5-Way Switch Down');
    if (b and $20) = $20 then
      Writeln(' - 5-Way Switch Right');
    if (b and $40) = $40 then
      Writeln(' - 5-Way Switch Left');
    if (b and $80) = $80 then
      Writeln(' - 5-Way Switch Press');
  end;
begin
(******************************
  Bit0: User Defined Button A
  Bit1: User Defined Button B
  Bit2: User Defined Button C
  Bit3: 5-Way Switch Up
  Bit4: 5-Way Switch Down
  Bit5: 5-Way Switch Right
  Bit6: 5-Way Switch Left
  Bit7: 5-Way Switch Press
 ******************************)
  b := getJoy2Key;
  Writeln('Joy2Key (Read):'); Disp; Writeln;
  
  b := $78;
  setJoy2Key(b); 
  Writeln('Joy2Key (Write):'); Disp;
end.
