/*
 * Team 7 - Obstacle avoidance robot
 */
 
 // Inclusions
#include <MotorDriver.h>
#include <Servo.h>
#include <NewPing.h>
#include <WiFiNINA.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoHttpClient.h>
#include <ThingSpeak.h>

 // WiFi Connections
const char* ssid="MITansh's Galaxy S21 5G";
const char* password="MK010803";
char thingSpeakAddress[] = "api.thingspeak.com";

// for HTTP
char ip[] = "192.168.56.2";
int port = 8080;

// om2m definitions
String cse_ip = "esw-onem2m.iiit.ac.in";  
int cse_port = 443;
String cse_server = "http://" + cse_ip + ":" + cse_port + "/~/in-cse/in-name/";
String ae = "ESW PeePs";
WiFiClient wifi;   
WiFiServer server(80);  
HttpClient Hclient = HttpClient(wifi, cse_ip, cse_port);

// Thingspeak definitions - redundant
//unsigned long thingspeak_channel = 4;
//const char* thingspeak_write_api = "1YXEOPE2Q5TQRARZ";;
//unsigned long channel_num=1918870;
//const int updateThingSpeakInterval = 15 * 1000; // 15 seconds

// for motors
MotorDriver m;

// WiFiCLient declaration
WiFiClient client; 

// Ultrasonic sensors
#define trigger_pin A0
#define echo_pin A1
const int maximum_distance = 400; // an ultrasonic sensor can read till 400 cm
int global_distance = 0;
NewPing ultrasonic(trigger_pin, echo_pin, maximum_distance);

//for om2m
int checker_status=0;
String str = "";
int delay_measure;

// for servo 
int initial_angle = 0;
Servo head_servo;

// for trajectory
double x = 0.0;
double y = 0.0;
double pi = 3.1416;
double trajectory_angle = 0;
double one_second_angle = 30.0/700.0;
double trajectory_factor = 1e-5;

// for linear motion
const int maximum_speed = 200;
const int speed_increment = 3;
bool moving = false;
int current_speed = 0;
long last_time = 0;
long time_period = 0;
const int linear_threshold_distance = 25;
const int angular_threshold_distance = 20;

// for probing
long last_probe = 0;
long probe_time = 0;

void CreateCI(String val)
{
  // String v = "20 45 20221127T211755";
  String body = "{\"m2m:cin\": {\"lbl\": [ \"Team-6\" ],\"con\": \"" + String(val) + "\"}}";

  Serial.println(val);

  Hclient.beginRequest();
  Hclient.post("/~/in-cse/in-name/Team-6/Node-1/Data/");
  Hclient.sendHeader("Content-Length", body.length());
  Hclient.sendHeader("X-M2M-Origin", "MPHAtt:nJpU3U");
  Hclient.sendHeader("Content-Type", "application/json;ty=4");
  Hclient.sendHeader("Connection", "keep-alive");
  Hclient.beginBody();
  Hclient.print(body);
  Hclient.endRequest();
  int status = Hclient.responseStatusCode();
  Serial.println(status);
  String responseBody = Hclient.responseBody();
  if(checker_status==0 && status==201)
  { 
    checker_status = 1;
    for(int i=0;i<responseBody.length()-1;i++){
      bool chk=false;
        if(responseBody[i]=='c' && responseBody[i+1]=='t'){
          chk = true;
          for(int j=i+7;j<i+22;j++){
              str += responseBody[j];
              // Serial.print(responseBody[j]);
          }
          delay_measure = millis(); 
          if(chk){
            break;
          }         
        } 
    }
  }
  else if(status==201)
    return;
  Serial.println(str);
  Serial.println(responseBody);
}

