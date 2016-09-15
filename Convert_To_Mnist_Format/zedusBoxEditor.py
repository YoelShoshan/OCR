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
import zedusConfig
if zedusConfig.support_gui == True:
    import tkMessageBox
import math
from zedusCharacterDict import *
import subprocess
from ctypes import *

if zedusConfig.support_gui == True:
	import Tkinter
	#import ctypes

	tk = Tkinter.Tk()

	tk.withdraw()

KEY_LEFT = 2424832
KEY_RIGHT = 2555904
KEY_TAB = 9
KEY_ENTER = 13
KEY_SPACE = 32

"""
znnLib = cdll.LoadLibrary(r"C:\google_code\zedusnn\Release\zedusNN.dll")
prob_type = c_uint(1) #enum - 1 means mnist, which actually means right now 28x28 grey-scale character classification.
znnLib.znnInit.restype = c_char_p
znnLib.znnLoadNN.restype = c_bool
znnLib.znnProcess.restype = c_bool
znnLib.znnProcessSingleCase.restype = c_bool
znnLib.znnTrainOnDB.restype = c_bool
znnLib.znnTrainOnDB_StartFromProvidedNN.restype = c_bool
res = znnLib.znnInit(prob_type, c_bool(False))
print "Initialized zedusNN library. Result was [%s]" % res
"""

debug_mode = True

#images_path = u"C:/OCR/test/"
#images_path = u"C:/OCR/z.test_set/"
#images_path = u"C:/OCR/z.train_set/"
#images_path = u"C:/OCR/new_hard/"
#images_path = u"C:/OCR/fails/words bbox detection/"
#images_path = u"C:/OCR/full_db/"
images_path = ""

typing_mode_hebrew = True

current_resolution = (0,0)
boxes_dict = {}

zoom_wind_width = 28*2
zoom_wind_height = 28*2

zoom_char_im = np.zeros((zoom_wind_width,zoom_wind_height), np.uint8)

if zedusConfig.support_gui == True:
	cv2.namedWindow("window", cv2.CV_WINDOW_AUTOSIZE)
	cv2.namedWindow("zoom_char", cv2.CV_WINDOW_AUTOSIZE)

original_image = None

num_to_color = {}

def showEnglishModeWarning():
    warning_im_size = (820,50)
    warning_im = Image.new('RGB',warning_im_size)    
    draw = ImageDraw.Draw(warning_im)    
    
    font_name = ""

    if os.name == 'posix':
        font_name = zedusConfig.home_path+"/fonts/arial.ttf"
    else:
        font_name = u"C:/Windows/Fonts/arial.ttf"
    
    font = ImageFont.truetype(font_name, 30, encoding="UTF-8")    
    #msg = u"אזהרה!! כתיבה בעברית כובתה!!!"
    #msg = u"התבוכ תירבעב הביתכ !! הרהזא !!!"
    msg = u"!! התבוכ תירבעב הביתכ !! הרהזא Press F5 to restore heb typing."
    
    draw.text((0,0), msg, (255,0,0), font=font)
    """pos_x = 0
    for c in decoded_last_word:
        #pos_x+=10
        draw.text((last_written_text_size[0]-10-pos_x,10), c, (255,0,0), font=font)
        
        s = draw.textsize(c, font=font)
        pos_x+=10"""
    #convert to cv in order to show
    imcv = cv2.cvtColor(np.asarray(warning_im), cv2.COLOR_RGB2BGR)
    if zedusConfig.support_gui == True:
    	cv2.imshow("TypingModeWarning",imcv)

#def onmouse(event, x, y, flags, param):
#    global seed_pt
#    if flags & cv2.EVENT_FLAG_LBUTTON:
#        seed_pt = x, y
#        update()

#cv2.setMouseCallback('mouse_click', onmouse)

