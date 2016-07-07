# -*- coding: utf-8 -*-
"""
Created on Thu Jun 30 15:01:24 2016

@author: tfinney
"""

import urllib2
import HTMLParser
from xml.etree import ElementTree as ET
from bs4 import BeautifulSoup
import requests
import pandas as pd
import csv
import os
import io


url="http://marblerye.org/cgi-bin/ninjavisit?print=year"

#new=urllib2.urlopen(url)
#response=new.read()

r=requests.get(url)
data=r.text
soup=BeautifulSoup(data)

table=soup.find_all('table')[0]


rows=table.find_all('tr')[1:]

data={
    'country' :[],
    'region' :[],
    'city' :[],
    'ip' :[],
    'last visit' :[]
}

for row in rows:
    cols=row.find_all('td')
    data['country'].append(cols[0].get_text())
    data['region'].append(cols[1].get_text())
    data['city'].append(cols[2].get_text())
    data['ip'].append(cols[3].get_text())
    data['last visit'].append(cols[4].get_text())
    
#spyData=pd.DataFrame(data,)

#spyData.to_csv("WindNinja_spy.csv"," \n")

puce=io.open('wnio.csv','w')
puce.write(u'country,region,city,ip,last visit\n')






for i in range(len(data['country'])):
#    print data['country'][i][1:].rstrip(),",",data['region'][i][1:].rstrip(),",",data['city'][i][1:].rstrip(),",",data['ip'][i][1:].rstrip(),",",data['last visit'][i][1:].rstrip()
    puce.write((data['country'][i][1:].rstrip()))
    puce.write(u',')
    puce.write((data['region'][i][1:].rstrip()))
    puce.write(u',')
    puce.write((data['city'][i][1:].rstrip()))
    puce.write(u',')
    puce.write((data['ip'][i][1:].rstrip()))
    puce.write(u',')
    puce.write((data['last visit'][i][1:].rstrip()))
    puce.write(u',\n')    
    
    
    
newpuce=io.open('pureip','w')
for i in range(len(data['country'])):
    newpuce.write((data['ip'][i][1:].rstrip()))
    newpuce.write(u'\n')





    
    
    
    



