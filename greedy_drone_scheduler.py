#-----------------------------------------#
# Greedy Implementation of Drone Scheduler
# Goal: enforce continuous data collection
#-----------------------------------------#

import numpy as np
class Drone:
    def __init__(self, name, batt, curr_loc, goal_loc, state):
        self.name = name
        self.batt = batt
        self.curr_loc = curr_loc
        self.goal_loc = goal_loc
        self.state = state
        
    def __repr__(self):
        return str(self.name) + " - Battery: " + str(self.batt) + " Current Location: " + str(self.curr_loc) + " Goal Location: " + str(self.goal_loc) + " State: " + self.state
    
    def dist(self):
        return ((self.goal_loc[0]-self.curr_loc[0])**2 + (self.goal_loc[1]-self.curr_loc[1])**2)**0.5

    def vec(self, rate):
        return np.asarray(((self.goal_loc[0]-self.curr_loc[0])*rate/self.dist(), (self.goal_loc[1]-self.curr_loc[1])*rate/self.dist()))

def simulation(numdrones, numlocs, start, rate, dbatt, cbatt, numiters):
    airspace = []
    dronecount = numdrones
    locs = [(0,1), (1,0), (0,3), (3,0), (1,2), (2,1), (2,3), (3,2)] #define locations for each drone on the grid

    for i in range(numlocs):
        airspace.append(Drone(str(i+1), 100, start, np.asarray(locs[i]), 'a'))
        
    for i in range(numdrones-numlocs):
        airspace.append(Drone(str(i+numlocs+1), 100, start, np.asarray((0,0)), 'c')) #unused drones
    
    for i in range(numiters):
        print('\nIteration: ' + str(i))
        for dro in airspace:
            if dro.state == 'a':
                
                #look at threshold (distance from goal and how much battery left before swapping)
                if dro.batt < 2*dro.dist()*dbatt/rate: #required battery to get back to start location

                    #call func to replace current dying drone with new drone in airspace
                    max_charged = None
                    
                    for dros in airspace:
                        if dros.state == 'c':
                            if max_charged == None:
                                max_charged = dros
                            elif dros.batt == 100:
                                max_charged = dros
                                break
                            elif max_charged.batt < dros.batt:
                                max_charged = dros
                    
                    if not max_charged == None:
                        max_charged.goal_loc = dro.goal_loc  
                        max_charged.state = 'a'
                    else:
                        dronecount += 1
                        airspace.append(Drone(str(dronecount), 100, start, dro.goal_loc, 'a')) #assuming infinite supply of drones

                    dro.goal_loc = np.asarray((0,0))
                    dro.state = 'r'
                    print('Drone ' + str(dro.name) + ' is turning around from ' + str(dro.curr_loc))

                if np.any(np.abs(dro.curr_loc - dro.goal_loc) > 0.3): #arbitrary epsilon
                    vecc = dro.vec(rate) + dro.curr_loc
                    print('Drone ' + str(dro.name) + ' has moved from ' + str(dro.curr_loc) + ' to ' + str(vecc))
                    dro.curr_loc = vecc
                else:
                    dro.curr_loc = dro.goal_loc
                    if dro.state == 'r':
                        dro.state == 'c'
                        print('Drone ' + str(dro.name) + ' is charging')
                    else:
                        print('Drone ' + str(dro.name) + ' is already at goal: ' + str(dro))

                dro.batt -= dbatt #change battery life for every drone 

            if dro.state == 'c':
                dro.batt = min(100, dro.batt+cbatt)
                
        print(airspace)

simulation(14, 8, (0,0), 1, 4, 1.7, 10) 
