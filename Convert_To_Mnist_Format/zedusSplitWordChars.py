#!/usr/bin/env python
# -*- coding: <utf-8> -*-

from PIL import Image
import ImageDraw
import ImageFont
import os
import sys
import struct
from array import array
import numpy as np
import cv2
import random
import zedusBoxes
import tkMessageBox
import math
from zedusCharacterDict import *
import subprocess
from ctypes import *
#import ctypes

save_dbg_images = False

znnLib = cdll.LoadLibrary(r"C:\google_code\zedusnn\Release\zedusNN.dll")
prob_type = c_uint(1) #enum - 1 means mnist, which actually means right now 28x28 grey-scale character classification.
znnLib.znnInit.restype = c_char_p
znnLib.znnLoadNN.restype = c_bool
znnLib.znnProcess.restype = c_char_p
znnLib.znnProcessSingleCase.restype = c_bool
znnLib.znnTrainOnDB.restype = c_bool
znnLib.znnTrainOnDB_StartFromProvidedNN.restype = c_bool
res = znnLib.znnInit(prob_type, c_bool(False))
print "Initialized zedusNN library. Result was [%s]" % res

res = znnLib.znnLoadNN(c_char_p(r"C:\session_winners\evolve_PID_4892_0x36D7931A\session_winner_PID_4892_19001_score_83.117_12.7308_In_784_Hidden_1_Hidden_Size_200_Output_124.org"))
print "Loaded NN. Result was:",
print res

images_path = u"C:/OCR/test2/"
view_factor = 5
current_resolution = (0,0)
cv2.namedWindow("s", cv2.CV_WINDOW_AUTOSIZE)

KEY_LEFT = 2424832
KEY_RIGHT = 2555904
KEY_TAB = 9
KEY_ENTER = 13
KEY_SPACE = 32


def GetKey():
    global typing_mode_hebrew
    global debug_mode
    
    #if True == debug_mode:
    #    return u"@".encode('UTF-8')
    
    #while True:        
    raw_ch = cv2.waitKey()
    ch = 0xFF & raw_ch        
    return raw_ch

def draw_square(im, (x1,y1),(x2,y2)):
    global view_factor
    #cv2.rectangle(img, pt1, pt2, color[, thickness[, lineType[, shift]]]) → None        
    #cv2.rectangle(im, (x1,y1), (x1+view_factor-1, y2+view_factor-1), (0,255,0), thickness=-1)
        
    """cv2.line(im, (x1,y1), (x1, y2), (0,255,0))
    cv2.line(im, (x2,y1), (x2, y2), (0,0,255))    
    cv2.line(im, (x1,y1), (x2, y1), (0,255,0))
    cv2.line(im, (x1,y2), (x2, y2), (0,255,0))
    """
    
    for j in xrange(y1,y2+1):
        for i in xrange(x1,x2+1):
            val = im[j][i]
            im[j][i] = (val[0],max(255,val[1]+50),val[2])
    
    """
    #left line
    for j in xrange(y1,y2+1):
        val = im[j][x1]
        im[j][x1] = (val[0],max(255,val[1]+50),val[2])
        
    #right line
    for j in xrange(y1,y2+1):
        val = im[j][x2]
        im[j][x2] = (val[0],max(255,val[1]+50),val[2])
        
    #top line
    for i in xrange(x1,x2+1):
        val = im[y2][i]
        im[y2][i] = (val[0],max(255,val[1]+60),val[2])
        
    #bottom line
    for i in xrange(x1,x2+1):
        val = im[y1][i]
        im[y1][i] = (val[0],max(255,val[1]+60),val[2])
    """


def get_val_from_dict(key):
    if ubyte_to_unicode_mapper.has_key(key):        
        char = ubyte_to_unicode_mapper[key]
        #print char.encode('utf-8')
        return char.encode('utf-8')
    else:
        return "^".encode('utf-8')
    
#define UCHAR_TO_FLOAT_MIN_1_TO_1(c) ( (double(c) / (255.0 / 2.0)) - 1.0)

def UCHAR_TO_FLOAT_MIN_1_TO_1(x):
    ret = ((float(x) / (255.0 / 2.0)) - 1.0)
    return ret
        
        
        
        
