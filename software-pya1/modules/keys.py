import esp32

class Keys:
    
    _u = esp32.ULP()

    def __init__(self, period = None):
        if(period):
            self.mux_period(period)
#        self._u = esp32.ULP()
#        self._u.pause()
#        self._u.run_embedded()
#        self._u.resume()

    SHIFT = 0

    DOT  = 2
    N0    = 1
    N1    = 4
    N2    = 5
    N3    = 6
    N4    = 8
    N5    = 9
    N6    = 10
    N7    = 12
    N8    = 13
    N9    = 14

    F1    = 16
    F2    = 17
    F3    = 18

    A  = 3
    B  = 7
    C  = 11
    D  = 15
    E  = 19
    
    KEY_SHIFT = SHIFT

    KEY_DOT  = DOT
    KEY_0    = N0
    KEY_1    = N1
    KEY_2    = N2
    KEY_3    = N3
    KEY_4    = N4
    KEY_5    = N5
    KEY_6    = N6
    KEY_7    = N7
    KEY_8    = N8
    KEY_9    = N8

    KEY_F1    = F1
    KEY_F2    = F2
    KEY_F3    = F3

    KEY_SIDE_A  = A
    KEY_SIDE_B  = B
    KEY_SIDE_C  = C
    KEY_SIDE_D  = D
    KEY_SIDE_E  = E

    def __eq__(self, check):
        return self.getRaw() & (1<<check)

    def isAnyDown(self):
        return self._u.read(self._u.VAR_BDOWN) > 0

    def isAnyUp(self):
        return self._u.read(self._u.VAR_BUP) > 0

    def getAllDown(self):
        k = self._u.read(self._u.VAR_BDOWN)
        keys = []
        for i in range(20):
            if((1<<i) & k):
                keys.append(i)
        self.clearAllDown()
        return keys

    def getAllUp(self):
        k = self._u.read(self._u.VAR_BUP)
        keys = []
        for i in range(20):
            if((1<<i) & k):
                keys.append(i)
        self.clearAllUp()
        return keys

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

    def mux_period(self,period):
        self._u.set_wakeup_period(period)
