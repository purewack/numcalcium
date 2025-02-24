import array_nat
def cc():
    print('callback')

print(array_nat.test(cc))
print(array_nat.test(cc, [2],48000))
print(array_nat.test(cc, [2]))
print(array_nat.test(cc, (32,33),48000))
print(array_nat.test(cc, (32,None),48000))