// Function to send data *********************
void sendData(){
String temp = "";

  long val_tm = millis(),num=0;

  val_tm /= 1000;


  String temp_time = "" , val = "";

  for(int i=9;i<str.length();i++){
    temp_time += str[i];
  }

  for(int i=0;i<=8;i++){
    val += str[i];
  }

  // Serial.println(temp_time);

  for(int i=5;i>=0;i--){
    long vval = (temp_time[i]-'0');
    delay(5);
    for(int j=0;j<5-i;j++)
      vval*=10;
    num += vval;
  }

  // Serial.println(val_tm);

  long time_ap=0;

  if(val_tm > 0){
    long temp_s= (num % 100) + val_tm;

    val_tm /= 60;

    if(temp_s>=60)
      val_tm++;
    
    temp_s%=60;

    time_ap+=temp_s;
  }
  else{
    time_ap+=(num%100);
  }

  if(val_tm>0){
    long temp_s= (num % 10000) + val_tm;


    val_tm /= 60;

    if(temp_s>=60)
      val_tm++;
    
    temp_s%=60;

    time_ap+=(temp_s*100);
  }
  else{
    long va = (num%10000);

    va/=100;
    va*=100;

    // Serial.println(va);
    
    time_ap+=va; 
  }

  if(val_tm>0){
    long temp_s= (num % 1000000) + val_tm;

    val_tm /= 24;

    if(temp_s>=24)
      val_tm++;

    temp_s %= 24;
    
    time_ap+=(temp_s*10000);
  }
  else{
    long va = (num%1000000);
    va/=10000;
    va*=10000;
    Serial.println(va);
    time_ap+=va; 
  }

  val += String(time_ap);

  String vsend = "";

  vsend += String(x);
  vsend += ' ';
  vsend += String(y);
  vsend += ' ';
  vsend += String(val);

  CreateCI(vsend);
}

// Code for WiFi *****************************
void wifiConnect(){
  int wifi_status = WL_IDLE_STATUS;

  while(wifi_status != WL_CONNECTED){
    Serial.print("... ");
    wifi_status = WiFi.begin(ssid, password);
    delay(500);
  }

  Serial.println("Wifi Connected!");
}

// To update global time ********************
void updateTime(){
  long cur_time = millis();
  time_period = (cur_time - last_time + 1000000)%1000000; // hoping this will account for integer overflow
  last_time = cur_time;
}

// Function to get distance *****************
void getDistance(int* dist){
  *dist = ultrasonic.ping_cm();
  if (*dist == 0)
    *dist = 400;
}

// To stop bot movement *********************
void stopBot(){
  m.motor(1, FORWARD, 0);    
  m.motor(2, FORWARD, 0);
  m.motor(3, FORWARD, 0); 
  m.motor(4, FORWARD, 0); 

  current_speed = 0;
  moving = false;
}

// function to update trajectory ************
void updateTrajectory(){
  updateTime();
    
  x = x + time_period * current_speed * sin(trajectory_angle * pi/180) * trajectory_factor;
  y = y + time_period * current_speed * cos(trajectory_angle * pi/180) * trajectory_factor;
}

// To make the bot move forward *************
void moveBot(){
  moving = true;
  
  for (current_speed = 0; current_speed <= maximum_speed; current_speed+=speed_increment){
    m.motor(1, FORWARD, current_speed);    
    m.motor(2, FORWARD, current_speed);
    m.motor(3, FORWARD, current_speed); 
    m.motor(4, FORWARD, current_speed); 

    updateTime();
      
    x = x + time_period * current_speed * sin(trajectory_angle * pi/180) * trajectory_factor;
    y = y + time_period * current_speed * cos(trajectory_angle * pi/180) * trajectory_factor;


    delay(5);
  }
}

// To make the bot move in reverse **********
void reverseBot(){
  moving = true;
  
  for (current_speed = 0; current_speed<= maximum_speed; current_speed+=speed_increment){
    m.motor(1, BACKWARD, current_speed);    
    m.motor(2, BACKWARD, current_speed);
    m.motor(3, BACKWARD, current_speed); 
    m.motor(4, BACKWARD, current_speed); 


    updateTime();
      
    x = x - time_period * current_speed * sin(trajectory_angle * pi/180) * trajectory_factor;
    y = y - time_period * current_speed * cos(trajectory_angle * pi/180) * trajectory_factor;

    delay(5);
  }
}

// Read distance at angle ******************
void angleDistance(int* dist, int theta, int last_theta){
  delay(50);
  head_servo.write(theta + initial_angle);
  delay(200);
  getDistance(dist);
  delay(100);
  head_servo.write(last_theta + initial_angle);
  delay(200);
}

