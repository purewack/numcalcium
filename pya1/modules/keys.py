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

    KEY_EQ   = 4
    KEY_PLUS = 7

    def resetDown(self, key):
        r = self._u.read(self._u.VAR_BDOWN)
        r &= ~ (1 << key)
        self._u.write(self._u.VAR_BDOWN, r)

    def resetUp(self, key):
        r = self._u.read(self._u.VAR_BUP)
        r &= ~ (1 << key)
        self._u.write(self._u.VAR_BUP, r)

    def isAnyDown(self):
        return self._u.read(self._u.VAR_BDOWN) > 0

    def isAnyUp(self):
        return self._u.read(self._u.VAR_BUP) > 0

    def getFirstDown(self):
        k = self._u.read(self._u.VAR_BDOWN)
        for i in range(20):
            if((1<<i) & k):
                self.resetDown(i)
                return i
        
        return None

    def getFirstUp(self):
        k = self._u.read(self._u.VAR_BUP)
        for i in range(20):
            if((1<<i) & k):
                self.resetUp(i)
                return i
        return None

    def isDown(self, key, reset=True):
        r = self._u.read(self._u.VAR_BDOWN) & (1<<key)
        if(reset): self.resetDown(key)
        return r

    def isUp(self, key, reset=True):
        r = self._u.read(self._u.VAR_BUP) & (1<<key)
        if(reset): self.resetUp(key)
        return r

    def getAll(self):
        return self._u.read(self._u.VAR_BSCAN)

    def clearAll(self):
        self._u.write(self._u.VAR_BDOWN,0)
        self._u.write(self._u.VAR_BUP,0)
