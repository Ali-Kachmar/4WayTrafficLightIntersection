/*
 * This code is for ECE 5367 with Dr. Harry Le. 
 * Written by Ali Kachmar, Jake Gonzalez, and Chandler Pickett. 
 * In this code, FreeRTOS tasks are implemented using priority scheduling with the help of semaphores.
 * This code mimicks a real life 4 way intersection with priority traffic for emergency vehicles, displaying a count down timer and notifying vehicles when it is safe to continue driving.
 * Furthermore, this code implements the ability for pedestrian traffic requests, in both directions (N/S and E/W) through task scheduling utilizing pushbuttons. 
 * This task, when ran, also displays a countdown timer notifiying the pedestrian of the remaining time to cross before traffic begins to flow once again.
 */


#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define the GPIO pins for the traffic light LEDs
const int RED_LED_NS = 2;
const int YELLOW_LED_NS = 3;
const int GREEN_LED_NS = 4;
const int RED_LED_EW = 5;
const int YELLOW_LED_EW = 6;
const int GREEN_LED_EW = 7;
const int EMERGENCY_BUTTON = 8; 
const int NS_PEDESTRIAN = 9; 
const int EW_PEDESTRIAN = 10; 
const int NS_SENSOR = 11; 
const int EW_SENSOR = 12; 
const int EMERGENCY_BUZZER = 29;

// Time durations for each traffic light stage in milliseconds
const int RED_DURATION = 5000;
const int YELLOW_DURATION = 2000;
const int GREEN_DURATION = 5000;
// Initialize variables
unsigned long emergencybutton = 1; 
unsigned long NSwalk = 1; 
unsigned long EWwalk = 1; 
unsigned long nstrafficsensor = 1; 
unsigned long ewtrafficsensor = 1; 
unsigned long emergencystop = 1; 
unsigned long emergencyactive = 0; 

int flashcounter; //counter for red_led_blink
int displaycounter = 4; //the counter for display

//intialize semaphores
SemaphoreHandle_t semaphore1, semaphore3, semaphore4, nspedbutton, ewpedbutton, nssensor, ewsensor, nsdisdelay, ewdisdelay;

//intialize LCD displays
LiquidCrystal_I2C NSPed(0x23, 16, 2);
LiquidCrystal_I2C EWPed(0x27, 16, 2);
LiquidCrystal_I2C LCD(0x3F, 20, 4);

void setup()
{
  // Initialize the traffic light pins as output
  pinMode(RED_LED_NS, OUTPUT);
  pinMode(YELLOW_LED_NS, OUTPUT);
  pinMode(GREEN_LED_NS, OUTPUT);
  pinMode(RED_LED_EW, OUTPUT);
  pinMode(YELLOW_LED_EW, OUTPUT);
  pinMode(GREEN_LED_EW, OUTPUT);

  //increase clockrate for display 
  Wire.setClock(400000L);

  //intialize buttons with pull up resistors
  pinMode(EMERGENCY_BUTTON, INPUT); //button for emergency vehicle task
  pinMode(EMERGENCY_BUTTON, INPUT_PULLUP); 

  pinMode(NS_PEDESTRIAN, INPUT); //button for pedestrian walking n/s
  pinMode(NS_PEDESTRIAN, INPUT_PULLUP); 

  pinMode(EW_PEDESTRIAN, INPUT); //button for pedestrian walking e/w
  pinMode(EW_PEDESTRIAN, INPUT_PULLUP); 

  pinMode(NS_SENSOR, INPUT); //button for when there's heavy traffic on n/s
  pinMode(NS_SENSOR, INPUT_PULLUP);

  pinMode(EW_SENSOR, INPUT); //button for when there's heavy traffic on e/w
  pinMode(EW_SENSOR, INPUT_PULLUP);

//Create the binary semaphores
semaphore1 = xSemaphoreCreateBinary(); //signal to know which led task to happen 
semaphore3 = xSemaphoreCreateBinary(); //signal to active pedestiran walk n/s 
semaphore4 = xSemaphoreCreateBinary(); //signal to active pedestrian walk e/w
nspedbutton = xSemaphoreCreateBinary(); //signal to let traffic light system know pedestiran walking n/s is happening
ewpedbutton = xSemaphoreCreateBinary(); //signal to let traffic light system know pedestiran walking n/s is happening
nssensor = xSemaphoreCreateBinary(); // signal for when there's heavy traffic on n/s road
ewsensor = xSemaphoreCreateBinary(); // signal for when there's heavy traffic on e/w road
nsdisdelay = xSemaphoreCreateBinary(); //signal for display delay on n/s pedestrian walk 
ewdisdelay = xSemaphoreCreateBinary(); //signal for dipslay delay on e/w pedestiran walk


  // Create FreeRTOS tasks
  xTaskCreate(Trafficlights, "Traffic Lights", 128, NULL, 1, NULL); //task for activating leds for traffic light
  xTaskCreate(emergencyvehicleTask, "Emergency Vehicle", 128, NULL, 2, NULL); // task for activating traffic light leds for emergency vehicle
  xTaskCreate(PedestrianWalkingNS, "Traffic Light NS", 128, NULL, 1, NULL); //task for activing walking display for n/s direction
  xTaskCreate(PedestrianWalkingEW, "Traffic Light EW", 128, NULL, 1, NULL); //task for activing walking display for e/w direction
  xTaskCreate(NSsensor, "Sensor for NS", 128, NULL, 1, NULL); //button for telling if there's heavy traffic on n/s 
  xTaskCreate(EWsensor, "Sensor for EW", 128, NULL, 1, NULL); //button for telling if there's heavy traffic on e/w 


  //intializing display for n/s direction
  NSPed.begin();
  NSPed.backlight();
  NSPed.setCursor(0, 0);
  NSPed.print("NO CROSSING REQ");
  NSPed.setCursor(0,1);
  NSPed.print("DO NOT WALK!");

  //intializing display for e/w direction
  EWPed.begin();
  EWPed.backlight();
  EWPed.setCursor(0, 0);
  EWPed.print("NO CROSSING REQ");
  EWPed.setCursor(0,1);
  EWPed.print("DO NOT WALK!");

  LCD.begin();
  LCD.backlight();

 
  //starts the code by giving the semaphore to the traffic light task
  xSemaphoreGive(semaphore1);
}

