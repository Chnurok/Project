const int cap = 34;
const int en_cap = 5;
const int res = 35;
const int en_res = 2;

float temp=0;
float res_datchik=0;

void setup() {
  
Serial.begin(115200);

void loop() {
  digitalWrite(en_cap, HIGH);
  delay(500);
  temp = analogRead(cap);
  Serial.print("Емкостной датчик");
  Serial.println(temp);
  digitalWrite(en_cap, LOW);
  delay(500);


  
  digitalWrite(en_res, HIGH);
  delay(500);
  res_datchik = analogRead(res);
  Serial.print("Резистивный датчик:");
  Serial.println(res_datchik);
  digitalWrite(en_res, LOW);
  delay(500);
}
