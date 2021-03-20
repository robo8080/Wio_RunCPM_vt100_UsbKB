{* ARDUINO.INC *}

const
  INPUT        = 0; {ESP32: 1}
  OUTPUT       = 1; {ESP32: 2}
  INPUT_PULLUP = 2; {ESP32: 5}

  LOW  = 0;
  HIGH = 1;
  
  function systemFrequency:Integer; Forward;

  procedure Delay2(v: Integer);
  var
    i, Freq: Integer;
  begin
    { The formula is for Wio Terminal }
    Freq := systemFrequency div 10;
    if v < 1638 then
      i := v * Freq div 7
    else
      i := v div 10 * Freq div 7 * 10;
    Delay(i);
  end; {of procedure Delay2}

  (*** Host BDOS ***)

  function systemFrequency;
  var
    func: Byte;
  begin
    func := 1;
    systemFrequency := BDosHL(231, Addr(func)); { hostbdos }
  end; {of function systemFrequency}

  function getRotation: Integer;
  var
    func: Byte;
  begin
    func := 8;
    getRotation := BDosHL(231, Addr(func)); { hostbdos }
  end; {of function getRotation}

  procedure setRotation(m: Byte);
  var
    func: array [0..1] of Byte;
  begin
    func[0] := 9;
    func[1] := m;
    BDos(231, Addr(func)); { hostbdos }
  end; {of procedure setRotation}

  procedure setDebugSerial(m: Byte);
  var
    func: array [0..1] of Byte;
  begin
    func[0] := 10;
    func[1] := m;
    BDos(231, Addr(func)); { hostbdos }
  end; {of procedure setDebugSerial}

  (*** Board BDOS ***)

  function getJoy2Key: Byte;
  var
    func: Byte;
    HL: Integer;
  begin
    func := $02; { Func }
    getJoy2Key := BDosHL(232, Addr(func)); { boardbdos }
  end; {of function getDate}

  procedure setJoy2Key(Mode: Byte);
  var
    func: array [0..1] of Byte;
    HL: Integer;
  begin
    func[0] := $03; { Func }
    func[1] := Mode; { Param }
    BDos(232, Addr(func[0]));
  end; {of procedure setJoy2Key}

  procedure getDate(var y: Integer; var m, d: byte);
  var
    func: array [0..1] of Byte;
    HL: Integer;
  begin
    func[0] := $2A; { Func }
    func[1] := 0;   { Param }
    y  := BDosHL(232, Addr(func[0])); { boardbdos }
    func[1] := 1;   { Param }
    HL := BDosHL(232, Addr(func[0])); { boardbdos }
    m  := HL shr 8;
    d  := HL and $FF;
  end; {of procedure getDate}

  function getDayOfTheWeek: byte;
  var
    func: array [0..1] of Byte;
  begin
    func[0] := $2A; { Func }
    func[1] := 2;   { Param }
    getDayOfTheWeek := BDosHL(232, Addr(func[0]));
  end; {of function getDayOfTheWeek}

  procedure setDate(y: Integer; m, d: Byte);
  var
    func: array [0..4] of Byte;
  begin
    func[0] := $2B;       { Func }
    func[1] := 0;         { Param }
    func[2] := y shr 8;   { Data }
    func[3] := y and $FF; { Data }
    BDos(232, Addr(func[0]));
    func[1] := 1;         { Param }
    func[2] := m;         { Data }
    func[3] := d;         { Data }
    BDos(232, Addr(func[0]));
  end; {of procedure setDate}

  procedure getTime(var h, m, s: Byte);
  var
    func: array [0..1] of Byte;
    HL: Integer;
  begin
    func[0] := $2C; { Func }
    func[1] := 0;   { Param }
    HL := BDosHL(232, Addr(func[0]));
    h  := HL shr 8;
    m  := HL and $FF;
    func[1] := 1;   { Param }
    HL := BDosHL(232, Addr(func[0]));
    s  := HL shr 8;
  end; {of procedure getTime}

  procedure setTime(h, m, s: Byte);
  var
    func: array [0..4] of Byte;
  begin
    func[0] := $2D; { Func }
    func[1] := 0;   { Param }
    func[2] := h;   { Data }
    func[3] := m;   { Data }
    BDos(232, Addr(func[0]));
    func[1] := 1;   { Param }
    func[2] := s;   { Data }
    func[3] := 0;   { Data }
    BDos(232, Addr(func[0]));
  end; {of procedure setTime}

  (*** RunCPM BDOS ***)

  procedure pinMode(pin, mode: Byte);
  begin
    Bdos(220, (pin shl 8) + mode);
  end; {of procedure pinMode}

  function digitalRead(pin: Byte): Byte;
  begin
    digitalRead := BDos(221, (pin shl 8));
  end; {of function digitalRead}

  procedure digitalWrite(pin, value: Byte);
  begin
    BDos(222, (pin shl 8) + value);
  end; {of procedure digitalWrite}

  function analogRead(pin: Byte): Integer;
  begin
    analogRead := BDosHL(223, (pin shl 8));
  end; {of function analogRead}

  procedure analogWrite(pin, value: Byte);
  begin
    BDos(224, (pin shl 8) + value);
  end; {of procedure analogWrite}

  function hostOS: Integer;
  begin
    hostOS := BDosHL(250);
  end; {of function hostOS}

