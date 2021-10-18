
// device default config

// ThingSpeak information.
// To update more fields, increase this number and add a field label below.
#define NUM_FIELDS 2
// ThingSpeak field for soil moisture measurement.
#define SOIL_MOISTURE_FIELD 1
// ThingSpeak field for elapsed time from startup.                      
#define ELAPSED_TIME_FIELD 2                       
#define THING_SPEAK_ADDRESS "api.thingspeak.com"
// Change this to the write API key for your channel.
String writeAPIKey= "XXXXXXXXXXXXXXXX";

WiFiClient client;

void setup()
{ 
  // pin mode config
  // wifi config 
}

void loop()
{ 
  // wifi connection verify
  // Write to successive fields in your channel by filling fieldData with up to 8 values.
  String fieldData[ NUM_FIELDS ];  
  // You can write to multiple fields by storing data in the fieldData[] array, and changing numFields.        
  // Write the moisture data to field 1.
  fieldData[ SOIL_MOISTURE_FIELD ] = String( readSoil( numMeasure ) ); 
  // Write the elapsed time from startup to Field 2.
  fieldData[ ELAPSED_TIME_FIELD ] = String( millis() ); 
  HTTPPost( NUM_FIELDS , fieldData );
  delay( 1000 );
  Serial.print( "Goodnight for "+String( SLEEP_TIME_SECONDS ) + " Seconds" );
  // delay time
}

int HTTPPost( int numFields , String fieldData[] ){
  if (client.connect( THING_SPEAK_ADDRESS , 80 )){
      // Build the postData string.  
      // If you have multiple fields, make sure the sting does not exceed 1440 characters.
      String postData= "api_key=" + writeAPIKey ;
      for ( int fieldNumber = 1; fieldNumber < numFields+1; fieldNumber++ ){
          String fieldName = "field" + String( fieldNumber );
          postData += "&" + fieldName + "=" + fieldData[ fieldNumber ];
        }
      client.println( "POST /update HTTP/1.1" );
      client.println( "Host: api.thingspeak.com" );
      client.println( "Connection: close" );
      client.println( "Content-Type: application/x-www-form-urlencoded" );
      client.println( "Content-Length: " + String( postData.length() ) );
      client.println();
      client.println( postData );
      String answer = getResponse();
  }
}

String getResponse(){
  String response;
  long startTime = millis();
  delay( 200 );
  while ( client.available() < 1 && (( millis() - startTime ) < TIMEOUT ) ){
    delay( 5 );
  }
  if( client.available() > 0 ){ // Get response from server.
    char charIn;
    do {
      // Read a char from the buffer.
      charIn = client.read();
      // Append the char to the string response.
      response += charIn;     
    } while ( client.available() > 0 );
  }
  client.stop(); 
  return response;
}