import sys;
for path in sys.path:
    print(path)

import numpy as np
print("Numpy version: " + np.__version__)

with open('output.txt', 'w') as file:
    file.write(' '.join(map(str, np.array([1, 2, 3]))))