void loop()
{
  // No code is required here, as the tasks are managed by FreeRTOS
}

void Trafficlights(void *pvParameters)
{
  for (;;) //for loop to keep the task keep going 
  {

    if(xSemaphoreTake(semaphore1, portMAX_DELAY)== pdPASS){ //takes semaphore1 to activate task
    LCD.clear();
    //first the red led for both directions needs to be red throughout this loop
    digitalWrite(RED_LED_NS,HIGH);
    digitalWrite(RED_LED_EW,HIGH);

    //delay for red led lights
    vTaskDelay(pdMS_TO_TICKS(1500));
  
    //turn off red led for n/s
    digitalWrite(RED_LED_NS,LOW);

    //if loop to see if the pedestrian for n/s has been activated
    if(xSemaphoreTake(nspedbutton, 0)==pdPASS){
     if(xSemaphoreTake(nsdisdelay,0)==pdPASS){ //if loop to see if the green light is longer due to heavy traffic
    displaycounter = 6; 
    }
    else{
      displaycounter = 4; 
    }
    xSemaphoreGive(semaphore3); //give semaphore to pedestiran walk n/s to activate 
    }
    xSemaphoreTake(nsdisdelay,0); //takes the nsdisdelay semaphore incase the nspedbutton isn't activate 
  
    //turn on green led for n/s
    digitalWrite(GREEN_LED_NS,HIGH);

    LCD.clear();
    LCD.print("Normal North Bound");
    LCD.setCursor(0,1);
    LCD.print("and South Bound");
    LCD.setCursor(0,2);
    LCD.print("Traffic May Proceed");

    if(xSemaphoreTake(nssensor, 0)==pdPASS){ //if there's longer traffic, make green light longer 

      vTaskDelay(pdMS_TO_TICKS(2000));
    }
    //green light delay
    vTaskDelay(pdMS_TO_TICKS(GREEN_DURATION));

    //turn off green led for n/s
    digitalWrite(GREEN_LED_NS,LOW);

    //turn on yellow led for n/s
    digitalWrite(YELLOW_LED_NS,HIGH);

    //yellow light delay
    vTaskDelay(pdMS_TO_TICKS(YELLOW_DURATION));
    
    //turn off yellow led for n/s 
    digitalWrite(YELLOW_LED_NS,LOW);
    
    //turn on red led for n/s
    digitalWrite(RED_LED_NS,HIGH);

    xSemaphoreGive(semaphore1);//give semaphore1 to determine next led lights
    }
    // START OF E/W Traffic Light TASK FOR LEDS

    if(xSemaphoreTake(semaphore1, portMAX_DELAY)== pdPASS){//takes semaphore1 to activate e/w led
    LCD.clear();
    //turn on red led lights for both intersections
    digitalWrite(RED_LED_NS,HIGH);
    digitalWrite(RED_LED_EW,HIGH);

    //delay red lights
    vTaskDelay(pdMS_TO_TICKS(1500));

    //turn off red led for e/w
    digitalWrite(RED_LED_EW,LOW);

    //if loop to see if the pedestrian for n/s has been activated
    if(xSemaphoreTake(ewpedbutton, 0)==pdPASS){
    if(xSemaphoreTake(ewdisdelay,0)==pdPASS){ // if loop to check if display needs a delay 
    displaycounter = 6; 
    }
    else{
      displaycounter = 4; 
    }
    xSemaphoreGive(semaphore4);  //give semaphore to pedestiran walk n/s to activate 
    }
    xSemaphoreTake(ewdisdelay,0);//takes the ewdisdelay semaphore incase the nspedbutton isn't activate 
    

    //turn on green led for e/w
    digitalWrite(GREEN_LED_EW,HIGH);

    LCD.clear();
    LCD.print("Normal East Bound");
    LCD.setCursor(0,1);
    LCD.print("and West Bound");
    LCD.setCursor(0,2);
    LCD.print("Traffic May Proceed");
    
    if(xSemaphoreTake(ewsensor, 0)==pdPASS){
      
      vTaskDelay(pdMS_TO_TICKS(2000));
    }

    //delay for green led light 
    vTaskDelay(pdMS_TO_TICKS(GREEN_DURATION));

    //turn off green led for e/w
    digitalWrite(GREEN_LED_EW,LOW);

    //turn on yellow led for e/w
    digitalWrite(YELLOW_LED_EW,HIGH);

    //delay for yellow led light
    vTaskDelay(pdMS_TO_TICKS(YELLOW_DURATION));

    //turn off yellow led for e/w
    digitalWrite(YELLOW_LED_EW,LOW);

    //turn on red led light
    digitalWrite(RED_LED_EW,HIGH);
 
    xSemaphoreGive(semaphore1); //give semaphore1 to determine which leds to continue next 
    }
    
    
  }
}