#returns suggested char , delta between first place and second place as returned by the NN (classifications)
def get_suggested_char(cropped, verbose=False):
    global debug_box_num
    
    resized = cv2.resize(cropped, (28,28), interpolation= cv2.INTER_LANCZOS4)
    
    
    if save_dbg_images and debug_box_num<6:
        cv2.imwrite("C:\\temp\\live_editor_resized_%d.bmp" % debug_box_num,resized)       
    
    cv2.imshow("Feed_NN",resized)
    
    input_array = (c_double*(28*28))()
      
    ind = 0
    for y in xrange(resized.shape[0]):
        for x in xrange(resized.shape[1]):
            input_array[ind] = c_double( UCHAR_TO_FLOAT_MIN_1_TO_1(resized[y][x]) )
            ind += 1
            
    
    input_pointer = cast(input_array, POINTER(c_double))
        
    outs_num = NN_Outputs_Num
    
    output_array_array = (c_double*(outs_num))()    
    output_pointer = cast(input_array, POINTER(c_double))
    
    res = znnLib.znnProcessSingleCase(28*28, input_pointer, outs_num, output_pointer)
    
    max_ind = -1
    max_val = -9999.0
    for i in xrange(outs_num):
        if output_pointer[i] > max_val:
            max_val = output_pointer[i]
            max_ind = i
            
    """second_best_ind = -1
    second_max_val = -9999.0
    for i in xrange(outs_num):      
        if i==max_ind:
            continue
        if output_pointer[i] > second_max_val:
            second_max_val = output_pointer[i]
            second_best_ind = i
    """
                
    #print "Single case results in ",
    #print res,        
        
    
    if verbose == True:
        print "Max_Ind=%d" % max_ind
    
    first_score_char = get_val_from_dict(max_ind)
    #print "1. ",
    #if first_score_char == " ":
    #    print "[White Space]"
    #else:        
    #    print first_score_char
    
    """second_score_char = get_val_from_dict(second_best_ind)
    print "2. ",
    if second_score_char == " ":
        print "[White Space]"
    else:
        print second_score_char
    """
        
        
    #print "Max result (%d,%f) " % (max_ind,output_pointer[max_ind]) ,
    #print "Second best (%d,%f )" % (second_best_ind,output_pointer[second_best_ind]), 
    #delta = output_pointer[max_ind] - output_pointer[second_best_ind]
    #print "Delta is %f" % delta
    
    #print "NN decided Character is: ",
        
    res = get_val_from_dict(max_ind)
    
    #return res , delta
    return res , max_val
    
debug_box_num = 0

def print_tabs(n):
    for i in xrange(n):
        print "\t",

#we always need to average the scores,
#to make sure we don't bias towards more splits.
def check_all_options_for_a_fixed_right_end(word_bw, x_end,depth, prev_char_width):    
    verbose = False    
    if x_end == -1:
        return 0.0, 0
    #if depth > 2:
    #    return 0.0 , -1
    
    best_start_x = -1
    curr_char_score = 0.0
    best_summed_score = 0.0
    best_avg_score = 0.0
    best_splits_num = 0
    
    tot_score = 0
    splits_num = 0
            
    #for curr_start_x in xrange(x_end,-1,-1):
    for curr_start_x in xrange(x_end,-1,-1):
        if x_end - curr_start_x >= prev_char_width*4:
            break
            
        
        if 0 == depth:
            print "Working on curr_start_x=%d - depth=0" % curr_start_x
        
        cropped_bw = word_bw[0:word_bw.shape[0] , curr_start_x:x_end+1]    
        char , curr_char_score = get_suggested_char(cropped_bw)                
        
        if True==verbose:
            print_tabs(depth)
            print "start_x=%d,curr_char_score=%f" % (curr_start_x, curr_char_score)
        
        if curr_char_score < 0.9:
            if True==verbose:
                print_tabs(depth)            
                print "Discarded."
            continue
        
        if depth < 3:
            tot_score, splits_num = check_all_options_for_a_fixed_right_end(word_bw, curr_start_x-1, depth+1, x_end+1-curr_start_x)
            tot_score += curr_char_score                                
        else:
            tot_score = curr_char_score
            splits_num = 0
            
        
        avg_score = tot_score / float(splits_num+1)
                
        
        if True==verbose:
            print_tabs(depth)
            print "avg_score = %f / %d" % (tot_score,splits_num+1)
            print_tabs(depth)
            print "tot_score=%f, avg_score=%f, %d elements participated" % (tot_score, avg_score, splits_num+1)
        
        # >= or > matters, because >= means that in the case of identical score,
        # we will favor lower start_x
        if avg_score >= best_avg_score: 
            best_avg_score = avg_score
            best_summed_score = tot_score
            best_start_x = curr_start_x
            best_splits_num = splits_num
            
        #hack - i need to recheck this
        #if x_end - curr_start_x > 10:
        #    break
    
    
    if True==verbose:
        print_tabs(depth)
        print "selected best_start_x=%d" % best_start_x
    
    if 0 == depth:
        if -1 == best_start_x:
            #we didn't find anything
            print "Error! no suggestion!!!"
            return 0
        else:
            if True==verbose:
                print "curr_char_score=%f" % curr_char_score
                print "best_summed_score=%f" % best_summed_score
                print "best_avg_score=%f" % best_avg_score
                print "best_start_x=%d" % best_start_x
            return best_start_x

    if best_start_x==-1: # didn't find any candidate
        return 0.0,1
        
    
    return best_summed_score , 1+best_splits_num


