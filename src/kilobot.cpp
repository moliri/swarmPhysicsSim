#pragma once
#include "kilolib.h"

class mykilobot : public kilobot
{
	unsigned char distance;
	message_t out_message;
	int rxed=0;
	float theta;
	float modTheta;

	float vmotion;
	float vmotionSum = 0;
	float vrepulsion;
	float vattraction;

	int mySpin;

	float regionRadius = 96;
	float motorTurnStrength = 50;
	
	int motion=0;
	long int motion_timer=0;

	int state = 0;
	bool transmissionComplete = 0;
	float theta_distance;

	int msrx=0;
	struct mydata {
		unsigned int data1;
		unsigned int data2;

	};

	//main loop
	void loop()
	{	
		// modTheta = fmod(theta, 2*PI);
		// vmotion = fmod(vmotion, 2*PI);
		printf("modTheta =%f\n\r",modTheta);
		printf("vmotion =%f\n\r",vmotion);
		printf("vmotionSum =%f\n\r",vmotionSum);


		if(state == 0) { //listening to neighbors
			printf("state: %d\n", state);
			if (transmissionComplete) {
				state = 1;
				theta_distance = 10;
			}
		}

		else if (state == 1) { //rotate
			printf("state: %d\n", state);

			if(fabs(vmotion+PI/2) <.1) { //if the angle is within the threshold
				state = 2;
			}

			else if((vmotion+PI/2) > 0) {
				spinup_motors();
				set_motors(motorTurnStrength,0);
				printf("turn left\n");
			}

			else
			{
				spinup_motors();
				set_motors(0,motorTurnStrength);
				printf("turn right\n");
			}
		}

		else if (state == 2) { //move forward 
			printf("state: %d\n", state);
			spinup_motors();
			set_motors(motorTurnStrength,motorTurnStrength);
			printf("move straight\n");
			theta_distance--;

			if (theta_distance <=0) {
				//reset
				transmissionComplete = 0;
				state = 0;
				vmotionSum = 0;
				// set_motors(0,0);
			}
		}

		// printf("compass =%f\n\r",compass);
		// printf("theta =%f\n\r",theta);
		// float modTheta = fmod(theta, 2*PI);
		// printf("modTheta =%f\n\r",modTheta);
		// // printf("abs compass =%f\n\r",fabs(compass));
		// if(fabs(modTheta+PI/2) <.1) {
		// 	spinup_motors();
		// 	set_motors(50,50);
		// 	printf("move straight");
		// }
		// else if((modTheta+PI/2) > 0) {
		// 	spinup_motors();
		// 	set_motors(kilo_turn_left,0);
		// 	printf("turn left");
		// }
		// else {
		// 		spinup_motors();
		// 		set_motors(0,kilo_turn_right);
		// 		printf("turn right");
		// }


		// //default code
		// if(id==0) //follow other robot
		// {
			
		// 	if(fabs(theta)<.3)
		// 	{
				
		// 		spinup_motors();
		// 		set_motors(50,50);

		// 	}
		// 	else if(theta<0)
		// 	{
		// 		spinup_motors();
		// 		set_motors(kilo_turn_left,0);
			
			

		// 	}
		// 	else
		// 	{
		// 		spinup_motors();
		// 		set_motors(0,kilo_turn_right);
	
			
		// 	}
		// }
		// else
		// {
		// 	printf("compass =%f\n\r",compass);
			
		// 	if(fabs(compass-1.5)<.1)
		// 	{
				
		// 		spinup_motors();
		// 		set_motors(50,50);

		// 	}
		// 	else if(compass-1.5<0)
		// 	{
		// 		spinup_motors();
		// 		set_motors(kilo_turn_left,0);
			
			

		// 	}
		// 	else
		// 	{
		// 		spinup_motors();
		// 		set_motors(0,kilo_turn_right);
	
			
		// 	}

		// }		
		
	}

	//executed once at start
	void setup()
	{
		// set spin to 0 or 1
		mySpin = rand() % 2;
		out_message.data[0] = mySpin;

		out_message.type = NORMAL;
		out_message.crc = message_crc(&out_message);
		set_color(RGB(0,1,0)); //starting color doesn't matter
	}

	//executed on successfull message send
	void message_tx_success()
	{
		//set_color(RGB(1,0,0));
		transmissionComplete = 1;
		
	}

	//sends message at fixed rate
	message_t *message_tx()
	{
		static int count = rand();
		count--;
		if (!(count % 10))
		{
			return &out_message;
		}
		return NULL;
	}

	//receives message
	void message_rx(message_t *message, distance_measurement_t *distance_measurement,float t)
	{
		distance = estimate_distance(distance_measurement);
		theta=t;
		int neighborSpin = message->data[0];
		printf("distance: %d\n", distance);


		printf("spin =%d\n\r",neighborSpin);
		
		modTheta = fmod(theta, 2*PI);
		
		//flip spin if distance is too close 
		if (distance <= 64) {
			// flip spin to 0 or 1
			printf("flip spin\n");
			if (mySpin==0) {
				mySpin = 1;
			}
			else {
				mySpin = 0;
			}
			out_message.data[0] = mySpin;
		}

		//determine region radius based on spin
		if (neighborSpin==mySpin) {
			//if I have same spin as neighbor
			 regionRadius = sqrt(2)*regionRadius;
			 printf("same spin\n");
		}
		else {
			//if I have different spin as neighbor
			regionRadius = 96; //hard coded =(
			printf("diff spin\n");
		}

		if (distance < regionRadius) {
			//repulsion
			printf("repulsion\n");
			if (modTheta < 0) {
				vmotion = modTheta - PI;
			}
			else {
				vmotion = modTheta + PI;
			}
		}
		else if (distance > regionRadius) {
			//attraction
			vmotion = modTheta;
			printf("attraction\n");
		}
		else {
			//neutral
			// vmotion = 0;
			motorTurnStrength = 20; //dampening turn strength if they are within range
		}

		vmotionSum = sumVectors(vmotion, vmotionSum);
		
	}

	float sumVectors(float thetaA, float thetaB) {
		float xposA = cos(thetaA);
		float yposA = sin(thetaA);
		float xposB = cos(thetaB);
		float yposB = sin(thetaB);

		float xposSum = xposA + xposB;
		float yposSum = yposA + yposB;

		float thetaSum = atan2(yposSum, xposSum);

		return thetaSum;
	}
};

