/* 
   Copyright (C) 2023 Noreen Jafri
   All Rights Reserved

   This source code is licensed under the MIT license found in the
   LICENSE file in the root directory of this source tree.
 */
#include <kipr/botball.h>
#include <stdbool.h>

//////////////////////////////////////
///////////  ROBOT CONFIG  ///////////
////////  (will need tweaking)  //////
//////////////////////////////////////

// servo ports
int SORT_SERVO = 1;
int TOP_SERVO = 2;

// motor ports
int LEFT_MOTOR = 0;
int RIGHT_MOTOR = 1;

// digital sensor prots
int LEFT_BUMP = 1;
int RIGHT_BUMP = 2;

// analog sensor ports
int LINE_SENSOR_LEFT = 0;
int LINE_SENSOR_RIGHT = 1;
int RIGHT_DEPTH_SENSOR = 2;

// positions
int LEFT = 1220;
int MIDDLE = 443;
int RIGHT = 1895;
int LINE_MIDDLE = 2500;

int ET_FOCAL_POINT = 2700;
int ET_TOO_CLOSE = 2900;

// arrays look ugly this looks better (speeds)
int MOTOR_SPEED_LEFT = 50; // 53.7;
int MOTOR_SPEED_RIGHT = 50; // 47.2;

// tick variables
int PAST_WHITESPACE = 1000;
int PAST_POMS = 50;

//////////////////////////////////////
////////////  FUNCTIONS  /////////////
//////////////////////////////////////

// for code readability
bool both_bump_pressed()
{
    /// if you cannot tell what this is doing:
    // && means and in programming
    // digital(LEFT_BUMP) returns 1 if the bump is pressed, 0 if it isn't (true or false)
    // digital(RIGHT_BUMP) returns 1 if the bump is pressed, 0 if it isn't (true or false)
    // so think, if both are pressed, you would get an expression looking something like true && true
    // true && true = true, true && false = false, false && false = false, false && true = false
    return (digital(LEFT_BUMP) && digital(RIGHT_BUMP)) == true;
}

// for code readability v2
bool on_line_horizontal()
{
    /// refer to both_bump_pressed() comments
    return (analog(LINE_SENSOR_LEFT) > LINE_MIDDLE && analog(LINE_SENSOR_RIGHT) > LINE_MIDDLE) == true;
}

bool on_line_vertical()
{
    /// refer to both_bump_pressed() comments
    // our goal is to have either one line sensor on white and the other on black
    // the one that's on black or white doesn't matter, and because it doesn't matter we have || here
    // think of it more like this:
    // return whether the left line sensor is on white and the right line sensor in on black
    // or the left line sensor is on black and the right line sensor is on white
    // return whether the value of that is equal to true
    return ((analog(LINE_SENSOR_LEFT) > LINE_MIDDLE && analog(LINE_SENSOR_RIGHT) < LINE_MIDDLE) ||
        (analog(LINE_SENSOR_LEFT < LINE_MIDDLE && analog(LINE_SENSOR_RIGHT) > LINE_MIDDLE))) == true;
}

void line_follow()
{
    while (on_line_vertical())
    {
    }
}

void square_up(int times)
{
    int i;
    for (i = 0; i < times; i++) {
        // square up
        // while both of the bump sensors are not pressed move backwards until they are
        while (!both_bump_pressed())
        {
            // waiting to be pressed
            motor(LEFT_MOTOR, -MOTOR_SPEED_LEFT);
            motor(RIGHT_MOTOR, -MOTOR_SPEED_RIGHT);
        } // once this exits both bump sensors are obviously pressed
    }
}

void move_past_whitespace()
{
    // as long if we're not on the line
    while (!on_line_horizontal())
    {
        motor(LEFT_MOTOR, MOTOR_SPEED_LEFT);
        motor(RIGHT_MOTOR, MOTOR_SPEED_RIGHT);
    } // if we've exited out of this while loop it means that we are past the whitespace
    
    ao();
}

