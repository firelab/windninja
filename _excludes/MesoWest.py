# -*- coding: utf-8 -*-
"""
Created on Tue Jul 05 15:42:52 2016


@author: tfinney


This is a simple program that performs the grabbing of weather data from the MESOWEST API. 



"""
import urllib2
import json
import csv
import numpy
import varlist
import io


dtoken="33e3c8ee12dc499c86de1f2076a9e9d4"
r"""
A token is necessary to get any data from the MesoWest API, use this one if you don't have one!
"""

variables=varlist.varlist

def helper():
    r"""
            Helper prints the following::
            
            MesoWest.py has the following functions: 
            
            URL constructors            
            
            singlebuilder()
            multibuilder()
            recent()
            stationradiusbuilder()
            radiuslatest()
            latlonradiusbuilder()
            bboxbuilder()
            
            Data Readers:
            
            json_interpret()
            
            writers
            
            writeToCSV()
            
            Information: 
            
            variables
            
            dtoken
            
            get_observation()

            get_station_info()

            HOW TO USE:

            1) create url with a URL constructor
            2) read data with json_interpret
            3) look at data with get_ functions
            4) write data to csv with writetoCSV()            
            
            
            
        """
    print """
            MesoWest.py has the following functions: 
            
            URL constructors            
            
            singlebuilder()
            multibuilder()
            recent()
            stationradiusbuilder()
            radiuslatest()
            latlonradiusbuilder()
            bboxbuilder()
            
            Data Readers:
            
            json_interpret()
            
            writers
            
            writeToCSV()
            
            Information: 
            
            variables-lists all known variables for the API
            
            dtoken
            
            get_observation()

            get_station_info()

            HOW TO USE:

            1) create url with a URL constructor
            2) read data with json_interpret
            3) look at data with get_ functions
            4) write data to csv with writetoCSV()            
            
            
            
        """
    

def sand(year_0,month_0,day_0,clock_0,year_1,month_1,day_1,clock_1):
    r"""
    This is an internal function that builds a string of time, you probably won't need to use it outside of
    a builder.
    """
    
    start="&start="
    y20="2016"
    m20="05"
    d20="22"
    c20="1000"
    estartful=start+y20+m20+d20+c20
    end="&end="
    y21="2016"
    m21="05"
    d21="23"
    c21="1000"
    eendful=end+y21+m21+d21+c21
    
    startfull=start+year_0+month_0+day_0+clock_0
    endfull=end+year_1+month_1+day_1+clock_1
    
    timemainfull=startfull+endfull
    
    return timemainfull
def singlebuilder(token,station_id,svar,yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky):
    r"""
    Creates a url for a single station for a range of times.
    
    Arguments:
    ----------
    token: string
        you need a token, use MesoWest.dtoken if you don't have one
    station_id: string
        you need a station ID 
    svar:
        select the variables you want to download from a station, to find out what variables are availbale
        use MesoWest.variables(), if you want all variables type ""

    """
    eburl="http://api.mesowest.net/v2/stations/timeseries?"  
    timesand=sand(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky)
    tokfull="&token="+token
    stidfull="stid="+station_id
    svarfull="&vars="+svar
    if svar=="":
        print "urlBuilder: downloading all variables for station"
        svarfull=""
    url=eburl+stidfull+svarfull+timesand+tokfull
    return url
    
def recent(token,station_id,svar,nHours):
    r"""
    Creates a url for a single station for the a recent number of hours.
    
    Arguments:
    ----------
    token: string
        you need a token, use MesoWest.dtoken if you don't have one
    station_id: string
        you need a station ID 
    svar:
        select the variables you want to download from a station, to find out what variables are availbale
        use MesoWest.variables(), if you want all variables type ""
    nHours: 
        How many hours you want to go back from right now.
    """

    eburl="http://api.mesowest.net/v2/stations/timeseries?"  
    flat=nHours
    #timesand=sand(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky)
    flatter=60*flat
    fats=str(flatter)
    timesand="&recent="+fats
    tokfull="&token="+token
    stidfull="stid="+station_id
    svarfull="&vars="+svar
    if svar=="":
        print "urlBuilder: downloading all variables for station"
        svarfull=""
    network="1,2,81"
    networkfull="&network="+network
    #output="&output="+outputa
    url=eburl+stidfull+svarfull+timesand+tokfull
    return url
    
#def geojson(token,station_id,svar,flat):
#    eburl="http://api.mesowest.net/v2/stations/timeseries?"  
#    flatter=60*flat
#    fats=str(flatter)
#    timesand="&recent="+fats
#    tokfull="&token="+token
#    stidfull="stid="+station_id
#    svarfull="&vars="+svar
#    if svar=="":
#        svarfull=""
#    output="&output=geojson"
#    url=eburl+stidfull+svarfull+output+timesand+tokfull
#    return url
    
def multibuilder(token,station_ids,svar,yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky):
    r"""
    Creates a url for a multiple stations for a specified time peroid, use strings.
    
    Arguments:
    ----------
    token: string
        you need a token, use MesoWest.dtoken if you don't have one
    station_id: string
        you need station IDs, enter them as comma seperated in a string ie "kmso,TR266"
    svar:
        select the variables you want to download from a station, to find out what variables are availbale
        use MesoWest.variables(), if you want all variables type ""

    """    
    eburl="http://api.mesowest.net/v2/stations/timeseries?"  
    timesand=sand(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky)
    tokfull="&token="+token
    stidfull="stid="+station_ids
    svarfull="&vars="+svar
    if svar=="":
        print "urlBuilder: downloading all variables for station"
        svarfull=""
    output="&output=geojson"
    url=eburl+stidfull+svarfull+timesand+tokfull
    return url

