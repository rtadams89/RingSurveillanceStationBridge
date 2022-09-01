#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <Arduino_JSON.h>
#include <esp_task_wdt.h>
#include <arduino_secrets.h>


const char* ssid = SECRET_SSID
const char* password = SECRET_PASSWORD
const String ssServer = SECRET_SSSERVER
const String ssuser = SECRET_SSUSER
const String ssuserpass = SECRET_SSUSERPASS

#define WDT_TIMEOUT 90

unsigned long heartBeatPeriod = 60000;

const int disarmedPin = 21; //Pin pulled low to indicate Ring is disarmed
const int homePin = 19; //Pin pulled low to indicate is armed
const int awayPin = 18; //Pin pulled low to indicate is armed

// This is the root Certificate Authority certificate assocaited with the Synology interface
const char* rootCACertificate = \
                                "-----BEGIN CERTIFICATE-----\n" \
                                "MIIG1TCCBL2gAwIBAgIQbFWr29AHksedBwzYEZ7WvzANBgkqhkiG9w0BAQwFADCB\n" \
                                "iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \
                                "cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \
                                "BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMjAw\n" \
                                "MTMwMDAwMDAwWhcNMzAwMTI5MjM1OTU5WjBLMQswCQYDVQQGEwJBVDEQMA4GA1UE\n" \
                                "ChMHWmVyb1NTTDEqMCgGA1UEAxMhWmVyb1NTTCBSU0EgRG9tYWluIFNlY3VyZSBT\n" \
                                "aXRlIENBMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAhmlzfqO1Mdgj\n" \
                                "4W3dpBPTVBX1AuvcAyG1fl0dUnw/MeueCWzRWTheZ35LVo91kLI3DDVaZKW+TBAs\n" \
                                "JBjEbYmMwcWSTWYCg5334SF0+ctDAsFxsX+rTDh9kSrG/4mp6OShubLaEIUJiZo4\n" \
                                "t873TuSd0Wj5DWt3DtpAG8T35l/v+xrN8ub8PSSoX5Vkgw+jWf4KQtNvUFLDq8mF\n" \
                                "WhUnPL6jHAADXpvs4lTNYwOtx9yQtbpxwSt7QJY1+ICrmRJB6BuKRt/jfDJF9Jsc\n" \
                                "RQVlHIxQdKAJl7oaVnXgDkqtk2qddd3kCDXd74gv813G91z7CjsGyJ93oJIlNS3U\n" \
                                "gFbD6V54JMgZ3rSmotYbz98oZxX7MKbtCm1aJ/q+hTv2YK1yMxrnfcieKmOYBbFD\n" \
                                "hnW5O6RMA703dBK92j6XRN2EttLkQuujZgy+jXRKtaWMIlkNkWJmOiHmErQngHvt\n" \
                                "iNkIcjJumq1ddFX4iaTI40a6zgvIBtxFeDs2RfcaH73er7ctNUUqgQT5rFgJhMmF\n" \
                                "x76rQgB5OZUkodb5k2ex7P+Gu4J86bS15094UuYcV09hVeknmTh5Ex9CBKipLS2W\n" \
                                "2wKBakf+aVYnNCU6S0nASqt2xrZpGC1v7v6DhuepyyJtn3qSV2PoBiU5Sql+aARp\n" \
                                "wUibQMGm44gjyNDqDlVp+ShLQlUH9x8CAwEAAaOCAXUwggFxMB8GA1UdIwQYMBaA\n" \
                                "FFN5v1qqK0rPVIDh2JvAnfKyA2bLMB0GA1UdDgQWBBTI2XhootkZaNU9ct5fCj7c\n" \
                                "tYaGpjAOBgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHSUE\n" \
                                "FjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwIgYDVR0gBBswGTANBgsrBgEEAbIxAQIC\n" \
                                "TjAIBgZngQwBAgEwUAYDVR0fBEkwRzBFoEOgQYY/aHR0cDovL2NybC51c2VydHJ1\n" \
                                "c3QuY29tL1VTRVJUcnVzdFJTQUNlcnRpZmljYXRpb25BdXRob3JpdHkuY3JsMHYG\n" \
                                "CCsGAQUFBwEBBGowaDA/BggrBgEFBQcwAoYzaHR0cDovL2NydC51c2VydHJ1c3Qu\n" \
                                "Y29tL1VTRVJUcnVzdFJTQUFkZFRydXN0Q0EuY3J0MCUGCCsGAQUFBzABhhlodHRw\n" \
                                "Oi8vb2NzcC51c2VydHJ1c3QuY29tMA0GCSqGSIb3DQEBDAUAA4ICAQAVDwoIzQDV\n" \
                                "ercT0eYqZjBNJ8VNWwVFlQOtZERqn5iWnEVaLZZdzxlbvz2Fx0ExUNuUEgYkIVM4\n" \
                                "YocKkCQ7hO5noicoq/DrEYH5IuNcuW1I8JJZ9DLuB1fYvIHlZ2JG46iNbVKA3ygA\n" \
                                "Ez86RvDQlt2C494qqPVItRjrz9YlJEGT0DrttyApq0YLFDzf+Z1pkMhh7c+7fXeJ\n" \
                                "qmIhfJpduKc8HEQkYQQShen426S3H0JrIAbKcBCiyYFuOhfyvuwVCFDfFvrjADjd\n" \
                                "4jX1uQXd161IyFRbm89s2Oj5oU1wDYz5sx+hoCuh6lSs+/uPuWomIq3y1GDFNafW\n" \
                                "+LsHBU16lQo5Q2yh25laQsKRgyPmMpHJ98edm6y2sHUabASmRHxvGiuwwE25aDU0\n" \
                                "2SAeepyImJ2CzB80YG7WxlynHqNhpE7xfC7PzQlLgmfEHdU+tHFeQazRQnrFkW2W\n" \
                                "kqRGIq7cKRnyypvjPMkjeiV9lRdAM9fSJvsB3svUuu1coIG1xxI1yegoGM4r5QP4\n" \
                                "RGIVvYaiI76C0djoSbQ/dkIUUXQuB8AL5jyH34g3BZaaXyvpmnV4ilppMXVAnAYG\n" \
                                "ON51WhJ6W0xNdNJwzYASZYH+tmCWI+N60Gv2NNMGHwMZ7e9bXgzUCZH5FaBFDGR5\n" \
                                "S9VWqHB73Q+OyIVvIbKYcSc2w/aSuFKGSA==\n" \
                                "-----END CERTIFICATE-----\n";