// rotating right *************************
void rotateRight(){
  current_speed = maximum_speed;
  
  m.motor(1, BACKWARD, current_speed);    
  m.motor(2, FORWARD, current_speed);
  m.motor(3, FORWARD, current_speed); 
  m.motor(4, BACKWARD, current_speed);
  
  trajectory_angle += 700 * one_second_angle;
  Serial.println(trajectory_angle);
  Serial.println(700 * one_second_angle);
  Serial.println(one_second_angle);

  delay(700);
  
  m.motor(1,FORWARD, current_speed);    
  m.motor(2,FORWARD, current_speed);
  m.motor(3,FORWARD, current_speed); 
  m.motor(4,FORWARD, current_speed);


    updateTime();
      
    x = x + time_period * current_speed * sin(trajectory_angle * pi/180) * trajectory_factor;
    y = y + time_period * current_speed * cos(trajectory_angle * pi/180) * trajectory_factor;
}

// rotating left **************************
void rotateLeft(){
  current_speed = maximum_speed;
  
  m.motor(1, FORWARD, current_speed);    
  m.motor(2, BACKWARD, current_speed);
  m.motor(3, BACKWARD, current_speed); 
  m.motor(4, FORWARD, current_speed);
  
  trajectory_angle -= 700 * one_second_angle;
  delay(700);
  
  m.motor(1,FORWARD, current_speed);    
  m.motor(2,FORWARD, current_speed);
  m.motor(3,FORWARD, current_speed); 
  m.motor(4,FORWARD, current_speed);


    updateTime();
      
    x = x + time_period * current_speed * sin(trajectory_angle * pi/180) * trajectory_factor;
    y = y + time_period * current_speed * cos(trajectory_angle * pi/180) * trajectory_factor;


}

// Running the bot *************************
void runBot(){
  delay(100);
  getDistance(&global_distance);

  bool probe_result = false;

  if (millis() - probe_time >= 5000){
    delay(50);

    // slow_down_bot
    current_speed = maximum_speed/5;
    m.motor(1,FORWARD, current_speed);    
    m.motor(2,FORWARD, current_speed);
    m.motor(3,FORWARD, current_speed); 
    m.motor(4,FORWARD, current_speed);

    int dist_read = 400;

    for (int i = 50; i <= 170; i+=20){
      int dist;
      head_servo.write(i);
      delay(100);
      getDistance(&dist);
      delay(50);

      if (dist < dist_read){
        dist_read = dist;
      }
    }

    head_servo.write(110 + initial_angle);
    delay(200);

    updateTrajectory();
    current_speed = maximum_speed;
    m.motor(1, FORWARD, current_speed);    
    m.motor(2, FORWARD, current_speed);
    m.motor(3, FORWARD, current_speed); 
    m.motor(4, FORWARD, current_speed);

    if (dist_read < angular_threshold_distance){
      probe_result = true;
    }
    
    probe_time = millis();
  }

  if (probe_result || global_distance <= linear_threshold_distance){
    Serial.println(String(x) + " " + String(y) + " " + String(trajectory_angle));
    // stop the bot and scan surroundings
    updateTrajectory();
    stopBot();

    // send data - 
    sendData();
    int left_dist, right_dist;
    angleDistance(&left_dist, 170, 110);
    angleDistance(&right_dist, 50, 110);

    Serial.println("Left - " + String(left_dist) + ", Right - " + String(right_dist));

    updateTime();

    if (right_dist <= angular_threshold_distance 
          && left_dist <= angular_threshold_distance){
      reverseBot();
      delay(500);
      
    } else if (right_dist >= left_dist){
      rotateRight();
      Serial.println("Turned Right");
      stopBot();
      
    } else {
      rotateLeft();
      Serial.println("Turned Left");
      stopBot();
    }
    
  } else {
    if (!moving){
      moveBot();
    }
  }
}

// setup ***********************************
void setup(){
  Serial.begin(9600);

  // attaching servo
  head_servo.attach(10);
  head_servo.write(110 + initial_angle);

  delay(3000);
  Serial.println("Starting bot...");
  updateTime();

  // connecting to WiFi
  wifiConnect();  
}

// loop ************************************
void loop(){
  //Connecting to wifi
  Serial.println("Connecting to wifi...");
  WiFi.begin(ssid, password);
  if(WiFi.status() != WL_CONNECTED){
  delay(50);
  Serial.println("Connecting to wifi...");
 }
 Serial.println("Wifi connected!");
  if(checker_status==0)
    {
      stopBot();
      CreateCI("dummy");
    }

  // Running the bot
  runBot();
}
