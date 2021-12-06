/* This code estimates the battery life of a UAV based on different wind measurements that are spatially distributed in an area as it travels from start to end.*/

#include <iostream>
#include <tuple>
#include <vector>
#include <math.h>

int AIRSPEED = 30;
int BATTERY = 1250;
int POWER = 25;
#define PI 3.14159265
using namespace std;

//interpolate various winds based on a inverse weighted metric (closer wind stations given better estimates than those further away)
//NOTE: given more flexibility for implementation (time, data, etc) a better approximation would happen more frequently than just at the midpoint of the path travelled
//Also, since we're only dealing with a few points that are closely spaced this algo considers all points but a smarter algo would use k closest stations or ignore any calculations that drop off to 0
pair<float, float> calculateWind(vector<pair<pair<float, float>, pair<float, float> > > windMeasures, pair<float, float> (prevWaypoint), pair<float, float> (waypoint)){
    float newX = get<0>(waypoint);
    float newY = get<1>(waypoint);
    float oldX = get<0>(prevWaypoint);
    float oldY = get<1>(prevWaypoint);
    pair<float, float> midpoint ((newX+oldX)/2.0, (newY+oldY)/2.0);

    float totalWindX  = 0;
    float totalWindY = 0;
    float windMag = 0;
    float windDir = 0;

    for(pair<pair<float, float>, pair<float, float> > station : windMeasures){
        float x = get<0>(get<0>(station));
        float y = get<1>(get<0>(station));
        float mag = get<0>(get<1>(station));
        float dir = get<1>(get<1>(station));
        float xDist = abs(get<0>(midpoint) - x);
        float yDist = abs(get<1>(midpoint) - y);

        float wX = 0;
        float wY = 0;

        for(int i = 0; i < windMeasures.size(); i++){
            if(xDist < 10) xDist = 0;
            if(yDist < 10) yDist = 0;
            if(xDist == 0){
                totalWindX = mag*cos(dir*PI/180);
                wX = 1;
                break;
            }
            if(yDist == 0){
                totalWindY = mag*sin(dir*PI/180);
                wY = 1;
                break;
            }
            if(xDist <= 300){ //don't care about stations after a certain distance
                float wXDist = 1/xDist;
                wX += wXDist;
                totalWindX += mag*cos(dir*PI/180)*wXDist;
            }
            if(yDist <= 300){
                float wYDist = 1/yDist;
                wY += wYDist;
                totalWindY += mag*sin(dir*PI/180)*wYDist;
            }
        }
        if(wX == 0) totalWindX = 0;
        else totalWindX /= wX;
        if(wY == 0) totalWindY = 0;
        else totalWindY /= wY;
    }
    windMag = sqrt(pow(totalWindX, 2) + pow(totalWindY, 2));
    windDir = int((atan2(totalWindX, totalWindY)* 180 / PI) + 360.0) % 360;
    return pair<float, float> (windMag, windDir);
}

//calculates the battery life after traveling to a designated waypoint
//parameters: current battery life, source waypoint, destination waypoint, aggregate wind (tuple of magnitude and direction), power required for traveling at airspeed
float calculateBattery(float battLife, pair<float, float> (prevWaypoint), pair<float, float> (waypoint), pair<float, float> (wind), float power){
    //seperate the tuples passed in for easy usage below
    float newX = get<0>(waypoint);
    float newY = get<1>(waypoint);
    float oldX = get<0>(prevWaypoint);
    float oldY = get<1>(prevWaypoint);
    float windMag = get<0>(wind);
    float windDir = get<1>(wind);
    
    //since airspeed is constant, the wind affects the groundspeed
    //below are relevant calculations to determine the groundspeed given the wind parameter
    int heading = int((atan2(newX-oldX, newY-oldY)* 180 / PI) + 360.0) % 360;
    pair<float, float> groundSpeed (AIRSPEED*sin(heading)+windMag*sin(windDir), AIRSPEED*cos(heading)+windMag*cos(windDir));
    float groundMag = sqrt(pow(get<0>(groundSpeed), 2) + pow(get<1>(groundSpeed), 2));
    
    //distance travelled from source waypoint to destination waypoint
    float dist = sqrt(pow(newX-oldX, 2) + pow(newY-oldY, 2));

    //update the battery life based on the time taken to travel the calculated distance at the calculated ground speed
    battLife -= (dist/groundMag)*power;

    //print statements for debugging
    std::cout << "Ground Speed: " << groundMag << endl;
    std::cout << "Heading: " << heading << endl;
    std::cout << "Wind Speed: " << windMag << endl;
    std::cout << "Wind Heading: " << windDir << endl;
    std::cout << "Distance Travelled: " << dist << endl;
    std::cout << "New Battery Life: " << battLife << endl;
    std::cout << "Source: " << get<0>(prevWaypoint) << " " << get<1>(prevWaypoint) << endl;
    prevWaypoint = waypoint;
    std::cout << "Destination: " << get<0>(prevWaypoint) << " " << get<1>(prevWaypoint) << endl;
    
    return battLife;
}

int main() {
    //list of waypoint coordinates
    vector<pair<float, float> > data = {{0,0}, {100,100}, {200,100}, {350,150}, {450,250}, {500,400}, {420,450}, {250,300}, {110,220}, {-50,100}};
    vector<pair<pair<float, float>, pair<float, float> > > windMeasures = {{{50,30},{15,135}}, {{350,260},{9,0}}, {{550,520},{2,350}}, {{240,560},{15,330}}, {{130,300},{22,315}}};
    float b = BATTERY;

    //loop through the waypoints
    for(int i=1; i < data.size(); i++){
        pair<float, float> wind = calculateWind(windMeasures, data[i-1], data[i]);
        b = calculateBattery(b, data[i-1], data[i], wind, POWER);
    }
    std::cout << "Final Battery Life: " << b << endl;
    return b;
}
