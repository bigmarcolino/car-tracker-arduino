String url_check_connection = "http://tracker.gobbi.info/checkConnection?login=arduino&password=jk+eUxwJ^%24aaDq2[";
String url_send_position = "http://tracker.gobbi.info/add?l=arduino&p=jk+eUxwJ^%24aaDq2[&lat=";

void setup() {
  //Init the driver pins for GSM function
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  
  //Output GSM Timing
  digitalWrite(5,HIGH);
  delay(1500);
  digitalWrite(5,LOW);
  
  Serial.begin(9600);
  
  // Use these commands instead of the hardware switch 'UART select' in order to enable each mode
  // If you want to use both GMS and GPS. enable the required one in your code and disable the other one for each access.
  digitalWrite(3,LOW);//enable GSM TX、RX
  digitalWrite(4,HIGH);//disable GPS TX、RX
  
  delay(20000); 
  
  config_GSM();
  //config_GPS();
  while(sendATcommand("AT+HTTPINIT", "OK", "ERROR", 20000).equals("ERROR"));
}

void loop() {
  /*long int lat = -22792567, lng = -43366982, segundo = 10;
  String dia = "24", mes = "04", ano = "2017", hora = "23", minuto = "50";

  if(method_get(url_check_connection).equals("OK")) {    
    while(true) {
      String url_position_parameters = url_send_position + String(lat, DEC) + "&lng=" + String(lng, DEC) + "&t=" + ano + "-" + mes + "-" + dia + "T" + hora + "%3A" + minuto + "%3A" + String(segundo, DEC) + ".100000";
      
      if(!method_get(url_position_parameters).equals("OK")) {
        break;
      }

      lat += -300;
      lng += 500;
      segundo++;

      delay(5000);
    }
  }

  delay(5000); */
}

void config_GSM() {
  sendATcommand("AT", "OK", " ", 20000);
  sendATcommand("AT+CREG?", "OK", "ERROR", 20000);
  sendATcommand("AT+SAPBR=3,1,\"APN\",\"java.claro.com.br\"", "OK", " ", 20000);
  sendATcommand("AT+SAPBR=3,1,\"USER\",\"claro\"", "OK", " ", 20000);
  sendATcommand("AT+SAPBR=3,1,\"PWD\",\"claro\"", "OK", " ", 20000);
  sendATcommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK", " ", 20000);
  sendATcommand("AT+SAPBR=2,1", "OK", " ", 20000);
  sendATcommand("AT+SAPBR=1,1", "OK", " ", 50000);
}

String method_get(String url) {  
  digitalWrite(3,LOW); //Enable GSM mode
  digitalWrite(4,HIGH); //Disable GPS mode

  sendATcommand("AT+HTTPPARA=\"URL\",\"" + url + "\"", "OK", "ERROR", 50000);
  sendATcommand("AT+HTTPPARA=\"CID\",1", "OK", "ERROR", 20000);
  sendATcommand("AT+HTTPACTION=0", "OK", "ERROR", 50000);
  return sendATcommand("AT+HTTPREAD", "OK", "ERROR", 50000);
}
/*
int8_t config_GPS() {
  unsigned long previous = millis();
  
  sendATcommand("AT+CGPSIPR=115200", "OK", 2000);
  sendATcommand("AT+CGPSPWR=1", "OK", 2000);
  sendATcommand("AT+CGPSRST=0", "OK", 2000); // AT+CGPSRST=1 is autonomy mode

  // waits for fix GPS
  while(( (sendATcommand("AT+CGPSSTATUS?", "2D Fix", 5000) || sendATcommand("AT+CGPSSTATUS?", "3D Fix", 5000)) == 0 ) && ((millis() - previous) < 90000));

  if ((millis() - previous) < 90000)
    return 1;
  else
    return 0;    
}

int8_t get_GPS() { 
  int8_t counter, answer;
  long previous;

  // First get the NMEA string
  // Clean the input buffer
  while(Serial.available() > 0) 
    Serial.read();
     
  sendATcommand("AT+CGPSINF=0", "AT+CGPSINF=0\r\n\r\n", 2000); // request Basic string

  counter = 0;
  answer = 0;
  memset(frame, '\0', 100); // Initialize the string
  previous = millis();
  
  do { // this loop waits for the NMEA string
    if(Serial.available() != 0) {    
      frame[counter] = Serial.read();
      counter++;
      
      if (strstr(frame, "OK") != NULL) // check if the desired answer is in the response of the module
        answer = 1;
    }
  }
  while((answer == 0) && ((millis() - previous) < 2000)); // Waits for the asnwer with time out

  frame[counter-3] = '\0'; 
  
  // Parses the string 
  strtok(frame, ",");
  strcpy(longitude, strtok(NULL, ",")); // Gets longitude
  strcpy(latitude, strtok(NULL, ",")); // Gets latitude
  strcpy(altitude, strtok(NULL, ".")); // Gets altitude 
  strtok(NULL, ",");    
  strcpy(date, strtok(NULL, ".")); // Gets date
  strtok(NULL, ",");
  strtok(NULL, ",");  
  strcpy(satellites, strtok(NULL, ",")); // Gets satellites
  strcpy(speedOTG, strtok(NULL, ",")); // Gets speed over ground. Unit is knots.
  strcpy(course, strtok(NULL, "\r")); // Gets course

  convert2Degrees(latitude);
  convert2Degrees(longitude);
  
  return answer;
}
*/
String sendATcommand(String ATcommand, String expected_answer1, String expected_answer2, unsigned int timeout) {
  String response, response1, response2;
  unsigned long previous;
  int indexResponse;
  
  Serial.println(ATcommand);
  previous = millis();

  do {
    if(Serial.available() != 0) {
      if(ATcommand.equals("AT+HTTPREAD")) {
        response = Serial.readString();
        indexResponse = response.lastIndexOf("+HTTPREAD:");
        
        if(indexResponse > -1) {
          response = response.substring(indexResponse);
          Serial.println(response);
          return response.substring(13, response.indexOf("\r", 13));
        }
      }
      else {
        response = Serial.readString();
        response1 = response.substring(response.length() - expected_answer1.length() - 2, response.length() - 2);
        response2 = response.substring(response.length() - expected_answer2.length() - 2, response.length() - 2);

        if(response1.equals(expected_answer1)) { 
          if(ATcommand.indexOf("AT+HTTPACTION=0") > -1) {
            while(Serial.readString().indexOf("+HTTPACTION:") == -1 && (millis() - previous) < timeout);
          }

          Serial.println(response1);
          return response1;
        }
        else if(response2.equals(expected_answer2)) {          
          Serial.println(response2);
          return response2;
        } 
      }
    }
  }
  while((millis() - previous) < timeout);

  Serial.println("Timeout");
  return "";
}
