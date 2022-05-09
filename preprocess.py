import tqdm
import os
import numpy as np

# Pre-process DlRsrpSinrStats.txt 
arr = []
check = np.zeros(200)
step = 0.1
first = True
file2 = open('processedSinr.txt', "w")

with tqdm.tqdm(total=os.path.getsize("DlRsrpSinrStats.txt")) as pbar:
    with open("DlRsrpSinrStats.txt", "r", encoding="utf8") as infile:
        for line in infile:
            pbar.update(len(line))
            arr = line.split("\t")
            if first:
                first = False
                file2.write(line)
            else:
                try:
                    if float(arr[0]) >= step:
                        check = np.zeros(200)
                        step = step + 0.1
                    if check[int(arr[2]) - 1] == 0:
                        file2.write(line)
                        check[int(arr[2]) - 1] = 1
                except Exception as e:
                    print(e)

file2.close()
