__author__ = 'Pravjot'


class AB_beacons:
    # BEACON LOCATIONS (ROTATION IN DEGREES, APPROXIMATE DISTANCE IN FEET)
    BEACON_LIST = [
        (90, 10),
        (90, 5),
        (180, 15)
    ]
    def __init__(self):
        self.currentID = 0
        self.nextID = 1
        self.numBeacons = len(self.BEACON_LIST)
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
        return self.BEACON_LIST[next_ID]

class marker:
    KNOWN_WIDTH = 11;

    def __init__(self, x, y, r):
        self.x = x;
        self.y = y;
        self.r = r;
