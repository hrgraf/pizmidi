# testing linmap up and down

def linmap(x, in_min, in_max, out_min, out_max):
    if x < in_min:
        return out_min
    return (x - in_min) * (out_max - out_min) // (in_max - in_min) + out_min



for x in range(10):
    print(x, linmap(x, 0, 9, 5, 31))
print()
    
for x in range(32):
    print(x, linmap(x, 5, 32, 0, 10))
print()