def translate(ascii_key):
    global typing_mode_hebrew
    
    if False == typing_mode_hebrew:
        c_enc = unicode(ascii_key).encode('UTF-8')
        print c_enc
        return c_enc
        
    # we are now at Hebrew typing mode
    
    
    #Note: i removed "ץ" because Elad Cohen requested that '.' will stay '.' (because of the '.' in the keypad)

    # orig:
    #heb_chars = u"/'קראטוןםפשדגכעיחלךף,זסבהנמצתץ."
    #eng_chars = u"qwertyuiopasdfghjkl;'zxcvbnm,./"
    
    #heb_chars = u"/'קראטוןםפשדגכעיחלךף,זסבהנמצתץ."
    #eng_chars = u"qwertyuiopasdfghjkl;'zxcvbnm,\/"
    
    heb_chars = u"/'קראטוןםפשדגכעיחלךף,זסבהנמצתץ.X"
    eng_chars = u"qwertyuiopasdfghjkl;'zxcvbnm,\/`"
    
    
    #special case - 
    
        
    ind = eng_chars.find(ascii_key)
    
    #if ascii_key == '.': #special case, make sure that '.' stays '.' (elad asked, because of the '.' in the keypad)
    
    if (ind >= 0):
        c = heb_chars[ind:ind+1]
        c_enc = c.encode('UTF-8')
        print '"',
        print c_enc,
        print '" '
        return c_enc
    c_enc = unicode(ascii_key).encode('UTF-8')
    print '"',
    print c_enc,
    print '" '
    return c_enc
        

def GetKey():
    global typing_mode_hebrew
    global debug_mode
    
    if True == debug_mode:
        return u"@".encode('UTF-8') , None
    
    while True:        
        raw_ch = cv2.waitKey()
        ch = 0xFF & raw_ch
        
        if KEY_LEFT == raw_ch:
            print "keep db label."
            return None , None
        
        #F5
        if 0x740000 == raw_ch:
            typing_mode_hebrew = True
            print "Hebrew Typing on!"
            continue
            
        #F6
        if 0x750000 == raw_ch:
            typing_mode_hebrew = False
            print "Hebrew Typing off!"
            continue
        
        #print ch
        if ch > 128:
            return None , None
        
        translated = translate(chr(ch))
        #if ch == 27:
        #    break        
        return translated, raw_ch



def get_box_x2(b):
    return b.x2

def get_box_y1(b):
    return b.y2

def compare_boxes(b1,b2):
    if b1.y2 < b2.y2:
        return 1
    elif b1.y2 > b2.y2:
        return -1
    if b1.x2 < b2.x2:
        return 1
    elif b1.x2 > b2.x2:
        return -1
    return 0

#rotates a point around (0,0)
#expects float values
def rotate_point((x,y),rad):
     x_res = (x*math.cos(rad)) - (y*math.sin(rad))
     y_res = (x*math.sin(rad)) + (y*math.cos(rad))     
     return (x_res,y_res)


def ray_point(ray_s,ray_d,t):
    x = ray_s[0] + (t*ray_d[0])
    y = ray_s[1] + (t*ray_d[1])
    return (x,y)

#
def ray_aabb_intersection(ray_s, ray_d, b):
    
    if (ray_d[1] != 0.0):
        t = (b.y1 - ray_s[1]) / ray_d[1] #top    
        if (t > 0):
            col = ray_point(ray_s,ray_d, t)
            if col[0] >= b.x1 and col[0] <= b.x2:
                return True
    if (ray_d[1] != 0.0):
        t = (b.y2 - ray_s[1]) / ray_d[1] #bottom
        if (t > 0):
            col = ray_point(ray_s,ray_d, t)
            if col[0] >= b.x1 and col[0] <= b.x2:
                return True
    if (ray_d[0] != 0.0):
        t = (b.x1 - ray_s[0]) / ray_d[0] #left
        if (t > 0):
            col = ray_point(ray_s,ray_d, t)
            if col[1] >= b.y1 and col[1] <= b.y2:
                return True
    if (ray_d[0] != 0.0):
        t = (b.x2 - ray_s[0]) / ray_d[0] #right
        if (t > 0):
            col = ray_point(ray_s,ray_d, t)
            if col[1] >= b.y1 and col[1] <= b.y2:
                return True             
    return False