void emergencyvehicleTask(void *pvParameters){
  
    vTaskDelay(pdMS_TO_TICKS(5000)); //delay to allow for other tasks to happen
    for(;;){ //for loop to to make the task keep going 
    emergencybutton = digitalRead(EMERGENCY_BUTTON); //read in the value of the emergency button
    if(emergencybutton == LOW){ //if loop to check if the button is pressed
        xSemaphoreTake(semaphore1,portMAX_DELAY);  //waits for semaphore 1 to be avaiable from the traffic light task
    tone(EMERGENCY_BUZZER,650);
    Blink_Red_Leds(); //blink red leds
    Turn_Red_Leds_on(); //turn leds back on to red
    noTone(EMERGENCY_BUZZER);
    tone(EMERGENCY_BUZZER,900);
    noTone(EMERGENCY_BUZZER);
    xSemaphoreGive(semaphore1);//gives back semaphore1 so another led task can start 
    }
     vTaskDelay(10); //prevents issues
    }
}


void Turn_Red_Leds_on(){
  //turning off all leds
  digitalWrite(RED_LED_NS,LOW);
  digitalWrite(YELLOW_LED_NS,LOW); 
  digitalWrite(GREEN_LED_NS,LOW); 
  digitalWrite(RED_LED_EW,LOW);
  digitalWrite(YELLOW_LED_EW,LOW); 
  digitalWrite(GREEN_LED_EW,LOW); 

  //turning on only red lights
  digitalWrite(RED_LED_NS,HIGH);
  digitalWrite(RED_LED_EW,HIGH);

}
void Turn_Leds_Off(){
    //turning off all leds
  digitalWrite(RED_LED_NS,LOW);
  digitalWrite(YELLOW_LED_NS,LOW); 
  digitalWrite(GREEN_LED_NS,LOW); 
  digitalWrite(RED_LED_EW,LOW);
  digitalWrite(YELLOW_LED_EW,LOW); 
  digitalWrite(GREEN_LED_EW,LOW); 
}

void Blink_Red_Leds(){
  // blink only red lights
  flashcounter = 0; 
  int timer = 6;
  LCD.clear();
  LCD.print("Emergency Vehicle");
  LCD.setCursor(0,1); // prints it on second line
  LCD.print("Priority Traffic");
  LCD.setCursor(0,2);// prints it on third line
  LCD.print("ALL OTHER TRAFFIC");
  LCD.setCursor(0,3); //prints it on fourth and final line
  LCD.print("MUST YIELD! TIME:");
  LCD.display();
  while(flashcounter < 6 ){
  digitalWrite(RED_LED_NS,HIGH);
  digitalWrite(RED_LED_EW,HIGH);
  vTaskDelay(pdMS_TO_TICKS(500));
  digitalWrite(RED_LED_NS,LOW);
  digitalWrite(RED_LED_EW,LOW);
  vTaskDelay(pdMS_TO_TICKS(500));
  flashcounter++;
  timer--;
  LCD.setCursor(19,3);
  LCD.print(timer); 
  }
}

