__author__ = 'Pravjot'


class AB_beacons:
    # BEACON LOCATIONS (ROTATION IN DEGREES, APPROXIMATE DISTANCE IN FEET)
    BEACON_ROTATION_LIST = [
        90,
        -90,
        180
        ]

    def __init__(self):
        self.currentID = 0
        self.nextID = 1
        self.numBeacons = len(self.BEACON_ROTATION_LIST)

    def next_beacon(self):
        if self.currentID < self.numBeacons-1:
            self.currentID = self.currentID + 1

            if self.nextID < self.numBeacons-1:
                self.nextID = self.nextID + 1
            else:
                self.nextID = self.numBeacons-1
        else:
            self.currentID = self.numBeacons-1

    def beacon_info(self, next_ID):
        return self.BEACON_ROTATION_LIST[next_ID]

class beaconLocation:

    def __init__(self, x, y, z):
        self.x =  x
        self.y = y
        self.z = z

class marker:
    #width of the beacon in cm
    KNOWN_WIDTH = 17.85

    #Mininmum distance drone has to be to trigger next beacon (in inches)
    MIN_DISTANCE_BEACON = 20

    def __init__(self, x, y, r):
        self.x = x;
        self.y = y;
        self.r = r;
