from contextlib import redirect_stdout
import io
import inspect
import os

class Testing:
    tests = 0
    passed = 0
    def __init__(self):
        self.tests = 0
        self.passed = 0
        caller_frame = inspect.stack()[1]
        self._creation_file = caller_frame.filename.split('/')[-1]
        print(f"\n\n========= Testing start  =========\n[ {self._creation_file} ] \n")
        
    def __del__(self):
        s = (f"\n\033[39m \n========= Test results =========\n[ {self._creation_file} ]\n")
        s += ('\033[31m' if not self.passed == self.tests else '\033[32m')
        s += (f"Failed:     [ {self.tests-self.passed} ]\033[39m\n")
        s += (f"Total:      [ {self.tests - (self.tests-self.passed)}/{self.tests} ] {100*(self.tests - (self.tests-self.passed))/self.tests}%\n")
        print(s)
    
    def eq(self,tester,value,heading=None):
        self.tests += 1
        if(not tester == value):
            print(f"    ({self.tests}) \t- ❌ {heading if heading else 'Test'} Failed: \n\033[31m{tester}\033[39m != \033[32m{value}\033[39m\n")
        else:
            print(f"    ({self.tests}) \t- ✅ {heading if heading else 'Test'}  Ok: \033[36m{tester}\033[39m == \033[36m{value}\033[39m")
            self.passed += 1
    
    def neq(self,tester,value,heading=None):
        self.tests += 1
        if(tester == value):
            print(f"    ({self.tests}) \t- ❌ {heading if heading else 'Test'} NEQ Failed: \n\033[31m{tester}\033[39m == \033[32m{value}\033[39m\n")
        else:
            print(f"    ({self.tests}) \t- ✅ {heading if heading else 'Test'} NEQ Ok: \033[36m{tester}\033[39m != \033[36m{value}\033[39m")
            self.passed += 1
    
    def nex(self,tester,*args,heading=None):
        self.tests += 1
        try:
            tester(*args)
            print(f"    ({self.tests}) \t- ✅ {heading if heading else 'Test'}  \033[36mtry ok\033[39m")
            self.passed += 1
        except Exception as e:
            print(f"    ({self.tests}) \t- ❌ {heading if heading else 'Test'}  \033[31mtry failed with exception:-> {e} <-\033[39m\n")
    
    def ex(self,tester,*args,heading=None):
        self.tests += 1
        try:
            r = tester(*args)
            print(f"    ({self.tests}) \t- ❌ {heading if heading else 'Test'}  \033[31mtry completed unexpectedly with result: {r}\033[39m\n")
        except Exception as e:
            print(f"    ({self.tests}) \t- ✅ {heading if heading else 'Test'}  \033[36mtry failed as expected with exception:-> {e} <-\033[39m")
            self.passed += 1
            
    def instance(self,tester,value,heading):
        self.eq(type(tester),type(value),heading)
    
    def eq_array(self, tester, value, heading=None):
        self.eq(self.print_to_string(tester), self.print_to_string(value), heading)
    
    def eq_array_float(self, tester, value, heading=None):
        tv = ['{0:.3f}'.format(f) for f in tester]
        xv = ['{0:.3f}'.format(f) for f in value]
        self.eq(tv,xv, heading)
        
    def eq_float(self, tester, value, heading=None):
        tv = '{0:.3f}'.format(tester)
        xv = '{0:.3f}'.format(value)
        self.eq(tv,xv, heading)
    
    def neq_array(self, tester, value, heading=None):
        self.neq(self.print_to_string(tester), self.print_to_string(value), heading)
    
    def print_to_string(self, v):
        with io.StringIO() as buf, redirect_stdout(buf):
            print(v)
            out = buf.getvalue()
            out = out.strip('\n\r')
        return out
