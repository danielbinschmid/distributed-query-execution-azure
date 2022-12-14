import csv 
import requests
from os.path import basename

filename = './files/filelist.csv'


with open(filename, 'r') as csvfile:
    datareader = csv.reader(csvfile)
    for row in datareader:
        url = row[0]
        r = requests.get(url, allow_redirects=True)
        open("files/"+basename(url), 'wb').write(r.content)
