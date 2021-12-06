#-------------------------------------------------------------------------------------------------------#
# Traffic Analysis (Lane Identification) Code 
# drone video data fed in as input to object detector, detections are tracked, tracked IDs analyzed here
#-------------------------------------------------------------------------------------------------------#

import numpy as np
import json
import math

def WriteJSON(stats, fname):
    data = {}
    data['cars'] = []
    
    for c in stats:
        data['cars'].append({
            "ID" : c["ID"],
            "start_frame" : c["start_frame"],
            "end_frame" : c["end_frame"],
            "start_coord" : c["start_coord"].tolist(),
            "end_coord" : c["end_coord"].tolist(),
            "speed" : c["speed"],
            "lane" : c["lane"]
            # "vectors" : c["vectors"],
            # "angles" : c["angles"]
            # "lin_reg_slope" : c["lin_reg_slope"],
            # "lin_reg_inter" : c["lin_reg_inter"]
        })
    data["average_speed"] = np.mean([ c["speed"] for c in stats])
    
    with open(fname, "w+") as f:
        json.dump(data, f, indent = 4)

def WriteStats(stats, fname):
    lanes = {}
    with open(fname, "w+") as f:
        for c in stats:
            f.write("\n \n Car : " + str(c["ID"]) )
            f.write("\n Start frame: "+ str(c["start_frame"])+ " End frame: "+ str(c["end_frame"]))
            f.write("\n Start coord: "+ str(c["start_coord"])+ " End coord: "+ str(c["end_coord"]))
            f.write("\n Speed: "+ str(c["speed"]))
            f.write("\n Lane: "+ str(c["lane"]))
            # f.write("\n Vectors: "+ str(c["vectors"]))
            # f.write("\n Angles: "+ str(c["angles"]))
            # f.write("\n Linear Regression Slope: "+ str(c["lin_reg_slope"]))
            # f.write("\n Linear Regression Intercept: "+ str(c["lin_reg_inter"]))
            if c["lane"] not in lanes.keys():
                lanes[c["lane"]] = [c["speed"]]
            else:
                lanes[c["lane"]].append(c["speed"])
        for l in lanes:
            f.write("\n \n Lane " + str(l) + " speed: " + str(np.mean(lanes[l])) + " m/s")
        avg_speed = np.mean([ c["speed"]  for c in stats ])
        f.write("\n \n Average speed: " + str(avg_speed) + " m/s")
    return None

def ImageAnalytics(trackableObjects, VIDEO_FPS, DIST_PER_PIXEL):
    stats = []  
    
    for key, obj in trackableObjects.items():

        traj = obj.trajectory
        
        # store all computed statistics for this object
        car = {}
        car["ID"] = obj.objectID    

        # duration the particular object was tracked
        car["start_frame"] = traj[0][0]
        car["end_frame"] = traj[-1][0]
        frame_len = car["end_frame"] - car["start_frame"]

        time = 1/VIDEO_FPS * frame_len

        # distance travelled by the particular object
        car["start_coord"] = traj[0][1]
        car["end_coord"] = traj[-1][1]
        
        pixel_dist = np.sqrt( (car["start_coord"][0] - car["end_coord"][0])**2 + (car["start_coord"][1] - car["end_coord"][1])**2  )

        distance = DIST_PER_PIXEL * pixel_dist      

        #speed = distance / time if time >0 else False      
        if (time >0):
            speed = distance/time
        else:
            speed = False

        car["speed"] = speed
        print("Speed of car ", car["ID"], " is ", car["speed"] , "m/s")
        stats.append(car)       


    avg_speed = np.mean([ c["speed"] for c in stats if c["speed"] is not False ])
    print("Average speed is ", avg_speed, " m/s")

    return stats

def linear_reg(x, y):
    xbar = np.mean(x)
    ybar = np.mean(y)
    numer = 0
    denom = 0
    for i in range(len(x)):
        numer += (x[i] - xbar)*(y[i]-ybar)
        denom += (x[i] - xbar)**2
    m = numer/denom
    b = ybar - (m*xbar)
    return (m, b)

def regressionlaneFinder(trackableObjects, VIDEO_FPS, DIST_PER_PIXEL):
    stats = []  
    lanes = {}  

    for key, obj in trackableObjects.items():

        traj = obj.trajectory
        x_vals = []
        y_vals = []
        
        # store all computed statistics for this object
        car = {}
        car["ID"] = obj.objectID    

        # duration the particular object was tracked
        car["start_frame"] = traj[0][0]
        car["end_frame"] = traj[-1][0]

        #Note that full trajectory is in traj!

        frame_len = car["end_frame"] - car["start_frame"]
        time = 1/VIDEO_FPS * frame_len

        # distance travelled by the particular object
        car["start_coord"] = traj[0][1]
        car["end_coord"] = traj[-1][1]

        if(car["start_coord"][0] < car["end_coord"][0]):
            car["direction"] = 'right'
        else:
            car["direction"] = 'left'
        
        pixel_dist = np.sqrt( (car["start_coord"][0] - car["end_coord"][0])**2 + (car["start_coord"][1] - car["end_coord"][1])**2)
        distance = DIST_PER_PIXEL * pixel_dist  
        speed = distance / time if time >0 else False       
        car["speed"] = speed

        car["lane"] = int(((car["start_coord"][1] + car["end_coord"][1])//2)/100) - 3
        if car["lane"] not in lanes.keys():
            lanes[car["lane"]] = [car["ID"]]
        else:
            lanes[car["lane"]].append(car["ID"])

        stats.append(car)   
        for t in traj:
            x_vals.append(t[1][0])
            y_vals.append(t[1][1])
        m, b = linear_reg(x_vals, y_vals)
        car["lin_reg_slope"] = m
        car["lin_reg_inter"] = b

    avg_speed = np.mean([ c["speed"] for c in stats if c["speed"] is not False ])
    print("Average speed is ", avg_speed, " m/s")

    newlist = sorted(stats, key=lambda car: car['start_coord'][1]) 
    twolist = sorted(stats, key=lambda car: car['lane']) 

    for car in newlist:
        print("Car ", car["ID"], " is moving ", car["direction"])
    
    print(lanes)

    return stats