String sstoken = "xxx";
unsigned long last_report = (heartBeatPeriod + 1);

void setClock() {
  configTime(0, 0, "pool.ntp.org");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

bool ss_home_mode_is_active()
{


  JSONVar JSONobject = JSON.parse(getHTTPPayload("https://" + ssServer + "/webapi/entry.cgi?api=SYNO.SurveillanceStation.HomeMode&version=1&method=GetInfo&_sid=" + sstoken));

  while ((boolean)JSONobject["success"] == false) {
    get_ss_token();
    JSONobject = JSON.parse(getHTTPPayload("https://" + ssServer + "/webapi/entry.cgi?api=SYNO.SurveillanceStation.HomeMode&version=1&method=GetInfo&_sid=" + sstoken));
  }


  if ((boolean)JSONobject["data"]["on"] == true) {
    Serial.println(F("Home mode currently on"));
    delay(5000);
    return true;
  }
  else {
    Serial.println(F("Home mode currently off"));
    delay(5000);
    return false;

  }

}


//Get SS auth token
void get_ss_token () {
  JSONVar JSONobject = JSON.parse(getHTTPPayload("https://" + ssServer + "/webapi/auth.cgi?api=SYNO.API.Auth&method=login&version=3&account=" + ssuser + "&passwd=" + ssuserpass + "&session=SurveillanceStation&format=sid"));

  if ((boolean)JSONobject["success"] == true) {
    sstoken = (const char*) JSONobject["data"]["sid"];
  }
  else {
    Serial.println(F("SS authentication error. Sleeping for 1 minute..."));
    delay(60000);
  }

}


//Check for valid SS token
bool ss_token_is_valid () {
  JSONVar JSONobject = JSON.parse(getHTTPPayload("https://" + ssServer + "/webapi/entry.cgi?api=SYNO.SurveillanceStation.HomeMode&version=1&method=GetInfo&_sid=" + sstoken));

  if ((boolean)JSONobject["success"] == true) {
    return true;
  }
  else {
    return false;
  }

}



String getHTTPPayload (String url) {

  String payload = "";

  WiFiClientSecure *client = new WiFiClientSecure;
  if (client) {
    client -> setCACert(rootCACertificate);

    {
      HTTPClient https;

      if (https.begin(*client, url)) {
        int httpCode = https.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {

          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            payload = https.getString();
          }
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          payload = F("Error: Get failes");
        }

        https.end();
      } else {
        Serial.println(F("[HTTPS] Error Unable to connect\n"));
        payload = F("Error: Unable to connect");
      }

    }

    delete client;
  } else {
    Serial.println(F("Error: Unable to create client"));
    payload = F("Error: Unable to create client");
  }
  return payload;
}

void setup()
{
  delay(2000);

  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  pinMode(disarmedPin, INPUT_PULLUP);
  pinMode(homePin, INPUT_PULLUP);
  pinMode(awayPin, INPUT_PULLUP);

  Serial.begin(115200);

  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  Serial.println(F("Setting clock"));
  setClock();

  Serial.println(F("RingSynologyBridge booted"));

}

void loop() {

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    delay(1000);
    WiFi.reconnect();
    delay(1000);
  }


  if (millis() - last_report > heartBeatPeriod) {
    Serial.println(F("RingSynologyBridge Heartbeat"));
    Serial.print(F("MinFreeHeap="));
    Serial.println(ESP.getMinFreeHeap());
    last_report = millis();
  }


  if (digitalRead(disarmedPin) == LOW && ss_home_mode_is_active() == false)
  {
    Serial.println(F("Turning home mode on..."));
    JSONVar JSONobject = JSON.parse(getHTTPPayload("https://" + ssServer + "/webapi/entry.cgi?api=SYNO.SurveillanceStation.HomeMode&version=1&method=Switch&on=true&_sid=" + sstoken));

    if ((boolean)JSONobject["success"] == true) {
      Serial.println(F("Home mode turned on succesfully"));
      delay(7500);
      Serial.println(F("Monitoring for changes again"));
    }
    else {
      Serial.println(F("Error turning home mode on. Sleeping for 1 minute..."));
      delay(60000);
    }

  }


  if ((digitalRead(homePin) == LOW || digitalRead(awayPin) == LOW) && ss_home_mode_is_active() == true)
  {
    Serial.println(F("Turning home mode off..."));
    JSONVar JSONobject = JSON.parse(getHTTPPayload("https://" + ssServer + "/webapi/entry.cgi?api=SYNO.SurveillanceStation.HomeMode&version=1&method=Switch&on=false&_sid=" + sstoken));

    if ((boolean)JSONobject["success"] == true) {
      Serial.println(F("Home mode turned off succesfully"));
      delay(7500);
      Serial.println(F("Monitoring for changes again"));
    }
    else {
      Serial.println(F("Error turning home mode off. Sleeping for 1 minute..."));
      delay(60000);
    }
  }
  esp_task_wdt_reset();
}