void swipe_poms()
{
    // when bump pressed do this stuff
    // our backs should be by this point to the pvc pipe so we can now start the sorting
    bool rightSide = true;
    while (1)
    {
        // clear our position counters
        cmpc(LEFT_MOTOR);
        cmpc(RIGHT_MOTOR);

        // move to sort new pom
        while (gmpc(LEFT_MOTOR) < PAST_POMS)
        {
            motor(LEFT_MOTOR, MOTOR_SPEED_LEFT);
            motor(RIGHT_MOTOR, MOTOR_SPEED_RIGHT);
        }

        // if we are supposed to move right move right, otherwise move left
        set_servo_position(SORT_SERVO, rightSide ? RIGHT : LEFT);
        rightSide = !rightSide;
        
        printf("Right side is %s\n", rightSide ? "true" : "false");
    }
}

void turn_right()
{
    motor(LEFT_MOTOR, MOTOR_SPEED_LEFT);
    msleep(1850);
    ao();
}

void turn_left()
{
    motor(RIGHT_MOTOR, MOTOR_SPEED_RIGHT);
    msleep(1850);
    ao();
}

void square_with_threshold()
{
    while (!both_bump_pressed())
    {
        // less then the focal point means we're not close enough
        if (analog(RIGHT_DEPTH_SENSOR) < ET_FOCAL_POINT)
        {
            motor(LEFT_MOTOR, -40);
            motor(RIGHT_MOTOR, -MOTOR_SPEED_RIGHT);
            // printf("\nwe're trying to shift right ");
        } else if (analog(RIGHT_DEPTH_SENSOR) > ET_FOCAL_POINT)
        {
            // greater then the focal point means we're closer
            // if its less then too close, we go backward
            if (analog(RIGHT_DEPTH_SENSOR) < ET_TOO_CLOSE)
            {
                // do stuff
                motor(LEFT_MOTOR, -MOTOR_SPEED_LEFT);
                motor(RIGHT_MOTOR, -MOTOR_SPEED_RIGHT);
                // printf("\ngoing forward/backward ");
            } else {
                // if we get too close turn right
                motor(LEFT_MOTOR, -MOTOR_SPEED_RIGHT);
                motor(RIGHT_MOTOR, -30);
                // printf("\nturn left ");
            }
        }
    }

    printf("Bump sensors have turned on, likely at pvc pipe...\n");
    printf("Moving forward to give robot breathing room.\n");

    // once we exited the while loop this means we are currenctly
    // facing our back to the pvc pipes
    // to give our robot a little breathing room, we move forward a little bit
    motor(LEFT_MOTOR, MOTOR_SPEED_LEFT);
    motor(RIGHT_MOTOR, MOTOR_SPEED_RIGHT);
    msleep(300);
    ao();
}

void drive_to_line()
{
    while (!on_line_horizontal())
    {
        motor(LEFT_MOTOR, MOTOR_SPEED_LEFT);
        motor(RIGHT_MOTOR, MOTOR_SPEED_RIGHT);
    }
    ao();
}

//////////////////////////////////////
////////////  MAIN FUNC  /////////////
//////////////////////////////////////
// categorized different parts into functions for ease of collaboration and lack of oop in c
int main()
{
    enable_servos();
    cmpc(LEFT_MOTOR); // use this for reference purposes
    cmpc(RIGHT_MOTOR);

    printf("Testing pull request\n");
   
    // we start in the middle of the board, in the narrow start box
    // we turn right without moving forward, and then square up 
    // (move backwards until both bumps are pressed to signify a straight robot)
    // keep between 2700 and 2900
    printf("Starting in the middle of the board, turning left...\n");
    turn_left();
    
    printf("Keeping robot in threshold of between 4 and 5 inches away from pvc pipe...\n");
    square_with_threshold();
    
    // we then turn right to face the black line above us
    turn_right();
    
    printf("Facing front to black line.\n");
    
    // drive until we are centered in the middle 
    drive_to_line();
    
    printf("Centered in the middle of line.");

    // since we are still facing straight we turn right to face the line, 
    // turn left (we might do a vertical line check but it's probably fine)
    turn_left();

    // we then start swiping poms
    swipe_poms();
    
    // move past whitepsace
    // move_past_whitespace();

    // turn right
    // motor(LEFT_MOTOR, MOTOR_SPEED_LEFT);
    // msleep(1850);

    // square up once (only if on left start box)
    // square_up(1);
 
    // start swiping the poms
    // swipe_poms();

    // turn off all motors
    ao();
    
    return 0;
}
