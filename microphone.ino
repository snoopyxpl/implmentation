int analogPin = PD_0;
int val = 0;

void setup() {
//put your setup code here
Serial.begin(9600);
}

void loop() {
//put your main code here, tu run repeatedly
val = analogRead(analogPin);
Serial.println(val);
}