import esp32

class Keys:
    
    _u = None

    def __init__(self):
        self._u = esp32.ULP()
        self._u.pause()
        self._u.run_embedded()
        self._u.resume()

    KEY_SHIFT = 0

    KEY_DOT  = 2
    KEY_0    = 1
    KEY_1    = 4
    KEY_2    = 5
    KEY_3    = 6
    KEY_4    = 8
    KEY_5    = 9
    KEY_6    = 10
    KEY_7    = 12
    KEY_8    = 13
    KEY_9    = 14

    KEY_F1    = 16
    KEY_F2    = 17
    KEY_F3    = 18

    KEY_SIDE_A  = 3
    KEY_SIDE_B  = 7
    KEY_SIDE_C  = 11
    KEY_SIDE_D  = 15
    KEY_SIDE_E  = 19

    def isAnyDown(self):
        return self._u.read(self._u.VAR_BDOWN) > 0

    def isAnyUp(self):
        return self._u.read(self._u.VAR_BUP) > 0

    def getAllDown(self):
        k = self._u.read(self._u.VAR_BDOWN)
        keys = []
        for i in range(20):
            if((1<<i) & k):
                keys.push(i)
        self.clearAllDown()
        return None

    def getAllUp(self):
        k = self._u.read(self._u.VAR_BUP)
        keys = []
        for i in range(20):
            if((1<<i) & k):
                keys.push(i)
        self.clearAllUp()
        return None

    def getNextDown(self):
        k = self._u.read(self._u.VAR_BDOWN)
        for i in range(20):
            if((1<<i) & k):
                self.clearDown(i)
                return i
        
        return None

    def getNextUp(self):
        k = self._u.read(self._u.VAR_BUP)
        for i in range(20):
            if((1<<i) & k):
                self.clearUp(i)
                return i
        return None

    def isDown(self, key, reset=False):
        r = self._u.read(self._u.VAR_BDOWN) & (1<<key)
        if(reset): self.clearDown(key)
        return r

    def isUp(self, key, reset=False):
        r = self._u.read(self._u.VAR_BUP) & (1<<key)
        if(reset): self.clearUp(key)
        return r

    def clearDown(self, key):
        r = self._u.read(self._u.VAR_BDOWN)
        r &= ~ (1 << key)
        self._u.write(self._u.VAR_BDOWN, r)

    def clearUp(self, key):
        r = self._u.read(self._u.VAR_BUP)
        r &= ~ (1 << key)
        self._u.write(self._u.VAR_BUP, r)

    def clearAll(self):
        self.clearAllUp()
        self.clearAllDown()

    def clearAllDown(self):
        self._u.write(self._u.VAR_BDOWN,0)

    def clearAllUp(self):
        self._u.write(self._u.VAR_BUP,0)

    def getRaw(self):
        return self._u.read(self._u.VAR_BSCAN) & 0xfffff