#a simple horizontal line scan test, checking which line has the most intersecting bboxes
#Note: will fail on non almost horizontal lines.
def FindBestLine(boxes, width, height, groupNumber):
    best_line_boxes_num = 0
    best_line = 0
    best_line_rad = 0.0
    count_line_boxes = 0
    for j in xrange(current_resolution[1]):        
        #status = int ((100.0 * j) / float(current_resolution[1]))            
        #if j%50 == 0:
        #    print "%d %%" % status
        #for i in reversed(xrange(current_resolution[0])):
        
        if j%100 == 0:
            print "Line %d" % j
        
        range = 7
        #for every angle 
        for a in xrange(range*2):
        #a = 0
            intersecting_boxes_num = 0
            curr_ang = float(a - range)
            curr_rad = curr_ang * 0.0174532925        
            for b in boxes:
                if True == b.selected:
                    continue
                #if b.y1 < j and b.y2 > j:
                #create a ray, shooting from the right edge of the screen
                #sweep a ray like a flash light, searching for the best angled ray
                
                ray_dir = rotate_point((-1.0,0.0), curr_rad)
                if ray_aabb_intersection( (float(width-1),float(j)) , ray_dir , b):
                    intersecting_boxes_num += 1
            if intersecting_boxes_num > best_line_boxes_num or (intersecting_boxes_num == best_line_boxes_num and curr_rad < best_line_rad):
                best_line_boxes_num = intersecting_boxes_num
                best_line = j
                best_line_rad = curr_rad
        
            
            
    
    res_boxes = []
    
    min_val = 100
    groupColor = (min_val+random.random()*(255-min_val-1),min_val+random.random()*(255-min_val-1),min_val+random.random()*(255-min_val-1))
    
    for b in boxes:
        if True == b.selected:
            continue
        #if b.y1 < best_line and b.y2 > best_line:
        ray_dir = rotate_point((-1.0,0.0), best_line_rad)
        if ray_aabb_intersection( (float(width-1),float(best_line)) , ray_dir , b):
            res_boxes.append(b)
            b.selected = True
            b.groupColor = groupColor
            b.groupNumber = groupNumber
    
    res_boxes = sorted(res_boxes, key=get_box_x2, reverse=True)
    
    return (best_line,best_line_rad,res_boxes)


#tesseract offer is quite bad for low res images...
def get_tesseract_offer_for_character_image(cropped_area):
    #offered_char
    #cropped_area.write
    cv2.imwrite(r"C:\temp\char.bmp", cropped_area)
    
    subprocess.call([r'Tesseract.exe', r"C:\temp\char.bmp", r"C:\temp\char_as_text",'-l', 'heb2', '-psm','10'])
    
    f = open(r"C:\temp\char_as_text.txt")
    
    lines = f.readlines()
            
    for l in lines:
        spl = l.split()
        #if (len(spl[0]) > 2):
        if (len(spl[0].decode('UTF-8')) > 1):
            print "Error!!! more than 1 character! [%s]. Discarding bbox." % spl[0]
            return None
        
        if (len(spl[0]) > 2):
            print "Error!!! more than 2 characters in the raw representation! Suspecting NIKKUD in [%s]. Discarding bbox." % spl[0]
            return None
        
        print "Tes Offering: %s" % spl[0]
        return spl[0]


def GetIntegerColor(num):
    global num_to_color
    groupColor = None    
    #get groupcolor from groupnum
    if num_to_color.has_key(num):
        groupColor = num_to_color[num]
    else:
        min_val = 100
        groupColor = (min_val+random.random()*(255-min_val-1),min_val+random.random()*(255-min_val-1),min_val+random.random()*(255-min_val-1))
        num_to_color[num] = groupColor
        
    return groupColor
      
"""    
def FindCharactersInWord(cropped):
    word_bw = cv2.cvtColor(cropped, cv2.COLOR_BGR2GRAY)
    
    #cropped = original_image[b.y1+1:b.y2+2,b.x1:b.x2+1]
    left_char = word_bw[0:word_bw.shape[0] , 0:10]
    resized = cv2.resize(left_char, (28,28),interpolation= cv2.INTER_LANCZOS4)
    cv2.imshow("NN_Input", resized)
  
    input_array = (c_double*(28*28))()
    
    ind = 0
    for y in xrange(resized.shape[0]):
        for x in xrange(resized.shape[1]):
            input_array[ind] = c_double(resized[y][x])
            ind += 1
            
    
    input_pointer = cast(input_array, POINTER(c_double))
        
    
    output_array_array = (c_double*(124))()    
    output_pointer = cast(input_array, POINTER(c_double))
    
    res = znnLib.znnProcessSingleCase(28*28, input_pointer, 124, output_pointer)
    
    max_ind = -1
    max_val = -9999.0
    for i in xrange(124):
        if output_pointer[i] > max_val:
            max_val = output_pointer[i]
            max_ind = i
                
    print "Single case results in ",
    print res,
    print " Max result was for index %d" % max_ind
    
    print "NN decided Character is: ",
    
    if ubyte_to_unicode_mapper.has_key(max_ind):        
        char = ubyte_to_unicode_mapper[max_ind]
        print char.encode('utf-8')
    else:
        print "[Out Of Range]"
"""