def PREV__check_all_options_for_a_fixed_right_end(word_bw, x_end):    
    best_start_x = -1
    best_delta = 0.0
    
    for curr_start_x in xrange(x_end,-1,-1):    
        cropped_bw = word_bw[0:word_bw.shape[0] , curr_start_x:x_end+1]    
        char , delta = get_suggested_char(cropped_bw)
        if delta > best_delta:
            best_delta = delta
            best_start_x = curr_start_x
            
        #hack - i need to recheck this
        if x_end - curr_start_x > 10:
            break
    
    return best_start_x

def FindCharactersInWord(word_rgb,abs_start_x, abs_start_y, current_resolution):    
    global view_factor
    global debug_box_num
    
    word_bw = cv2.cvtColor(word_rgb, cv2.COLOR_BGR2GRAY)
    
    tot_width = word_bw.shape[1]
    tot_height = word_bw.shape[0]    
            
    #scaled_bw_orig = cv2.resize(word_bw, (word_bw.shape[1]*view_factor,word_bw[0]*view_factor),interpolation= cv2.INTER_NEAREST)
    
    scaled_bw_orig = cv2.resize(word_bw, (tot_width*view_factor,tot_height*view_factor),interpolation= cv2.INTER_NEAREST)        
            
    cv2.imshow("orig_bw", scaled_bw_orig)
    
    x_end = tot_width-1
    x_start = max(0,x_end-3)
        
    char_boxes = []
        
    prev_length = 2
    prev_prev_length = 2
    
        
    
    initial_guess_set = False
    
    while (True):
        
        #choose best option using the trained NN
        #starting from a 1 pixel wide bbox, while the right end is fixed,
        #iterate on all options, and choose the one with the most distinct classification.
        #currently it's measured by delta between top classification and second place classification.
        
        """
        if False == initial_guess_set:
            x_start = check_all_options_for_a_fixed_right_end(word_bw, x_end, 0)
            cropped_bw = word_bw[0:word_bw.shape[0] , x_start:x_end+1]
            char , delta = get_suggested_char(cropped_bw)
            initial_guess_set = True
        """                
        
        left_char = word_bw[0:word_bw.shape[0] , x_start:x_end+1]
        display_rgb = word_rgb.copy()        
        draw_square(display_rgb, (x_start,0), (x_end,(tot_height-1)))
        
        scaled_display_rgb = cv2.resize(display_rgb, (tot_width*view_factor,tot_height*view_factor),interpolation= cv2.INTER_NEAREST)        
               
        cv2.imshow("s", scaled_display_rgb)
        
        k = GetKey()
        
        cv2.destroyWindow("s")
        
        if 102 == k: #'f'
            x_start = check_all_options_for_a_fixed_right_end(word_bw, x_end, 0, 100)
            cropped_bw = word_bw[0:word_bw.shape[0] , x_start:x_end+1]
            char , delta = get_suggested_char(cropped_bw)
            initial_guess_set = True
        
        
        if KEY_LEFT == k:
            x_start-= 1
            if x_start < 0:
                x_start = 0
                
        if KEY_RIGHT == k:
            x_start+= 1
            if x_start >= x_end:
                x_start = x_end
                
        if KEY_ENTER == k:
            print "Char at: %d->%d" % (x_start, x_end)
            
            cropped_bw = word_bw[0:word_bw.shape[0]-1, x_start:x_end+1]
            
            if save_dbg_images and debug_box_num<6:
                cv2.imwrite("C:\\temp\\live_editor_cropped_%d.bmp" % debug_box_num,cropped_bw)
            
            char , score = get_suggested_char(cropped_bw)
            
            debug_box_num += 1
            
            char_b = zedusBoxes.BBox(abs_start_x+x_start, abs_start_y+0, abs_start_x+x_end, abs_start_y+tot_height, char, 0, current_resolution, mirror_y=False)
            char_boxes.append(char_b)
            
            if x_start == 0:
                print "Done."
                break
            
            prev_prev_length = prev_length
            
            prev_length = x_end-x_start
            x_end = x_start - 1
            #x_start = x_end-1
            
            
            #restore!
            #x_start = max(0,x_end-prev_prev_length)
            
            x_start = max(0,x_end-1)
            
            initial_guess_set = False
            
        if KEY_TAB == k:
            print "Abandoned word."
            break
        
        cropped_bw = word_bw[0:word_bw.shape[0] , x_start:x_end+1]            
        char , score = get_suggested_char(cropped_bw, True)
        print "Curr marked char score=%f, x_start=%d, char=%s" % (score, x_start,char)
        
        
    cv2.destroyWindow("orig_bw")
        

    return char_boxes

