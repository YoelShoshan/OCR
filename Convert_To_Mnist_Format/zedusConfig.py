#!/usr/bin/env python
import os

fully_local = False #used when releasing a standalone kit, where all the needed binary/files are next to the main executable.
sort_left_to_right = True

if os.name == 'posix':
    home_path = u"/home/ubuntu/zedusOCR" #for the server
    #home_path = u"/home/yoel/google_code" #for home dev
else:
    home_path = u"C:/google_code"

path_to_ocr_project = home_path+"/dooake/OCR"


path_to_CPUMedian = ""
path_to_BoxMerger = ""
path_to_BoxOrganizer = ""
path_to_BoxFindWords = ""

if os.name == 'posix':
    path_to_CPUMedian = path_to_ocr_project+"/CPUMedian/CPUMedian/release/CPUMedian"
    path_to_BoxMerger = path_to_ocr_project+"/BoxOrganizer/release/BoxMerger"
    path_to_BoxOrganizer = path_to_ocr_project+"/BoxOrganizer/release/BoxOrganizer"
    path_to_BoxFindWords = path_to_ocr_project+"/BoxOrganizer/release/BoxFindWords"
else:
    path_to_CPUMedian = path_to_ocr_project+"/CPUMedian/Release/CPUMedian.exe"
    path_to_BoxMerger = path_to_ocr_project+"/BoxOrganizer/Release/BoxMerger.exe"
    path_to_BoxOrganizer = path_to_ocr_project+"/BoxOrganizer/Release/BoxOrganizer.exe"
    path_to_BoxFindWords = path_to_ocr_project+"/BoxOrganizer/Release/BoxFindWords.exe"


#path_to_ocr_project = "C:/blah_blah" #update for windows

#zedusnn_path = "/home/yoel/google_code/zedusnn/release"
zedusnn_path = home_path+"/zedusnn/release"
#zedusnn_path = r"C:\google_code\zedusnn\Release"

if os.name == 'posix':
    zedusnn_file = "libzedusnn.so"
else:
    zedusnn_file = "zedusNN.dll"

support_gui = False






