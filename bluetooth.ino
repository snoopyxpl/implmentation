void setup() {
Serial1.begin(9600);
}
void loop() {
int valeur = analogRead(29);
float tension = valeur * (3.3 / 4092.0);
Serial1.println(tension);
delay(1000);
}