def FillInChars(no_ext,input_box_ext):
    global current_resolution
    global boxes_dict
    global zoom_wind_width,zoom_wind_height        
    global zoom_char_im
    global debug_mode
    im = cv2.imread(no_ext+".jpg")
    
    original_image = cv2.imread(no_ext+".jpg")
    
    current_resolution = (im.shape[1], im.shape[0])
    
    print "Resolution: %dx%d" % (current_resolution[0],current_resolution[1])
    
    cv2.imshow("window", im)
    
    
    #restore if you want to start with the full words bboxes
    #load_bboxes_name = no_ext+u".boxWithWordsNumbers"        
    
    load_bboxes_name = no_ext+u".%d_%d_fullySortedCharBoxes" % (current_resolution[0],current_resolution[1])
    
    #restore for full word bboxes
    #load_bboxes_name = no_ext+"."+input_box_ext
    
    
    save_bboxes_name = no_ext+u".boxSplittedToChars"
        
    if os.path.isfile(save_bboxes_name):
        if not tkMessageBox.askyesno("Box file already exists!" , "Do you want to edit [%s]?" % save_bboxes_name):
            return        
    
    print "Loading boxes from [%s] ..." % load_bboxes_name
    
    boxes = zedusBoxes.read_box_file(load_bboxes_name, current_resolution)
    
    boxes_num = len(boxes)  
    
    box_num = 1    
    b_ind = 0
    
    character_boxes = []
    
    #for b in boxes:
    #for b_ind in xrange(boxes_num):
    while (b_ind < boxes_num):
        b = boxes[b_ind]
        
        cropped = original_image[b.y1:b.y2+1,b.x1:b.x2+1]
        
        #print "Top left corner is (%d,%d)" % (b.x1,b.y1+1)
        word_char_boxes = FindCharactersInWord(cropped,b.x1,b.y1, current_resolution)
        character_boxes.extend(word_char_boxes)
        
        #character_boxes.
        
        """while (True):
            k = GetKey()            
            #if k != None:
            break
            """
        
        box_num+=1        
        b_ind += 1
    
    
    zedusBoxes.save_box_file(save_bboxes_name, character_boxes, current_resolution)
    

for root, _, files in os.walk(images_path):
        for f in files:
            file_lwr = f.lower()
           
            #restore for full words bboxes
            #if not 'sortedFullWords' in f:
            if not 'boxDataBase' in f:
                continue
            
            spl = f.split('.')
                           
            print u'Working on: %s' % f
            fullpath = os.path.join(root, f)    
            no_ext = fullpath[0:-len(spl[-1])-1]
                                   
            #input_box_ext = no_ext+"."+spl[-1]
                                   
            FillInChars(no_ext, spl[-1])
            #GetKey()            


cv2.destroyAllWindows()