def FillInChars(no_ext):
    global current_resolution
    global boxes_dict
    global zoom_wind_width,zoom_wind_height        
    global zoom_char_im
    global debug_mode
    
    im = cv2.imread(no_ext+".jpg",1) # flag>0 makes it return 3 component color image
    #im = cv2.cvtColor(im, cv2.COLOR_GRAY2BGR)
    #im=im.convert('RGB')
    
    original_image = cv2.imread(no_ext+".jpg",1)# flag>0 makes it return 3 component color image
    #original_image = cv2.cvtColor(original_image, cv2.COLOR_GRAY2BGR)
    #original_image=original_image.convert('RGB')
    
    current_resolution = (im.shape[1], im.shape[0])
    
    print "Resolution: %dx%d" % (current_resolution[0],current_resolution[1])
    if zedusConfig.support_gui == True and False == debug_mode:
        cv2.imshow("window", im)
    
    
    #restore if you want to use the zedusSplitWordChars.py flow
    #load_bboxes_name = no_ext+u".boxSplittedToChars"
    #load_bboxes_name = no_ext+u".boxFillMe"
    #load_bboxes_name = no_ext+u".%d_%d_boxCharsAfterDiscardingNonWordsBBoxes" % (current_resolution[0],current_resolution[1])
    
    load_bboxes_name = no_ext+u".%d_%d_boxFillMe" % (current_resolution[0],current_resolution[1])
    
    save_bboxes_name = no_ext+u".boxDataBase"
        
    if False == debug_mode and os.path.isfile(save_bboxes_name):
        if tkMessageBox.askyesno("Box file already exists!" , "Do you want to edit [%s]?" % save_bboxes_name) == True:
            load_bboxes_name = save_bboxes_name            
        else:
            if tkMessageBox.askyesno("Box file already exists!" , "Do you want to create a new one instead of it? [%s]?" % save_bboxes_name) == True:
                pass
            else:
                return
            
    
    
    #load_bboxes_name = no_ext
    
    print "Loading boxes from [%s] ..." % load_bboxes_name
    
    boxes = zedusBoxes.read_box_file(load_bboxes_name, current_resolution)
    
    boxes_num = len(boxes)  
    
    box_num = 1
    
    b_ind = 0
    
    ################################################
    #loop on all characters and allow editing
    ################################################
    
    last_written_text_size = (800,50)
    last_written_text_im = Image.new('RGB',last_written_text_size)
    last_written_text_im_data = last_written_text_im.load()
    draw = ImageDraw.Draw(last_written_text_im)    
    
    font_name = ""

    if os.name == 'posix':
        font_name = zedusConfig.home_path+"/fonts/arial.ttf"
    else:
        font_name = u"C:/Windows/Fonts/arial.ttf"
        
    font = ImageFont.truetype(font_name, 16, encoding="UTF-8")    
   
    last_word = ""
   
    prev_chars = []
    
    #-1 means not initialized
    #-2 means do not add space in the first time you see difference
    curr_word_number = -1
   
    #for b in boxes:
    #for b_ind in xrange(boxes_num):
    while (b_ind < boxes_num):
        b = boxes[b_ind]
        
        if debug_mode == False:
            print "--------------------"
                        
                        
        if curr_word_number == -2:
            curr_word_number = b.wordNumber
        elif curr_word_number != b.wordNumber:
            prev_chars.append(u" ")
            curr_word_number = b.wordNumber
            
        #create zoom image
        #cropped = cv2.CreateImage( ( b.x2-b.x1, b.y1-b.y2), im.depth, im.nChannels)
        
        #cropped = cv2.cv.GetSubRect(original_image, )
        
        #cv2.imwrite('roi.png', im[r[0]:r[0]+r[2], r[1]:r[1]+r[3]])
        
        #if (b.y1-b.y2 == 0):
        #    dbg = 12
        
        #b.y1-b.y2
        #print b
        
        cropped = original_image[b.y1:b.y2+1,b.x1:b.x2+1]
        
        #FindCharactersInWord(cropped)
        
        
        #print "Extracting cropped (%d:%d,%d:%d)", (b.y1+1,b.y2+2,b.x1,b.x2+1)
        
        resized = cv2.resize(cropped, (28*2,28*2),interpolation= cv2.INTER_LANCZOS4)
        
        #print "DBG_BOX (%d,%d),(%d,%d)" % (b.x1,b.y2,b.x2,b.y1)
        
        #if b.x1 == 31:
        #cv2.imwrite(r"C:\temp\cropped.jpg",cropped)
        
        if zedusConfig.support_gui == True and False == debug_mode:
            cv2.imshow("zoom_char", resized)
            #cv2.
            
        """if False == debug_mode:
            cv2.destroyWindow("only_cropped")
            cv2.imshow("only_cropped", cropped)"""
        
        #offered_char = get_tesseract_offer_for_character_image(cropped)
        
        extra_y = 15
        
        #make sure we don't go out of bounds
        fixed_y2 = max(0,b.y1-extra_y)
        fixed_y1 = min(current_resolution[1]-1,b.y2+extra_y)
        
        extra_x = 50
        
        fixed_x1 = max(0,b.x1-extra_x)        
        fixed_x2 = min(current_resolution[0]-1,b.x2+extra_x)
                
        less_zoom_im = original_image[fixed_y2:fixed_y1,fixed_x1:fixed_x2].copy()
        
        line_bottom = min(b.y2-b.y1+extra_y+1 , current_resolution[1]-1)
        
        line_top = extra_y
        
        line_start = 50
        if b.x1 - 50 < 0:
            line_start += b.x1 - 50
        line_end = line_start + b.x2 - b.x1
                
        fact = 5        
                        
        less_zoom_im = cv2.resize(less_zoom_im, (less_zoom_im.shape[1]*fact,less_zoom_im.shape[0]*fact),interpolation= cv2.INTER_LANCZOS4)
        
        #bottom
        cv2.line(less_zoom_im, (line_start*fact,line_bottom*fact), (line_end*fact, line_bottom*fact), (0,0,255))
        #top
        cv2.line(less_zoom_im, (line_start*fact,line_top*fact), (line_end*fact, line_top*fact), (0,0,255))
        #left
        cv2.line(less_zoom_im, (line_start*fact,line_bottom*fact), (line_start*fact, line_top*fact), (0,0,255))
        #right
        cv2.line(less_zoom_im, (line_end*fact,line_bottom*fact), (line_end*fact, line_top*fact), (0,0,255))
        
        if False == debug_mode:
            draw_area_width = min(500,less_zoom_im.shape[1]-1)
            char_im = Image.new('RGB',(draw_area_width,30))
            char_im_data = char_im.load()
            draw = ImageDraw.Draw(char_im)    
            font = ImageFont.truetype(u"C:/Windows/Fonts/Arial.ttf", 30, encoding="UTF-8")
                       
            pos_x = 0
            count = 0
            for c in reversed(prev_chars):
                col = (0,255,0)
                draw.text((pos_x,0), c.decode('UTF-8'), col, font=font)
                size = draw.textsize(c.decode('UTF-8'), font=font)
                pos_x += size[0]
                count += 1
                if count > 100:
                    break
            #draw.text((0,0), u"Prev: ".decode('UTF-8')+prev_char.decode('UTF-8'), (0,255,0), font=font)
            
            char_im_cv = cv2.cvtColor(np.asarray(char_im), cv2.COLOR_RGB2BGR)
            
            w = char_im_cv.shape[1]
            h = char_im_cv.shape[0]
            
            less_zoom_im[0:h,0:w] = char_im_cv
            
            #less_zoom_im = original_image[fixed_y2:fixed_y1,fixed_x1:fixed_x2].copy()
            
            cv2.imshow("less_zoom_char", less_zoom_im)
        
        #cv2.rectangle(im, (b.x1,b.y2), (b.x2,b.y1), b.groupColor)
        #cv2.rectangle(im, (b.x1,b.y2), (b.x2,b.y1), (255,0,0))
        if zedusConfig.support_gui == True and False == debug_mode:
            cv2.imshow("window", im)
        
        if debug_mode == False:
            print "Current value:%s" % b.char                                        
        
        
        while (True):
            k, raw_k = GetKey()            
            #if k != None:
            break
            
                                    
        if typing_mode_hebrew == False:
            showEnglishModeWarning()
        else:
            if zedusConfig.support_gui == True:
                cv2.destroyWindow("TypingModeWarning")
        
            
        #backspace
        if '\x08' == k:
            if b_ind > 0:
                b_ind -= 1
            #last_word = last_word[:-len(]
            
            #if we just moved to a new word, we only need to remove this space
            poped = prev_chars.pop()
            if poped == u' ':
                poped = prev_chars.pop()
                curr_word_number = -2
            continue
            
        #if k == u' '.encode('UTF-8'):
        if raw_k == KEY_TAB:
            print "Skipped!"
            b.ignore = True
            prev_chars.append(u"?")
        else:
            if None != k: # for the case that we decided to keep current entry
                b.char = k
                #selected_boxes.append(b)
                prev_chars.append(b.char)
            else:
                #prev_chars.append(b.char)
                pass

        """
        if b.ignore == False:            
            last_word+= b.char
            
            decoded_last_word = last_word.decode('UTF-8')
            
            #last_written_text_im
            #draw.
            for i in xrange(last_written_text_im.size[0]):
                for j in xrange(last_written_text_im.size[1]):
                    last_written_text_im_data[i,j] = (0,0,0)
            
            pos_x = 0
            for c in decoded_last_word:
                #pos_x+=10
                draw.text((last_written_text_size[0]-10-pos_x,10), c, (0,0,255), font=font)
                
                s = draw.textsize(c, font=font)
                pos_x+=10
            #convert to cv in order to show
            imcv = cv2.cvtColor(np.asarray(last_written_text_im), cv2.COLOR_RGB2BGR)
            cv2.imshow("LastTyped",imcv)
        """
            
        groupColor = GetIntegerColor(b.wordNumber)
        wordColor = GetIntegerColor(b.groupNumber)
                
        cv2.rectangle(im, (b.x1,b.y2+1), (b.x2,b.y1), wordColor)        
        cv2.line(im, (b.x1,b.y1), (b.x2,b.y1), groupColor)
        
        if debug_mode == False:
            print " (%d out of %d boxes)" % (box_num, boxes_num)        
        box_num+=1
        
        b_ind += 1
        
    print "Saving new box files into [%s]..." % save_bboxes_name        
    
    selected_boxes = []
    
    for b in boxes:
        if False == b.ignore:
            selected_boxes.append(b)
    
    zedusBoxes.save_box_file(save_bboxes_name, selected_boxes, current_resolution)
    
    #debug - in order that other box file viewer apps will show
    #zedusBoxes.save_box_file(no_ext+".boxDebugDataBase", selected_boxes, current_resolution, force_group_zero=True)    
    
    cv2.imwrite(no_ext+"_ColoredLines.png",im);
    
    if True == debug_mode:
        return
    if not tkMessageBox.askyesno("Done creating [%s]." % save_bboxes_name, "Continue to next file?"):
	if zedusConfig.support_gui == True:
        	cv2.destroyAllWindows()
        sys.exit(0)
        
        
    


