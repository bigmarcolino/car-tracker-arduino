String lat, lng, utc;

void setup() {  
  Serial.begin(9600);
    
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  
  digitalWrite(5, HIGH);
  delay(1500);
  digitalWrite(5, LOW);
  
  digitalWrite(3, LOW);
  digitalWrite(4, HIGH);
  
  delay(20000); 
  
  config_GPS();
  config_GSM();

  while( send_AT_command("AT+HTTPINIT", "OK", "ERROR", 20000).equals("ERROR") );
}

void loop() {
  if( method_get("http://tracker.gobbi.info/checkConnection?login=arduino&password=jk+eUxwJ^%24aaDq2[").equals("OK") ) {        
    while(true) {
      get_GPS();
    
      if( method_get("http://tracker.gobbi.info/add?l=arduino&p=jk+eUxwJ^%24aaDq2[&lat=" + lat + "&lng=" + lng + "&t=" + utc).equals("ERROR") )
        break;
    }
  }
}

void config_GSM() {
  send_AT_command("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK", "ERROR", 20000);
  send_AT_command("AT+SAPBR=3,1,\"APN\",\"claro.com.br\"", "OK", "ERROR", 20000);
  send_AT_command("AT+SAPBR=3,1,\"USER\",\"claro\"", "OK", "ERROR", 20000);
  send_AT_command("AT+SAPBR=3,1,\"PWD\",\"claro\"", "OK", "ERROR", 20000);
  while( !send_AT_command("AT+SAPBR=1,1", "OK", "ERROR", 50000).equals("OK") );
}

String method_get(String url) {          
  send_AT_command("AT+HTTPPARA=\"URL\",\"" + url + "\"", "OK", "ERROR", 50000);
  send_AT_command("AT+HTTPPARA=\"CID\",1", "OK", "ERROR", 20000);
  send_AT_command("AT+HTTPACTION=0", "OK", "ERROR", 70000);
  return send_AT_command("AT+HTTPREAD=0,5", "OK", "ERROR", 100000);
}

void config_GPS() {
  send_AT_command("AT+CGPSIPR=9600", "OK", "ERROR", 10000);
  delay(1000);
  send_AT_command("AT+CGPSPWR=1", "OK", "ERROR", 10000);
  delay(1000);
  send_AT_command("AT+CGPSRST=0", "OK", "ERROR", 10000);
  delay(120000);
}

void get_GPS() {
  String gps_info = send_AT_command("AT+CGPSINF=0", "OK", "ERROR", 5000);
  
  int index_comma_one = gps_info.indexOf(",");
  int index_comma_two = gps_info.indexOf(",", index_comma_one + 1);
  int index_comma_tree = gps_info.indexOf(",", index_comma_two + 1);
  int index_comma_four = gps_info.indexOf(",", index_comma_tree + 1);
  int index_comma_five = gps_info.indexOf(",", index_comma_four + 1);
  
  lng = gps_info.substring(index_comma_one + 1, index_comma_two);
  lat = gps_info.substring(index_comma_two + 1, index_comma_tree);
  utc = gps_info.substring(index_comma_four + 1, index_comma_five);
  utc = utc.substring(0, utc.indexOf("."));
  //Serial.println("Lng: " + lng + " Lat: " + lat + " UTC: " + utc + "\r");
}

String send_AT_command(String AT_command, String expected_answer1, String expected_answer2, unsigned long timeout) {
  String response, response1, response2;
  int index_response;
  unsigned long previous = millis();

  Serial.println(AT_command);

  do {
    if(Serial.available() > 0) {
      if(AT_command.equals("AT+HTTPREAD=0,5")) {
        response = Serial.readString();
        index_response = response.lastIndexOf("+HTTPREAD:");
        
        if(index_response > -1) {
          response = response.substring(index_response);
          Serial.println(response);
          return response.substring(13, response.indexOf("\r", 13));
        }
      }
      else if(AT_command.equals("AT+CGPSINF=0")) {
        response = Serial.readString();        
        index_response = response.lastIndexOf("\r\n0,");

        if(index_response > -1) {
          response = response.substring(index_response + 2, response.length() - 6);
          Serial.println(response);          
          return response; 
        }
      }
      else {
        response = Serial.readString();        
        response1 = response.substring(response.length() - expected_answer1.length() - 2, response.length() - 2);
        response2 = response.substring(response.length() - expected_answer2.length() - 2, response.length() - 2);

        if(response1.equals(expected_answer1)) { 
          if(AT_command.equals("AT+HTTPACTION=0"))
            while(Serial.readString().lastIndexOf("+HTTPACTION:") == -1 && (millis() - previous) < timeout);

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