void PedestrianWalkingEW(void *pvParameters){
   
  vTaskDelay(pdMS_TO_TICKS(2000)); //delay to allow proper synchornization 
  for(;;){ //for loop to keep task going 
    EWwalk = digitalRead(EW_PEDESTRIAN);  //read value from pedestrian push button for ew
 if(EWwalk== LOW ){ //if loop to check if button is pushed for pedestrian button for ew
   xSemaphoreGive(ewpedbutton); //notify the traffic light task that it is ready to use pedestrian task ew
   //once button is pressed, display notifies you that the button has been pressed and ur request is pending
    EWPed.clear();
    EWPed.setCursor(0,0);
    EWPed.print("E/W CrossWalk");
    EWPed.setCursor(0,1);
    EWPed.print("Requested");
 if(xSemaphoreTake(semaphore4, portMAX_DELAY)==pdPASS){ //waiting signal to know if green light is active
    EWPed.clear(); //clear display 
    EWPed.setCursor(0,0);
    EWPed.print("East/West May"); //let pedestiran know to go 
    EWPed.setCursor(0,1);
    EWPed.print("Start Walking");
    vTaskDelay(pdMS_TO_TICKS(1000)); //delay before next count
    for (int i = displaycounter; i >= 1; i--) { //loop to count down for the display 
      EWPed.clear(); 
      EWPed.setCursor(0,0);
      EWPed.print("Time Left: ");
      EWPed.setCursor(11,0);
      EWPed.print(i);
      EWPed.setCursor(12,0);
      EWPed.print("s");
      vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 1 second
   }
   EWPed.clear();  //clear display
   EWPed.setCursor(0,0);
   EWPed.print("NO CROSSING REQ");
   EWPed.setCursor(0,1);
   EWPed.print("DO NOT WALK!");
 }
 }
  }
}
void PedestrianWalkingNS(void *pvParameters){
  vTaskDelay(pdMS_TO_TICKS(2000)); //delay to allow proper synchornization 
  for(;;){ //for loop to keep task going
    NSwalk = digitalRead(NS_PEDESTRIAN); //read value from pedestrian push button for ns
  if(NSwalk == LOW){ //if loop to check if button is pushed for pedestrian button for ns
    xSemaphoreGive(nspedbutton); //notify the traffic light task that is ready to use pedestrian task ns
    //once button is pressed, display notifies you that the button has been pressed and ur request is pending
    NSPed.clear();
    NSPed.setCursor(0,0);
    NSPed.print("N/S CrossWalk");
    NSPed.setCursor(0,1);
    NSPed.print("Requested");
   if(xSemaphoreTake(semaphore3, portMAX_DELAY)==pdPASS){//waiting signal to know if green light is active
    NSPed.clear(); //clear display 
    NSPed.setCursor(0,0);
    NSPed.print("North/South May"); //let pedestiran know to go 
    NSPed.setCursor(0,1);
    NSPed.print("Start Walking");
   vTaskDelay(pdMS_TO_TICKS(1000));  //dleay before next count
      for (int i = displaycounter; i >= 1; i--) {//loop to count down for the display
      NSPed.clear(); 
      NSPed.setCursor(0,0);
      NSPed.print("Time Left: ");
      NSPed.setCursor(11,0);
      NSPed.print(i);
      NSPed.setCursor(12,0);
      NSPed.print("s");
      vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 1 second
   }
   NSPed.clear(); //clear display 
   NSPed.setCursor(0, 0);
   NSPed.print("NO CROSSING REQ");
   NSPed.setCursor(0,1);
   NSPed.print("DO NOT WALK!");
   }
 }
  }
}
void NSsensor(void *pvParameters){
  for(;;){
  nstrafficsensor = digitalRead(NS_SENSOR); //reading the value of the nssensor
  if(nstrafficsensor == 0){ //checks if nssensor is pushed
    xSemaphoreGive(nssensor); //lets the traffic light task know there is delay for green light ns
    xSemaphoreGive(nsdisdelay); //adds the delay to the display for ns
  }
  }
}
void EWsensor(void *pvParameters){
    for(;;){
  ewtrafficsensor = digitalRead(EW_SENSOR);//reading the value of the nssensor
  if(ewtrafficsensor == 0){  //checks if ewsensor is pushed
    xSemaphoreGive(ewsensor); //lets the traffic light task know there is delay for green light ew
    xSemaphoreGive(ewdisdelay); //adds the delay for the display for ew
  }
  }
}
