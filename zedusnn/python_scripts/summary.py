#!/usr/bin/env python
import os
import shutil
import hashlib
import Tkinter, tkFileDialog, tkMessageBox
import sys

root = Tkinter.Tk()

organisms_directory = ""
organisms_directory = tkFileDialog.askdirectory(parent=root,initialdir=u"C:/",title=u'Please select organisms directory')

sessions = {}

for subdir, dirs, files in os.walk(organisms_directory):
  for file in files:
      if not ".org" in file:
        continue
      s = file.split('_')
      session_num = int(s[3])
      print "File=%s" % file
      print "session num=%d" % session_num
      print " "
      organism_num = int(s[4])
      s2 = s[5].split('.')
      num_str = s2[0]+"."+s2[1]
      score = float(num_str)
      print "[%d]" % score
      if session_num not in sessions:
        sessions[session_num] = {}
      sessions[session_num][organism_num] = [float(s[-2]),score] # [time,score]

sorted_sessions = {}
for key in sessions.keys():
  sorted_sessions[key] = sorted(sessions[key].items())
          
#create the csv
f = open(organisms_directory+"/summary.csv","w")
for key in sessions.keys():
  org_dict = sessions[key]
  for x in sorted_sessions[key]:
    line = "%.3f, %.3f\n" % (x[1][0], x[1][1])
    print line,
    f.write(line)

f.close()
