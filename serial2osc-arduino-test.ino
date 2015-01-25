void setup() {
 Serial.begin(9600); 
}


void loop() {
  Serial.print(random(0,1000));
  Serial.print("\n");
  delay(200);
  
}