def main(path, dummy_typing):
    global images_path
    global debug_mode
    images_path = path
    debug_mode = False
    just_copy = False #no visual output
    if dummy_typing == "True":
        debug_mode = True
    elif dummy_typing == "Fast":
        just_copy = True
    
    for root, _, files in os.walk(images_path):
        for f in files:
            file_lwr = f.lower()
            spl = file_lwr.split('.')
            if not 'jpg' in spl[-1]:
                continue
            
            #print f
            
            if 'Local_Threshold' in f or 'WithBoxes' in f or 'MedianThreshold' in f:
                continue
                           
            print u'Working on: %s' % f
            fullpath = os.path.join(root, f)    
            no_ext = fullpath[0:-4]
            
            if just_copy == True:
                original_image = cv2.imread(no_ext+".jpg",1)# flag>0 makes it return 3 component color image    
                current_resolution = (original_image.shape[1], original_image.shape[0])    
                print "Resolution: %dx%d" % (current_resolution[0],current_resolution[1])    
                load_bboxes_name = no_ext+u".%d_%d_boxFillMe" % (current_resolution[0],current_resolution[1])    
                save_bboxes_name = no_ext+u".boxDataBase"
                
		if os.name == 'posix':
			os_cmd = 'cp "%s" "%s"' % (load_bboxes_name, save_bboxes_name)
		else:	
	                os_cmd = 'copy "%s" "%s"' % (load_bboxes_name, save_bboxes_name)
                
                print "About to call [%s]" % os_cmd
                
                os.system(os_cmd)                
            else:
                FillInChars(no_ext)
    print "cv2 - destroying all windows!"

    if zedusConfig.support_gui == True:        	
    	cv2.destroyAllWindows()
    
    
if __name__ == "__main__":
    if len(sys.argv) < 3:
        print "Not enough args!"
    main(*sys.argv[1:])