def stationradiusbuilder(token,station_id,radius,limit,svar,yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky):
    eburl="http://api.mesowest.net/v2/stations/timeseries?"  
    timesand=sand(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky)
    tokfull="&token="+token
    stidfull="&radius="+station_id+","+radius
    svarfull="&vars="+svar
    if svar=="":
        svarfull=""
    output="&output=geojson"
    limiter="&limit="+limit
    url=eburl+stidfull+svarfull+limiter+timesand+tokfull
    return url
    
def radiuslatest(token,station_id,radius,limit,svar,flat):
    eburl="http://api.mesowest.net/v2/stations/timeseries?"  
    flatter=60*flat
    fats=str(flatter)
    timesand="&recent="+fats
    tokfull="&token="+token
    stidfull="&radius="+station_id+","+radius
    svarfull="&vars="+svar
    if svar=="":
        print "urlBuilder: downloading all variables for station"
        svarfull=""
    output="&output=geojson"
    limiter="&limit="+limit
    network="1,2"
    networkfull="&network="+network
    url=eburl+stidfull+svarfull+limiter+timesand+tokfull
    return url
    
    
def latlonradiusbuilder(token,lat,lon,radius,limit,svar,yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky):
    eburl="http://api.mesowest.net/v2/stations/timeseries?"  
    timesand=sand(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky)
    tokfull="&token="+token
    stidfull="&radius="+lat+","+lon+","+radius
    svarfull="&vars="+svar
    if svar=="":
        print "urlBuilder: downloading all variables for station"
        svarfull=""
    output="&output=geojson"
    limiter="&limit="+limit
    url=eburl+stidfull+svarfull+limiter+timesand+tokfull
    return url

def bboxbuilder(token,lat1,lon1,lat2,lon2,svar,yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky):
    box="&bbox="+lon1+","+lat1+","+lon2+","+lat2
    eburl="http://api.mesowest.net/v2/stations/timeseries?"  
    timesand=sand(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky)
    tokfull="&token="+token
    svarfull="&vars="+svar
    if svar=="":
        print "urlBuilder: downloading all variables for station"
        svarfull=""
    output="&output=geojson"
    url=eburl+box+svarfull+timesand+tokfull
    return url
    
def json_interpret(url):
    new=urllib2.urlopen(url)
    response=new.read()
    
    json_string=response
    a=json.loads(json_string)
    
    return a
    
def writeToCSV(dictData,csvName):
    r"""
    Writes a CSV of weather data for a dataset
    
    Arguments:
    ----------
    dictData:
        A dictionary of the data is required, json_interpret creates a dicitonary
    from the json data downloaded (use it)
    
    csvName: 
        NAME YOUR FILE!

    """
    lib=dictData
    datLen=len(lib['STATION'])
    with open(csvName,'wb') as csvfile:
        blue=csv.writer(csvfile,delimiter=',')
        for j in range(datLen):
            header=list()
            obsRow=list()
            dictKey=lib['STATION'][j]['OBSERVATIONS'].keys()
            keyLen=len(dictKey)
            
            obsLen=len(lib['STATION'][j]['OBSERVATIONS']['date_time'])
            stationInfo=[lib['STATION'][j]['NAME'],lib['STATION'][j]['LATITUDE'],
                         lib['STATION'][j]['LONGITUDE'],lib['STATION'][j]['TIMEZONE'],
                         lib['STATION'][j]['STID']]   
            for ex in range(keyLen):
                header.append(dictKey[ex])        
            
            blue.writerow(stationInfo)
            blue.writerow(header)
            for k in range(obsLen):
                obsRow=list()
                for i in range(keyLen):
                    obsRow.append( lib['STATION'][j]['OBSERVATIONS'][dictKey[i]][k])            
                blue.writerow((obsRow))
                
def get_observation(dictData, observation,station_number,observation_number):
    r"""
    returns data from the dictionary in a more readable format
    
    Arguments:
    ----------
    dictData: 
        the data to read in dictionary format.
    
    observation: 
        the observation you want ie "wind_speed", see MesoWest.variables 
    for more information
    
    station_number: 
        the number in the list of stations. If you download two stations,
    station 1 is the [0] dictData['STATION'][x]
    
    observation_number: 
        if you want a specific observation for a certian time. 
    for example: dictDAtap['STATION'][x]['OBSERVATIONS']['wind_speed'][z]
    
    z is the observation_number
        

    """
    obs=str(observation)+"_set_1"
    if observation_number=="all":
        ska= dictData['STATION'][int(station_number)]['OBSERVATIONS'][obs]
    else:
        ska= dictData['STATION'][int(station_number)]['OBSERVATIONS'][obs][observation_number]
    return ska
    
    
def get_station_info(dictData,station_number):
    r"""
    returns information about the station such as
    
    NAME, latitude, longitude, STID and timeZone
    
    Arguments:
    ----------
    dictData: 
        the data to read in dictionary format
    
    station_number: 
        the number in the list of stations. If you download two stations,
    station 1 is the [0] dictData['STATION'][x]

    """  
    station_info=[dictData['STATION'][station_number]['NAME'],dictData['STATION'][station_number]['LATITUDE'],
                         dictData['STATION'][station_number]['LONGITUDE'],dictData['STATION'][station_number]['TIMEZONE'],
                         dictData['STATION'][station_number]['STID']] 
    return station_info
    
    
    
    
    
    
    
    
    