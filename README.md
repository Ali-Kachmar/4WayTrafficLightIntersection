# 4WayTrafficLightIntersection
This project was created in the C language using FreeRTOS.
It includes a 4 way traffic light intersection with crosswalk capability, emergency vehicle detection, and the ability to detect congested traffic in a certain direction. Which in turn prolongs the green light in that direction and reflects the time given to the pedestrians in crossing in that direction.
This code uses an Arduino Mega 2560 R3, 5 pushbuttons, a piezo buzzer, 4 Traffic Light LEDs, 2 16x2 I2C LCD Displays, 1 20x4 I2C LCD Display and Jumper wires.
Unfortunately, there is no circuit diagram for this project however I do not believe it is too complicated to recreate in your own vision, just remember to update the pin declarations for each component in the code.

This project and its applications can be funcitonally described in layman's terms as what you can expect to see when you are out and about in any area where intersections
need to be controlled by means more sophisticated than simple stop signs at every corner.
The project not only allows for the possibilities of smooth traffic flow in congested areas via allocating time to give all persons a chance to cross the street in their respective directions
but also allows for these actions to be carried out in a safe and efficient manner. In this project, more than just traditional traffic flow was accounted for by implementing
semaphores and signaling for when emergency vehicles need priority in crossing through busy intersections and correspondingly when pedestrians are safe to cross the side roads
of the intersection. The simulated 4-way directional traffic coordinator is what you can expect from a real-world environment, minus the dedicated light signals for allowing turns through the intersection.

Below is a Description of Each Function:

setup()
 Initialize needed variables, taks, semaphores, displays, and etc.
 
loop() 
This function does nothing since the code is run on FreeRTOS.

Trafficlights()
This function is responsible for managing the main traffic light sequence for North/South and East/West directions.
It toggles between red, green, and yellow LEDS for each direction. 
Handles pedestrian crossing requests. 
Gets signaled by either NSsensor or EWsensor if there is need for a delay on a green LED. 
Displays to the main display that the traffic light task is active.

emergencyvehicleTask()
This function handles the emergency vehicle scenario. 
When the emergency button is pressed, it interrupts the normal traffic light sequence by linking red LEDs for both directions and forcing all traffic to yield. 
Activates the buzzer to let drivers know there is an emergency vehicle.

Turn_Red_Leds_on()
This function turns off all LEDs for both North/South and East/West directions. 
Displays to the main display that the emergency task is active.

Turn_Leds_off()
This function turns off all LEDs for both North/South and East/West directions.

Blink_Red_Leds()
This function is responsible for blinking the red LEDs for both directions, indicating an emergency vehicle is approaching.

PedestrianWalkingEW()
This function handles the pedestrian crossing request for the East/West direction. 
When the pedestrian button is pressed, it activates a countdown timer for crossing and updates the display accordingly.

PedestrianWalkingNS()
This function handles the pedestrian crossing request for the North/South direction. 
When the pedestrian button is pressed, it activates a countdown timer for crossing and updates the North/South display accordingly.

NSsensor()
The function reads the value of the North/South traffic button/sensor. 
If the sensor is triggered, it informs the traffic light task that there is a delay for the green light in the North/South direction and adds the delay to the North/South display.

EWsensor()
The function reads the value of the East/West traffic button/sensor. 
If the sensor is triggered, it informs the traffic light task that there is a delay for the green light in the East/West direction and adds the delay to the East/West display. 
