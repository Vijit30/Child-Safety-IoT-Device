#include 
#include 

HardwareSerial SIM7670Serial(1); 
int sencev;
int sv = 15;

String phoneNumbers[3]; 
int numPhoneNumbers = 0; 

WebServer server(80); 

void sendATCommand(const char* cmd, const char* expectedResponse, unsigned long timeout) {
    SIM7670Serial.println(cmd);
    String response = "";
    unsigned long startTime = millis();
    bool responseOK = false;

    while (millis() - startTime < timeout) {
    while (SIM7670Serial.available() > 0) {
        char c = SIM7670Serial.read();
        response += c;
    }
    if (response.indexOf(expectedResponse) != -1) {
        responseOK = true;
        break;
    }
    }
    Serial.println(response);

    if (responseOK)
    Serial.println("Response OK");
    else
    Serial.println("Timeout without expected Response");
}

void sendSMS(String number, String message) {
    String cmd = "AT+CMGS=\"" + number + "\"\r\n";
    SIM7670Serial.print(cmd);
    delay(100);
    SIM7670Serial.println(message);
    delay(100);
    SIM7670Serial.write(0x1A); 
    delay(100);
}

void handleRoot() {
    String html = "
Enter up to 3 Phone Numbers
";
    html += "
";
    for (int i = 0; i < 3; i++) {
    html += "Phone " + String(i + 1) + ": 

";
    }
    html += "
";
    html += "";

    server.send(200, "text/html", html);
}

void handleSubmit() {
    numPhoneNumbers = 0; 
    
    for (int i = 0; i < 3; i++) {
    String paramName = "phone" + String(i + 1);
    String phoneNumber = server.arg(paramName);
    phoneNumber.trim();
    if (phoneNumber.length() == 10) {
        phoneNumbers[numPhoneNumbers] = phoneNumber;
        numPhoneNumbers++;
    }
    }

    String response = "
Phone numbers saved successfully:
";
    for (int i = 0; i < numPhoneNumbers; i++) {
    response += "
" + phoneNumbers[i] + "
";
    }
    response += "
";

    server.send(200, "text/html", response);
}

void setup() {
    Serial.begin(115200);
    SIM7670Serial.begin(115200, SERIAL_8N1, 14, 13);
    pinMode(sv, INPUT_PULLUP);

    WiFi.begin("Ganapa pustaka-", "narasimha1234"); //
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot); 
    server.on("/submit", handleSubmit); 

    server.begin(); 

    sendATCommand("AT+CGMM", "OK", 1000); 
    sendATCommand("AT+CMGF=1", "OK", 1000); 
}

void loop() {
    server.handleClient(); 

    sencev = digitalRead(sv);

    if (sencev == 0 && numPhoneNumbers > 0) {
    for (int i = 0; i < numPhoneNumbers; i++) {
        sendSMS(phoneNumbers[i], "I am in an emergency, please help. This is my location: https://maps.app.goo.gl/MSbsYCAYHxmgqz3g7");
        SIM7670Serial.println("ATD +" + phoneNumbers[i] + ";");
        delay(10000);
    }
    }

    if (SIM7670Serial.available()) {
    String response = SIM7670Serial.readString();
    Serial.println(response);
    if (response.indexOf("CONNECT") != -1) {
        Serial.println("Call Connected");
    }
    }
}
"""

async def handle_client(reader, writer):
    request_line = await reader.readline()
    print("Request:", request_line.decode())

    # Read the rest of the request headers
    while True:
        line = await reader.readline()
        if line == b"\r\n":
            break

    if request_line.startswith(b"GET /data"):
        sensor_data = read_sensors()
        avg_value = sum(sensor_data) / len(sensor_data)

        response_data = {
            "sensor1": sensor_data[0],
            "sensor2": sensor_data[1],
            "sensor3": sensor_data[2],
            "average": avg_value
        }

        response = json.dumps(response_data)
        writer.write(b'HTTP/1.1 200 OK\r\n')
        writer.write(b'Content-Type: application/json\r\n')
        writer.write(b'Connection: close\r\n')
        writer.write(b'\r\n')
        writer.write(response.encode())
    else:
        writer.write(b'HTTP/1.1 200 OK\r\n')
        writer.write(b'Content-Type: text/html\r\n')
        writer.write(b'Connection: close\r\n')
        writer.write(b'\r\n')
        writer.write(html.encode())

    await writer.drain()
    await writer.aclose()

async def main():
    server = await asyncio.start_server(handle_client, "0.0.0.0", 80)
    print("Server running on http://0.0.0.0:80")
    
    while True:
        await asyncio.sleep(3600)  # Keep the server running indefinitely

asyncio.run(main())
                
