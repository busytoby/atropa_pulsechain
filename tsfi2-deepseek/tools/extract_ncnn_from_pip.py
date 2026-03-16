import ncnn
import os
import sys

print("[INFO] Attempting to map NCNN from pip package...")
try:
    print(ncnn.__file__)
except Exception as e:
    print(e)
