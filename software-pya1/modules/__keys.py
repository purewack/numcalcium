import esp32
import __ulpio
from machine import mem32

class Keys:

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
        return self.raw() & (1<<check)


    def isAnyDown(self):
        return self.__bdown() > 0

    def isAnyUp(self):
        return self.__bup() > 0
    
    def isDown(self, key, reset=False):
        r = self.__bdown() & (1<<key)
        if(reset): self.clearDown(key)
        return r

    def isUp(self, key, reset=False):
        r = self.__bup() & (1<<key)
        if(reset): self.clearUp(key)
        return r


    def getAllDown(self):
        k = self.__bdown()
        keys = []
        for i in range(20):
            if((1<<i) & k):
                keys.append(i)
        self.clearAllDown()
        return keys

    def getAllUp(self):
        k = self.__bup()
        keys = []
        for i in range(20):
            if((1<<i) & k):
                keys.append(i)
        self.clearAllUp()
        return keys

    def getNextDown(self):
        k = self.__bdown()
        for i in range(20):
            if((1<<i) & k):
                self.clearDown(i)
                return i
        
        return None

    def getNextUp(self):
        k = self.__bup()
        for i in range(20):
            if((1<<i) & k):
                self.clearUp(i)
                return i
        return None




    def clearDown(self, key):
        r = self.__bdown()
        r &= ~ (1 << key)
        self.__bdown(r)

    def clearUp(self, key):
        r = self.__bup()
        r &= ~ (1 << key)
        self.__bup(r)

    def clearAll(self):
        self.clearAllUp()
        self.clearAllDown()
        self.turns()

    def clearAllDown(self):
        self.__bdown(0)

    def clearAllUp(self):
        self.__bup(0)



    def turns(self,invert=False):
        t = self.__rw('turns')
        if(invert):
            t = -t
        self.__rw('turns',0) 
        return t
    
    def shouldHome(self, value = None):
        if value == None:
            return self.__rw('bhome')
        else:
            self.__rw('bhome',value)

    def wasHomeRequested(self):
        if self.__rw('bhome'):
            self.__rw('bhome',0)
            return True
        return False
    

    def raw(self):
        return self.__rw('bscan')
    
    def raw_home(self):
        return self.__rw('bok')
    
    def raw_turns(self):
        return self.__rw('turns')


    def __rw(self,key,val=None):
        if(not val == None):
            mem32[__ulpio.data['symbols'][key]] = val
        else:
            return mem32[__ulpio.data['symbols'][key]]
    
    def __bup(self,val=None):
        return self.__rw('bup',val)
        
    def __bdown(self,val=None):
        return self.__rw('bdown',val)
        
        
    def __driver_load(self):
        self._u.load_binary(__ulpio.data['binary'])
        self._u.run()
        i = self.__rw('ulp_tick')
        while i == self.__rw('ulp_tick'):
            continue

    def __driver_mux_period(self,period):
        self._u.set_wakeup_period(0,period)

    def __init__(self, period = None):
        self._u = esp32.ULP_RV()
        self.__driver_load()
        if(period):
            self.__driver_mux_period(period)