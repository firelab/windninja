#!/usr/bin/env python

import subprocess
import os

import smtplib
from email.MIMEText import MIMEText

import datetime

cfg = '/home/natalie/src/windninja/nightly_tests/cfg/'
logfile = '/home/natalie/src/windninja/nightly_tests/nightlyTests.log'

#=============================================================================
#        Open a log file
#=============================================================================
startTime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
log = open(logfile, 'w')

wn = subprocess.Popen(["export WINDNINJA_DATA=/home/natalie/src/windninja/windninja/data"], shell = True, stdout=subprocess.PIPE)

#=============================================================================
#         Run the cfgs
#=============================================================================
for f in os.listdir(cfg):
    wn = subprocess.Popen(["WindNinja_cli " + cfg + f], shell = True, stdout=subprocess.PIPE)
    out, err = wn.communicate()
    if wn.returncode != 0:
        log.write('Time = %s, %s: Failed, stdout = %s, stderr = %s \n' % (startTime, f, out, err))

log.close()

#=============================================================================
#        Send email if there were errors
#=============================================================================
if os.stat(logfile).st_size != 0:
    fp = open(logfile, 'r')
    msg = MIMEText(fp.read())
    fp.close()
    body = msg

    fromaddr = "sendingaddress@something.com"
    recipients = ['address1@something.com', 'address2@something.com']
    msg['From'] = fromaddr
    msg['To'] = ", ".join(recipients)
    msg['Subject'] = "WindNinja nightly tests failed"
      
    server = smtplib.SMTP('smtp.gmail.com', 587)
    server.starttls()
    server.login(fromaddr, "password")
    text = msg.as_string()
    server.sendmail(fromaddr, recipients, text)
    server.quit()

