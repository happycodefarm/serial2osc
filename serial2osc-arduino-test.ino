void setup() {
 Serial.begin(9600); 
}


void loop() {
  Serial.println(random(0,1000));
  delay(200);
  
}
