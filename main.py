import numpy as np


a = np.arange(15).reshape(3, 5)
# get numpy info
print(a)
print(a.shape)
print(a.ndim)
print(a.dtype.name)
print(a.itemsize)
print(type(a))

b = np.array([6, 7, 8])
b = b**2 # get square
print(b)

print("Done Cameron 1")

import torch
x = torch.rand(5, 3)
print(x)

print("Done Cameron 2")
