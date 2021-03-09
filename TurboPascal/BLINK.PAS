program BLINK;
{$I ARDUINO.INC}
const
  LED_PIN = 0;
begin
  pinMode(LED_PIN, OUTPUT);
  repeat
    digitalWrite(LED_PIN, HIGH);
    Delay2(Conv(1000));
    digitalWrite(LED_PIN, LOW);
    Delay2(Conv(1000));
  until KeyPressed;
end.
