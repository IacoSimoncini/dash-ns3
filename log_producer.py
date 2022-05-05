
import os
import math
import pandas as pd
from pathlib import Path
import warnings
warnings.filterwarnings("ignore")


#Conversione file di output in dataframe

a_file = open("output.txt", "r")
lines = a_file.readlines()
a_file.close()
pos=0
for l in lines:
    if "dim_packets" in l:
        break
    pos=pos+1
for i in range(pos):
    del lines[0]
pos=len(lines)-1
"""
Codice che cancella le eventuali linee di errore stampate nell'output.
Da inserire se le righe in più non permettono di generare il dataframe,
se invece viene generato, come sembra dai test effettuati, le righe superflue 
vengono comunque filtrate
while True:
    if lines[pos].startswith("LOG"):
        break
    del lines[pos]
    pos=pos-1
"""
ind=lines[0].replace("\n","").split(",")
del lines[0]
log_segmento = pd.DataFrame([l.replace("\n","").split(",") for l in lines],columns=ind)

# creazione altri dataframe con i file nella directory files e conversione del formato dei valori
directory = Path(os.path.abspath("output.txt")).parent.absolute()
li = []
i=0
for filename in sorted(os.listdir(directory)):
    if 'Stats' in filename:
        df = pd.read_csv(filename, delimiter='\t', header=0, index_col=False)
        li.append(df)

log1 = pd.DataFrame()
log2 = pd.DataFrame()
rsrp = li[2]
mcs = li[0]
pdcp = li[1]

mcsTb1 = mcs[['% time', 'mcsTb1', 'IMSI', 'sizeTb1']].astype(float)
mcsTb1['IMSI'] = mcsTb1['IMSI'].astype(int)
mcsTb1['IMSI'] = mcsTb1['IMSI'].apply(lambda x: x - 1)
mcsTb1.rename(columns = {'sizeTb1': 'mac_throughput'}, inplace = True)

rsrp = rsrp[['% time', 'rsrp', 'IMSI', 'sinr']].astype(float)
rsrp['IMSI']==rsrp['IMSI'].astype(int)
rsrp['IMSI'] = rsrp['IMSI'].apply(lambda x: x - 1)

pdcp['% time'] = pdcp['end']
pdcp = pdcp[['% time', 'CellId', 'IMSI']].astype(float)
pdcp[['CellId', 'IMSI']] = pdcp[['CellId', 'IMSI']].astype(int)
pdcp['IMSI'] = pdcp['IMSI'].apply(lambda x: x - 1)
pdcp['CellId'] = pdcp['CellId'].apply(lambda x: x - 1)

nodes = mcsTb1['IMSI'].unique()

log1 = log_segmento[log_segmento['index'] == 'LOG1']
log1.drop('index', inplace=True, axis=1)
log1.drop('est_bit_rate', inplace=True, axis=1)
log1=log1.astype(float)
log1[['dim_packets','count_packet','node_id','new_bit_rate','old_bit_rate']]=log1[['dim_packets','count_packet','node_id','new_bit_rate','old_bit_rate']].astype(int)
log2 = log_segmento[log_segmento['index'] == 'LOG2']
log2.drop('index', inplace=True, axis=1)

colmcs = []
colrsrp = []
colpdcp = []
logs = []

#dividere i dataframe per numero del nodo
for i in nodes:
    colmcs.append(mcsTb1[mcsTb1['IMSI'] == i])
    colrsrp.append(rsrp[rsrp['IMSI'] == i])
    colpdcp.append(pdcp[pdcp['IMSI'] == i])
    logs.append(log1[log1['node_id'] == i])


#Merge dei dataframe per ogni nodo
final = []
for i in range(len(nodes)):
    temp = pd.DataFrame()
    
    colmcs[i]['% time'] = colmcs[i]['% time'].apply(lambda x: x * 2)
    colmcs[i]['% time'] = colmcs[i]['% time'].apply(lambda x: math.trunc(x))
    temp['mac_throughput'] = colmcs[i].groupby(['% time']).sum()['mac_throughput']
    temp['mac_throughput'] = temp['mac_throughput'].apply(lambda x: x * 16)
    temp['mode_mcs'] = colmcs[i].groupby(['% time'])['mcsTb1'].agg(pd.Series.mode).to_frame()
    temp['median_mcs'] = colmcs[i].groupby(['% time'])['mcsTb1'].agg(pd.Series.median).to_frame()
    temp['min_mcs'] = colmcs[i].groupby(['% time']).min()['mcsTb1']
    temp['max_mcs'] = colmcs[i].groupby(['% time']).max()['mcsTb1']

    colrsrp[i]['% time'] = colrsrp[i]['% time'].apply(lambda x: x * 2)
    colrsrp[i]['% time'] = colrsrp[i]['% time'].apply(lambda x: math.trunc(x))
    temp['mean_rsrp'] = colrsrp[i].groupby(['% time']).mean()['rsrp']
    temp['median_rsrp'] = colrsrp[i].groupby(['% time'])['rsrp'].agg(pd.Series.median).to_frame()
    temp['mean_sinr'] = colrsrp[i].groupby(['% time']).mean()['sinr']

    colpdcp[i]['% time'] = colpdcp[i]['% time'].apply(lambda x: x * 2)
    colpdcp[i]['% time'] = colpdcp[i]['% time'].apply(lambda x: math.trunc(x))
    temp['CellId'] = colpdcp[i].groupby(['% time']).max()['CellId']
    temp['CellId']=temp['CellId'].fillna(method='ffill').astype('Int64')

    logs[i]['time'] = logs[i]['time'].apply(lambda x: x * 2)
    logs[i]['time'] = logs[i]['time'].apply(lambda x: math.trunc(x))
    logs[i] = logs[i].set_index('time')
    
    final.append(logs[i].join(temp).reset_index())
    

    
#Concatenazione dataframe ordinati per tempo e produzione dei csv
result = pd.concat(final)
result = result.sort_values(by=['time'])
result['time']= result['time'].apply(lambda x: float(x / 2.0))
result.to_csv('output.csv', index=False)
log2.to_csv('log_segmento.csv', index=False)