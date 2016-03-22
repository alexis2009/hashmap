import random, string

file_in = open('in', 'w')
file_out = open('out', 'w')

d = {}

for i in xrange(100000):
    key = ''.join(random.choice(string.ascii_uppercase + string.digits)   \
        for _ in range(random.randint(1, 9)))
    value = ''.join(random.choice(string.ascii_uppercase + string.digits) \
        for _ in range(random.randint(1, 20)))
    file_in.write('+ ' + key + ' ' + value + '\n')
    d[key] = value

items = d.items()
random.shuffle(items)
for key, value in items:
    file_in.write(str(key) + '\n')
    file_out.write(str(value) + '\n')
    file_in.write('- ' + str(key) + '\n')
    file_in.write(str(key) + '\n')

    random_str = ''.join(random.choice(string.ascii_uppercase + string.digits) \
        for _ in range(random.randint(1,9)))
    if random_str not in d:
        file_in.write(str(random_str) + '\n')
        file_in.write('- ' + str(random_str) + '\n')
