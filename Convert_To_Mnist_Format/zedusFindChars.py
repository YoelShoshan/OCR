#!/usr/bin/env python
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
import subprocess
import zedusConfig
if zedusConfig.support_gui == True:
	import tkMessageBox
import math
import time


"""
master plan:
1.1 discard all individual char boxes that don't sit inside a word bbox - DONE!
1.2 stretch the remaining individual char bboxes into the height min/max of the word bbox (do it after i chopped the top and bottom) - DONE!
1.3 make sure that the rest of the flow reads it
-----
2.1 train a NN on 6-7 receipts (but don't touch 3 of the hards400)
2.2 based on that NN, start playing with automatic selection of characters within a word bbox
"""

#images_path = u"C:/OCR/fails/words bbox detection/"
#images_path = u"C:/OCR/test/"
#images_path = u"C:/OCR/z.train_set/"
#images_path = u"C:/OCR/hards400/"
#images_path = u"C:/OCR/new_hard/"
#images_path = u"C:/OCR/z.test_set/"
#images_path = u"C:/OCR/full_db/"
images_path = u""
 
im_width = 0
im_height = 0


calc_convert_to_greyscale = True

calc_threshold = True
#results in:
#8_merged_Local_Threshold.BMP
calc_find_boxes = True          
#results in:
#8_merged.626_2035_boxUnsorted

calc_find_words = True

#results in:
#8_merged.boxFillMe

calc_sort_boxes = True

#results in:
#8_merged.626_2035_boxFullWords 
#8_merged.sortedFullWords
#8_merged.626_2035_boxCharsAfterDiscardingNonWordsBBoxes
#8_merged.626_2035_fullySortedCharBoxes


#calc_detect_chars_in_words_bbox = False


conf_stretch_char_boxes = True
conf_split_sticked_chars_using_heuristics = True

debug_save_blob_merging_steps = False

median_filter_width     = 7
median_filter_height    = 7
median_filter_c         = 5
    

def check_pos(x,y,kernel_width, kernel_height):
    global im_width,im_height
    if kernel_width%2 == 0:
        print "Error! kernel width must be odd number. Given [%d] value is invalid." % kernel_width
        sys.exit(0)
    if kernel_height%2 == 0:
        print "Error! kernel height must be odd number. Given [%d] value is invalid." % kernel_width
        sys.exit(0)
    kernel_width_side = kernel_width / 2
    kernel_height_side = kernel_height / 2
    #ignore corners of the image    
    if x-kernel_width_side < 0:
       return False
    
    if x+kernel_width_side > im_width-1:
       return False
    
    if y-kernel_height_side < 0:
       return False
    
    if y+kernel_height_side > im_height-1:
       return False

    return True

def threshold_mean(data, x, y,kernel_width, kernel_height, c):
    global im_width,im_height

    if check_pos(x,y,kernel_width,kernel_height) == False:
        return 0

    kernel_width_side = kernel_width / 2
    kernel_height_side = kernel_height / 2    
    #mean
    
    avg = 0    
    for j in xrange(kernel_height):
        for i in xrange(kernel_width):    
            avg += data[x+i-kernel_width_side,y+j-kernel_height_side]
    
    avg /= float(kernel_width*kernel_height)
    
    if data[x,y] > avg-c:
        return 255

    return 0
    

def threshold_median_opt_9X9(data, x, y, kernel_width, kernel_height, c):
    global im_width,im_height

    if check_pos(x,y,kernel_width,kernel_height) == False:
        return 0

    kernel_width_side = kernel_width / 2
    kernel_height_side = kernel_height / 2
    
    #median
    l = []
    for j in xrange(kernel_height):
        for i in xrange(kernel_width):
            l.append(data[x+i-kernel_width_side,y+j-kernel_height_side])
        
    #a = np.array(l)
    #med = np.median(a)
    
    if data[x,y] > med-c:
        return 255    
    
    return 0 


def threshold_median(data, x, y,kernel_width, kernel_height, c):
    global im_width,im_height

    if check_pos(x,y,kernel_width,kernel_height) == False:
        return 0

    kernel_width_side = kernel_width / 2
    kernel_height_side = kernel_height / 2
    
    #median
    l = []
    for j in xrange(kernel_height):
        for i in xrange(kernel_width):
            l.append(data[x+i-kernel_width_side,y+j-kernel_height_side])
            
    a = np.array(l)
    med = np.median(a)
    
    if data[x,y] > med-c:
        return 255    
    
    return 0
      
def flip(data, x, y,kernel_width, kernel_height, c):
    return 255-data[x,y]

#basic method of erode, just kill pixels with no neighbours
def erode(data, x, y,kernel_width, kernel_height, c): 
    global im_width,im_height
    if check_pos(x,y,kernel_width,kernel_height) == False:
        return 0

    kernel_width_side = kernel_width / 2
    kernel_height_side = kernel_height / 2
    
    #erode
    minimum_allowed_found = 2
    found = 0
    for j in xrange(kernel_height):
        for i in xrange(kernel_width):
            if i==j:
                continue
            if data[x+i-kernel_width_side,y+j-kernel_height_side] == 0:
                found += 1
                if found >= minimum_allowed_found:
                    break
        if found >= minimum_allowed_found:
            break
        
    #if False == found:
    if found < minimum_allowed_found:
        #print "eroded! (%d,%d)" % (x,y)
        return 255

    return data[x,y]    


def threshold_image_cuda(no_ext):
    #if zedusConfig.fully_local == True:
        #subprocess.call([r"learn_cuda.exe", no_ext+".bmp", no_ext+"_MedianThreshold.bmp"])
    #else:
        #subprocess.call([r"C:\google_code\dooake\OCR\learn_cuda\Release\learn_cuda.exe", no_ext+".bmp", no_ext+"_MedianThreshold.bmp"])
        
    print 'About to run "CPUMedian.exe"'
    
    exe_name = "CPUMedian"
    
    if os.name != 'posix':
        exe_name+= ".exe"
            
    if zedusConfig.fully_local == True:
        subprocess.call([exe_name, no_ext+".bmp", no_ext+"_MedianThreshold.bmp"])
    else:
        #subprocess.call([r"C:\google_code\dooake\OCR\CPUMedian\Release\CPUMedian.exe", no_ext+".bmp", no_ext+"_MedianThreshold.bmp"])
        subprocess.call([zedusConfig.path_to_CPUMedian, no_ext+".bmp", no_ext+"_MedianThreshold.bmp"])

"""def threshold_image(im,out_file_name):    
    global im_width,im_height
    global threshold_median_width, threshold_median_height, threshold_median_c

    rgb_im = im
    #rgb_data = rgb_im.load()

    #### debug
    
    def commentus():
        gray = cv2.cvtColor(np.asarray(im), cv2.COLOR_RGB2BGR)    
        gray = cv2.cvtColor(gray,cv2.COLOR_RGB2GRAY)
    
        cv2.imshow("debug1", gray)
        cv2.waitKey()

        #gray = cv2.equalizeHist(gray)

        gray = cv2.adaptiveThreshold(gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C,cv2.THRESH_BINARY,7,7)
        #gray = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY | cv2.THRESH_OTSU)
        
        cv2.imshow("debug2", gray[1])
        cv2.waitKey()

        #TODO: Avoid using files - do it in memory.
        cv2.imwrite('temp_gray.BMP',gray)

        ####


        im = Image.open('temp_gray.BMP')       
        data = im.load()

    im = im.convert('L')
    data = im.load()
        

    def COMMENTED_OUT_prev_thresholding_method():
        threshold_im = Image.new('L',(im_width, im_height))
        threshold_im_data = threshold_im.load()
    
        print "Thresholding..."
    
        #threshold    
        for j in xrange(im_height):
            if j%50 == 0:
                print "Line %d" % j
            for i in xrange(im_width):
                threshold_im_data[i,j] = threshold_median(data,i,j,median_filter_width,median_filter_height,median_filter_c)
                #threshold_im_data[i,j] = threshold_median(data,i,j,3,3,5)
    
        #threshold_im.show()
    
        print "Eroding..."            
    
        for j in xrange(im_height):
            if j%50 == 0:
                print "Line %d" % j
            for i in xrange(im_width):
                if threshold_im_data[i,j] > 0:
                    threshold_im_data[i,j] = erode(threshold_im_data,i,j,3,3,7)


    print "Remove non gray pixels..."     

    col_delta = 45
    
    for j in xrange(im_height):
        if j%50 == 0:
            print "Line %d" % j
        for i in xrange(im_width):
            if threshold_im_data[i,j] > 0:
                if abs(rgb_data[i,j][0]-rgb_data[i,j][1]) > col_delta:
                    threshold_im_data[i,j] = 0
                    continue
                if abs(rgb_data[i,j][1]-rgb_data[i,j][2]) > col_delta:
                    threshold_im_data[i,j] = 0
                    continue
                if abs(rgb_data[i,j][0]-rgb_data[i,j][2]) > col_delta:
                    threshold_im_data[i,j] = 0
                    continue


    #threshold_im.show()            
    
    threshold_im_rgb = threshold_im.convert('RGB')
    threshold_im_rgb.save(out_file_name)
"""    

def merge(b1,b2):
    global im_width, im_height
    y1 = min(b2[0][1], b1[0][1])
    y2 = max(b2[1][1], b1[1][1])

    x1 = min(b2[0][0], b1[0][0])
    x2 = max(b2[1][0], b1[1][0])

    #if x2 - x1 > im_width / 4:
    #    return None

    #if y2 - y1 > 400:
    #    return None
    
    return ((x1,y1), (x2,y2))    
    
    
#[min/max][x/y]
def check_intersection_tuples(b1,b2):
    if b2[0][1]  > b1[1][1]:
        return False

    if b2[1][1]  < b1[0][1]:
        return False
    
    if b2[0][0]  > b1[1][0]:
        return False

    if b2[1][0]  < b1[0][0]:
        return False
    
    return True

def check_intersection(b1,b2):
    if b2.y1  > b1.y2:
        return False

    if b2.y2  < b1.y1:
        return False
    
    if b2.x1  > b1.x2:
        return False

    if b2.x2  < b1.x1:
        return False
    
    return True
    
def merge_intersecting_boxes(b1,b2):
    global im_width, im_height
    
    if check_intersection_tuples(b1,b2) == False:
        return None    

    #added limit for the merged box size
    #this is a hack for now, to handle non axis alligned receipts
            
    res = merge(b1,b2)
    
    if res!=None:
        if res[1][1]-res[0][1] > im_width * 0.05:
            return None
    
    #restore for debug print
    #if res!=None:
    #    print "Merged into: (%d,%d),(%d,%d)" % (b2[0][0],b2[0][1],b2[1][0],b2[1][1])

    return res


def group_blocks(blocks):
    new_blocks = []

    iter = 0
    while (len(blocks) > 0):
        if iter%1000 == 0:
            print "Iteration %d"  % iter
        iter += 1
        b = blocks.pop()

        found_merge_partner = False
        others_num = len(blocks)
        box_index = 0
        for b2 in blocks:            
            merged = merge_intersecting_boxes(b,b2)           
            if (merged != None):
                #if merged[0][0] == merged[1][0]:
                #    print "Error 2! Xs are identical! - %d" % bboxes[-1][0][0]
                new_blocks.append(merged)
                found_merge_partner = True
                del blocks[box_index]
                break
            box_index += 1
        if found_merge_partner == False:
            new_blocks.append(b) #didn't find someone to merge

    #for b in new_blocks:
    #    draw_block(b)          
            
    return new_blocks        


def detect_lines(gray, rgb_im):
    #i need to detect the longest 4 lines, in order to detect the receipt area.
    #starting from each of the 4 frame lines, allow some degree of rotation, and detect the receipt
    m,n = gray.shape

    gray = cv2.bitwise_not(gray)

    blur = cv2.GaussianBlur(gray, (0,0), 5)    

    if zedusConfig.support_gui == True:
    	cv2.imshow("debug1", blur)
    	cv2.waitKey()
    
    edges = cv2.Canny(blur, 20, 60)
    
    if zedusConfig.support_gui == True:
    	cv2.imshow("debug1", edges)
    	cv2.waitKey()         
    
    lines = cv2.HoughLines(edges, 2, np.pi/90, 40)[0]
    plines = cv2.HoughLinesP(edges, 1, np.pi/180, 20, np.array([]), 10)[0]

    #def t():    
    for (rho, theta) in lines[:5]:
        # blue for infinite lines (only draw the 5 strongest)
        x0 = np.cos(theta)*rho 
        y0 = np.sin(theta)*rho
        pt1 = ( int(x0 + (m+n)*(-np.sin(theta))), int(y0 + (m+n)*np.cos(theta)) )
        pt2 = ( int(x0 - (m+n)*(-np.sin(theta))), int(y0 - (m+n)*np.cos(theta)) )
        cv2.line(rgb_im, pt1, pt2, (255,0,0), 2)

    
    if zedusConfig.support_gui == True:
	    cv2.imshow("debug1", rgb_im)
	    cv2.waitKey()        

    for l in plines:
        # red for line segments
        cv2.line(rgb_im, (l[0],l[1]), (l[2],l[3]), (0,0,255), 2)

    if zedusConfig.support_gui == True:
	    cv2.imshow("debug1", rgb_im)
	    cv2.waitKey()

    sys.exit(0)    
    

def save_image_with_rects_tuple(load_image_name,boxes,save_image_name):
    im = cv2.imread(load_image_name)
    
    for b in boxes:
        min_val = 100
        col = (min_val+random.random()*(255-min_val-1),min_val+random.random()*(255-min_val-1),min_val+random.random()*(255-min_val-1))
        cv2.rectangle(im, (b[0][0],b[0][1]), (b[1][0],b[1][1]), col)
    cv2.imwrite(save_image_name,im)
    
def save_image_with_rects(load_image_name,boxes,save_image_name):
    im = cv2.imread(load_image_name)
    
    for b in boxes:
        min_val = 100
        col = (min_val+random.random()*(255-min_val-1),min_val+random.random()*(255-min_val-1),min_val+random.random()*(255-min_val-1))
        #cv2.rectangle(im, (b[0][0],b[0][1]), (b[1][0],b[1][1]), col)
        cv2.rectangle(im, (b.x1,b.y1), (b.x2,b.y2), col)
    cv2.imwrite(save_image_name,im)
    
    

def find_boxes_cv2_based(no_ext):
    global im_width, im_height
    
    #im = cv2.imread(u"C:\\OCR\\test\\hard1_Local_Threshold.BMP")
    im = cv2.imread(no_ext+u".bmp")
    
    im_width = im.shape[1]
    im_height = im.shape[0]

    print "Image size = %d x %d" % (im_width, im_height)
    
    imgray = cv2.cvtColor(im,cv2.COLOR_RGB2GRAY)

    #detect_lines(imgray,im)

   
   
    #minimum_pixels_amount = float(im_width) / 120.0
    
    #minimum_pixels_amount = float(im_width) / 250.0
    minimum_pixels_amount = float(im_width) / 250.0
    
    
    minimum_pixels_amount = int(minimum_pixels_amount)
    
    #debug
    #minimum_pixels_amount = 3
    print "Minimum pixel amount to keep blob is %d " % minimum_pixels_amount
   
    maximum_pixels_amount = float(im_width) / 20.0
    maximum_pixels_amount = int(maximum_pixels_amount*maximum_pixels_amount)
    print "Maximum pixel amount to keep blob is %d " % maximum_pixels_amount
   
    #maximum_height = float(im_width) / 15.0 #when i used this value, i discarded sticked (vetically) boxes
    maximum_height = float(im_width) / 7.5
    maximum_height = int(maximum_height)
    print "Maximum box height is %d" % maximum_height
    #cv2.imshow("debug1", imgray)
    #cv2.waitKey()

    #imgray = cv2.equalizeHist(imgray)
    #cv2.imshow("debug1", imgray)
    #cv2.waitKey()
    
    #imgray = cv2.adaptiveThreshold(imgray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C,cv2.THRESH_BINARY,7,7)
    
    #element = cv2.getStructuringElement(cv2.MORPH_CROSS,(3,3))
    #imgray = cv2.erode(imgray,element)
    #imgray = cv2.dilate(imgray,element)
    
    #cv2.imshow("window", imgray)
    #cv2.waitKey()

    h, w = imgray.shape[:2]
    mask = np.zeros((h+2, w+2), np.uint8)

    bboxes = []

    box_count = 0

    stop = False
    
    ignored_margins_width_factor = 0.05
    ignored_margins_width_pixels = ignored_margins_width_factor * im_width
    print "Ignoring the following margin size: %d" % ignored_margins_width_pixels        

    t1 = time.time()
    
    for j in xrange(h):
        for i in xrange(w):
            #if 481 == i+1:
            #    print "i"
            #if 481 == j+1:
            #    print "j"
            if im.item(j,i,0) > 0:
                continue
            if i >= w -2:
                continue
            if j >= h -2:
                continue
            #print "%d,%d" %(i+1,j+1)
            if mask[j+1,i+1] != 1:
                min_val = 100
                col = (min_val+random.random()*(255-min_val-1),min_val+random.random()*(255-min_val-1),min_val+random.random()*(255-min_val-1))
                #print "seed:%dx%d" % (i,j)
                r = cv2.floodFill(im, mask, (i,j), col)#,  flags = cv2.FLOODFILL_MASK_ONLY)
                #r = cv2.floodFill(im, mask, (310,402), col)#,  flags = cv2.FLOODFILL_MASK_ONLY)
                #if r[0] < 2:
                if r[0] < minimum_pixels_amount:
                    continue
                
                if r[0] > maximum_pixels_amount:
                    continue
                
                if r[1][3] > maximum_height:
                    continue
                
                #if r[0] > 30:
                #    continue
                box = ((r[1][0],r[1][1]), (r[1][0]+r[1][2]-1,r[1][1]+r[1][3]-1) )
                
                #discard word bboxes that are completely in the margins        
                if box[1][0] <= ignored_margins_width_pixels:
                    print "discarded for being entirely inside the left margin (%d pixels)" % ignored_margins_width_pixels
                    #mark_box_on_image(im_data,merged_box,(255,255,0))
                    continue
                #discard word bboxes that are completely in the margins        
                if box[0][0] >= im_width-ignored_margins_width_pixels-1:
                    print "discarded for being entirely inside the left margin (%d pixels)" % ignored_margins_width_pixels
                    #mark_box_on_image(im_data,merged_box,(255,255,0))
                    continue
                
                
                #if box[0][0] == box[1][0]:
                #    print "Error 1! Xs are identical! - %d" % box[0][0]
                #    print "Error was for box: (%d,%d),(%d,%d)" % (box[0][0],box[0][1],box[1][0],box[1][1])
                    
                
                bboxes.append(box)

                if box_count %50 == 0:
                    print "%d boxes" % box_count
                #print "seed=(%d,%d) - Rect = " % (j,i) ,
                #print r
                box_count += 1
                #if box_count > 2:
                    #stop = True
                    #break
            #if stop==True:
                #break

    

    #cv2.imshow("window", im)
    #cv2.waitKey()            
    
    #cv2.imwrite()
    
    t2 = time.time()
    #print "Find boxes end time: ",
    #print t2
    print "find boxes: Find the inital boxes time: ",
    print (t2-t1) / 60.0 ,
    print "Minutes."    
    
    print "Boxes Num = %d" % len(bboxes)
    
    t1 = time.time()    
    
    print "Merging initial boxes..."
    
    initial_boxes = []
    for b in bboxes:
        #t_b = zedusBoxes.BBox(b[0][0],im_width-b[0][1],b[1][0],im_width-b[1][1],u'^',0,(im_height, im_width))
        t_b = zedusBoxes.BBox(b[0][0],b[0][1],b[1][0],b[1][1],u'^',0,(im_width, im_height))
        initial_boxes.append(t_b)
    
    initial_boxes_file_name = no_ext[: -len('_Local_Threshold')]+u'.%d_%d_initialBoxes' % (im_width, im_height)
    
    zedusBoxes.save_box_file(initial_boxes_file_name, initial_boxes)
    
            
    if True == debug_save_blob_merging_steps:
        save_image_with_rects(no_ext+".bmp",initial_boxes, no_ext+"_Boxes_0.png" )
    
    print 'About to run "BoxMerger.exe"'
    
    exe_name = "BoxMerger"
    
    if os.name != 'posix':
        exe_name+= ".exe"
    
    if zedusConfig.fully_local == True:
        subprocess.call([exe_name, initial_boxes_file_name, "mergedBoxes"])
    else:
        #subprocess.call([r"C:\google_code\dooake\OCR\BoxOrganizer\Release\BoxMerger.exe", initial_boxes_file_name, "mergedBoxes"])
        subprocess.call([zedusConfig.path_to_BoxMerger, initial_boxes_file_name, "mergedBoxes"])

    t2 = time.time()
    #print "Find boxes end time: ",
    #print t2
    print "find boxes: Merge boxes time: ",
    print (t2-t1) / 60.0 ,
    print "Minutes."              
        
    
    print "Image width is:%d" % im_width
    smallest_bbox_area = im_width / 185.0
    smallest_bbox_area *= smallest_bbox_area
    print "Smallest allowed final box area is %d" % smallest_bbox_area
    
    col_kabala = cv2.imread(no_ext[: -len('_Local_Threshold')]+u".jpg")

    
    
    #now - filter out too small boxes, and split boxes that we assume that are sticked
    
    #read_box_file(box_file+".boxFillMe", im.size)
    merged_boxes = zedusBoxes.read_box_file(no_ext[: -len('_Local_Threshold')]+u'.mergedBoxes')
      
    t_bboxes = []
    filtered_out = 0
    
    #filter out too small boxes and calc new average width/height
    for b in merged_boxes:
        #t_b = zedusBoxes.BBox(b[0][0],im_width-b[0][1],b[1][0],im_width-b[1][1],u'^',0,(im_height, im_width))
        #t_b = zedusBoxes.BBox(b[0][0],b[0][1],b[1][0],b[1][1],u'^',0,(im_width, im_height))
        t_b = b
        #remove tiny boxes
                        
        #trying to discard bboxes based on size didn't work so this is not very used right now.
        #in the case of areas of full blackness, bboxes should be discarded there using other methods.
        area = (t_b.x2-t_b.x1+1)*(t_b.y2-t_b.y1+1)
        #print "Area:%d" % area
        if area < smallest_bbox_area:
        #if t_b.x2-t_b.x1 < 2:
            filtered_out += 1
            col = (0,0,255)
            cv2.rectangle(col_kabala, (t_b.x1,t_b.y1), (t_b.x2,t_b.y2), col)
            #print "Discaded bbox with area %d" % area
            continue
        
        col = (0,255,0)
        cv2.rectangle(col_kabala, (t_b.x1,t_b.y1), (t_b.x2,t_b.y2), col, lineType=8)
        
        t_bboxes.append(t_b)                
    
    print "Filtered out %d boxes" % filtered_out

    #split boxes suspected as sticked vertically or horizontally
    #attempt to fix cases where a single_char_bbox is actually containing more than 1 char.
    #this can happened due to "sticked" chars.
    
    splitted_char_boxes = None

    if conf_split_sticked_chars_using_heuristics == True:                
        splitted_char_boxes = fix_sticked_single_char_boxes(t_bboxes)
    else:
        splitted_char_boxes = t_bboxes
    
    #sort the single-char-bboxes
    #sorted_boxes = sort_boxes(fixed_char_boxes)
        
    zedusBoxes.save_box_file(no_ext[: -len('_Local_Threshold')]+u'.%d_%d_boxUnsorted' % (im_width, im_height), splitted_char_boxes)
    
    
  
    #cv2.drawContours(im,big_contours,-1,(0,0,255),1)
    
    #cv2.imshow("window", col_kabala)
    #cv2.waitKey()
    
    #cv2.imwrite(no_ext+u"_WithBoxes.jpg", col_kabala,[cv2.IMWRITE_JPEG_QUALITY, 90])
    cv2.imwrite(no_ext+u"_WithBoxes.bmp", col_kabala)


g_word_counter = 0

"""
#not traditional distance
def are_boxes_close_1(b1, b2):
    global current_re
    x_dist = min(abs(b1.x1 - b2.x1), abs(b1.x1 - b2.x2), abs(b1.x2 - b2.x1), abs(b1.x2 - b2.x2))
    y_dist = min(abs(b1.y2 - b2.y2), abs(b1.y2 - b2.y1), abs(b1.y1 - b2.y2), abs(b1.y1 - b2.y1))
    
    max_x_distance = int(float(im_width) * 0.015 )
    max_y_distance = 0 #int(float(im_width) * 0.002 )
    
    if (x_dist > max_x_distance):
        return False
    
    if (y_dist > max_y_distance):
        return False
    
    return True
"""

def boxes_distance(b1,b2):
    if b2.x1 > b1.x2: #if b2 is completely on the right side of b1
        if b2.y1 > b1.y2: #and under
            dir_x,dir_y = normalize_2d ( b2.x1 - b1.x2 , b2.y1 - b1.y2)
            return (math.sqrt( (b2.x1-b1.x2)*(b2.x1-b1.x2) + (b2.y1-b1.y2)*(b2.y1-b1.y2) ) , (dir_x,dir_y))
        if b2.y2 < b1.y1: #and above
            dir_x,dir_y = normalize_2d ( b2.x1 - b1.x2 , b2.y2 - b1.y1)
            return (math.sqrt( (b2.x1-b1.x2)*(b2.x1-b1.x2) + (b2.y2-b1.y1)*(b2.y2-b1.y1) ) , (dir_x,dir_y))
        dir_x,dir_y = normalize_2d ( b2.x1 - b1.x2 , 0.0)
        return (math.sqrt ( (b2.x1-b1.x2)*(b2.x1-b1.x2) ) , (dir_x,dir_y))
        
    #if b2 is completely on the left side of b1
    if b2.x2 < b1.x1:
        if b2.y1 > b1.y2: #and under
            dir_x,dir_y = normalize_2d ( b2.x2 - b1.x1 , b2.y1 - b1.y2)
            return (math.sqrt( (b2.x2-b1.x1)*(b2.x2-b1.x1) + (b2.y1-b1.y2)*(b2.y1-b1.y2) ) , (dir_x,dir_y))
        if b2.y2 < b1.y1: #and above
            dir_x,dir_y = normalize_2d ( b2.x2 - b1.x1 , b2.y2 - b1.y1)
            return (math.sqrt( (b2.x2-b1.x1)*(b2.x2-b1.x1) + (b2.y2-b1.y1)*(b2.y2-b1.y1) ) , (dir_x,dir_y))
        
        dir_x,dir_y = normalize_2d ( b2.x2 - b1.x1 , 0.0)
        return ( math.sqrt ( (b2.x2-b1.x1)*(b2.x2-b1.x1) ), (dir_x,dir_y))
        
    if b2.y2 < b1.y1: #above but not completely to the left or right
        dir_x,dir_y = normalize_2d ( 0.0 , b2.y2-b1.y1)
        return math.sqrt ( (b2.y2-b1.y1)*(b2.y2-b1.y1) ) , (dir_x,dir_y)
        
    if b2.y1 > b1.y2: #below but not completely to the left or right
        dir_x,dir_y = normalize_2d ( 0.0 , b2.y1-b1.y2)
        return math.sqrt ( (b2.y1-b1.y2)*(b2.y1-b1.y2) ) , (dir_x,dir_y)
    
    #if we reached here, it means the boxes intersect!
    return (0.0 , (0.0,0.0))

def get_vector_angle_2d(a,b):
    #a*b = |a|*|b|*cos(ang)
    #since vectors are normalized
    #cos(ang) = (a*b)
    #ang = arccos(a*b)    
    cos_ang = a[0]*b[0] + a[1]*b[1]
    rad = math.acos(cos_ang)
    #print "rad: %f" % rad
    return rad * 57.2957795
    
def normalize_2d(x,y):
    f_x = float(x)
    f_y = float(y)
    vec_len = f_x*f_x+f_y*f_y
    vec_len = math.sqrt(vec_len)
    _x = f_x / vec_len
    _y = f_y / vec_len
    
    #new_length = math.sqrt(float(_x)*_x)+(float(_y)*_y)
    #print "Length after normalizing is: %f" % new_length
    return _x,_y
    

def are_boxes_close_2(b1, b2):
    global current_re
        
        
    dbg_me = False
        
    #if b1.x1 == 385 and b1.x2 == 399 and b1.y1 == 1017:
    #    if b2.x1 == 405 and b2.y1 == 1016:
    #        print "b1=",
    #        print b1 ,
    #        print "   b2=",
    #        print b2 
    #        dbg_me = True
        
    #if b2.x1 == 385 and b2.x2 == 399 and b2.y1 == 1017:
    #    if b1.x1 == 405 and b1.y1 == 1016:
    #        print "b1=",
    #        print b1 ,
    #        print "   b2=",
    #        print b2 
    #        dbg_me = True
        
    res = boxes_distance(b1,b2)
    
    dist = res[0]
    direction = res[1]
    
    ang_deg = get_vector_angle_2d((-1.0,0.0),direction)
    
    #max_distance = int(float(im_width) * 0.012 )
    
    #to make sure that titles are also identified as words
    #notice - on purpose using height as a fraction of image width!
    #factor_based_on_box_height = 20.0 * (float(b1.y2-b1.y1+1) / im_width)  # THIS SOUNDED GOOD, but somehow didn't work
    factor_based_on_box_height = 1.0
    #print "factor_based_on_box_height=%f" % factor_based_on_box_height
    
    max_distance = int(float(im_width) * 0.015 * factor_based_on_box_height)
    
    if dist > max_distance:
        return False
    
    miftah = 32.0
    
    if ang_deg > miftah and ang_deg < 180.0 - miftah:
        return False
    
    center_b1 = ( b1.x1 + float(b1.x2-b1.x1)/2, b1.y1 + float(b1.y2-b1.y1)/2)
    center_b2 = ( b2.x1 + float(b2.x2-b2.x1)/2, b2.y1 + float(b2.y2-b2.y1)/2)
    
    centers_dir_x,centers_dir_y = normalize_2d( center_b2[0]-center_b1[0], center_b2[1]-center_b1[1])
    
    center_vec_ang_deg = get_vector_angle_2d((-1.0,0.0),(centers_dir_x,centers_dir_y))
    
    
    miftah_2 = 38.0
    
    if center_vec_ang_deg > miftah_2 and center_vec_ang_deg < 180.0 - miftah_2:
        return False
    
    if dbg_me == True:
        print "Returned True!"
    
    return True        
    

def find_word(boxes,box_index):
    global g_word_counter
    
    #this bbox already assigned a wordNumber
    if boxes[box_index].wordNumber != -1:
        return
    
    boxes[box_index].wordNumber = g_word_counter
    boxes_num = len(boxes)
    
    #for b in xrange(box_index,boxes_num):
    for b in xrange(boxes_num):
        if boxes[b].wordNumber == -1:
            if are_boxes_close_2(boxes[box_index],boxes[b]):
                find_word(boxes,b)
        
    
    
def find_chars_in_word(box, rgb_data, bw_data):
    global im_width, im_height    
    vals = []
    for x in xrange(box.x1,box.x2+1):
        for y in xrange(box.y1,box.y2+2):
            x = min(x,im_width-1)
            y = min(y,im_height-1)
            #print "%d,%d -> %d,%d (%d,%d)" % (x,y,_x,_y,im_width,im_height)
            vals.append(bw_data[x,y])
    
    a = np.array(vals)
    med = np.median(a)
    #print "Median: %f" % med
    
    for x in xrange(box.x1,box.x2+1):
        for y in xrange(box.y1,box.y2+1):        
            x = min(x,im_width-1)
            y = min(y,im_height-1)          
            if bw_data[x,y] < med - 30:
                bw_data[x,y] = 0
            else:
                bw_data[x,y] = 255
    
    columns_sum = []    
    for x in xrange(box.x1,box.x2+1):
        sum = 0        
        for y in xrange(box.y1,box.y2+1):
            x = min(x,im_width-1)
            y = min(y,im_height-1)    
            if bw_data[x,y] == 0:
                sum += 1
        columns_sum.append(sum)
    
    for x in xrange(box.x1,box.x2+1):
        x = max(x,0)
        x = min(x,im_width-1)
        y = max(box.y1-1,0)
        y = min(y,im_height-1)
        if columns_sum[x-box.x1] > 0:            
            #print "%d,%d (%d,%d)" % (x,y,im_width,im_height)
            rgb_data[x,y] = (0,0,0)
        else:
            rgb_data[x,y] = (255,255,255)
    
    #run on rows - test which rows are empty in the new threshold (local to this word)
    
    rows_sum = []
    for y in xrange(box.y1,box.y2+1):
        sum = 0        
        for x in xrange(box.x1,box.x2+1):
            x = min(x,im_width-1)
            y = min(y,im_height-1)
            if bw_data[x,y] == 0:
                sum += 1
        rows_sum.append(sum)
        
    for y in xrange(box.y1,box.y2+1):
        #val = int(255.0 * columns_sum[x-box.x1])
        #rgb_data[x,box.y1] = (val,val,val)
        #if columns_sum[x-box.x1] > col_med + 10:
        x = min(box.x1-1,im_width-1)
        x = max(x,0)
        y = min(y,im_height-1)
        y = max(y,0)
        
        if rows_sum[y-box.y1] > 0:
            rgb_data[x,y] = (0,0,0)
        else:
            rgb_data[x,y] = (255,255,255)
            
    
    #remove empty lines from the top and from the bottom, based on the horizontal histogram.
    empty_lines_at_the_top_num = 0
    for r in rows_sum:
        if r==0:
            empty_lines_at_the_top_num += 1
            # mark what we removed:
            for x in xrange(box.x1,box.x2+1):
                x = min(x,im_width-1)
                y = min(box.y1+empty_lines_at_the_top_num-1,im_height-1)
                rgb_data[x,y] = (0,255,255)
        else:
            break
            
    
    empty_lines_at_the_bottom_num = 0
    for r in reversed(rows_sum):
        if r==0:
            empty_lines_at_the_bottom_num += 1
            # mark what we removed:
            for x in xrange(box.x1,box.x2+1):
                x = min(x,im_width-1)
                y = min(box.y2-empty_lines_at_the_bottom_num+1,im_height-1)
                rgb_data[x,y] = (0,255,255)
        else:
            break
    
    box.y1+= empty_lines_at_the_top_num
    if box.y1 > box.y2:
        box.y1 = box.y2
        
    box.y2-= empty_lines_at_the_bottom_num
    
    if box.y2 < box.y1:
        box.y2 = box.y1
    
    if box.y1 > box.y2:
        print "find_chars_in_word::error! box.y1 > box.y2"
        
    

#searches if box intersects with any of the boxes
#discards if not inside a word box
"""def stretch_boxes_intersecting_with(boxes,box):
    for b in boxes:
        if check_intersection(b,box):
            stretch_height(box,b)
            return True            
    return False
    """
def check_if_box_intersect_any(boxes,box):
    for b in boxes:
        if check_intersection(b,box):
#            stretch_height(box,b)
            return True            
    return False


def stretch_height(stretch_me_box, stretch_to_box):
    if not check_intersection(stretch_me_box,stretch_to_box):
        print "stretch_height:: Error! trying to stretch a box that doesn't intersect with the other."
        return

    #if out top is below it - stretch up to it
    if stretch_me_box.y1 > stretch_to_box.y1:
        stretch_me_box.y1 = stretch_to_box.y1
        
    #if out bottom is above it - stretch down to it
    if stretch_me_box.y2 < stretch_to_box.y2:
        stretch_me_box.y2 = stretch_to_box.y2
    
    stretch_me_box.y1 = stretch_to_box.y1
    stretch_me_box.y2 = stretch_to_box.y2

def mark_boxes_on_image(rgb_im_data, boxes):
    for b in boxes:
        mark_box_on_image(rgb_im_data,b)
        """
        for y in xrange(b.y1,b.y2+1):
            for x in xrange(b.x1,b.x2+1):
                rgb_im_data[x,y] = (rgb_im_data[x,y][0],max(255,rgb_im_data[x,y][1]+35),rgb_im_data[x,y][2])
                #rgb_im_data[x,y] = (255,0,255)
        """
            
        

def mark_box_on_image(rgb_im_data, box, col=None):
    for y in xrange(box.y1,box.y2+1):
        for x in xrange(box.x1,box.x2+1):            
            if col == None:
                rgb_im_data[x,y] = (rgb_im_data[x,y][0],max(255,rgb_im_data[x,y][1]+35),rgb_im_data[x,y][2])
            else:
                rgb_im_data[x,y] = col
            #rgb_im_data[x,y] = (255,0,255)

def find_words(box_file):
    global g_word_counter
    global im_width, im_height
    
    g_word_counter = 0
    
    """im = Image.open(box_file+".jpg")
    im_data = im.load()
    
    bw_im = im.convert('L')
    bw_im_data = bw_im.load()"""
    
    bw_im = Image.open(box_file+".bmp")
    bw_im_data = bw_im.load()
    
    im = bw_im.convert('RGB')
    im_data = im.load()
    
    
    
    im_width, im_height = im.size
    
    boxes = zedusBoxes.read_box_file(box_file+".%d_%d_boxFillMe" % (im_width, im_height))
    
    if (len(boxes) == 0):
        print "find_words::Error! didn't find any boxes!!!"
    
    boxes_num = len(boxes)
    
    #for i in xrange(1000):
    while True:
        found_numberless_word = False
        for b in xrange(boxes_num):
            if boxes[b].wordNumber == -1:
                found_numberless_word = True
                find_word(boxes,b)
                #print "Assigned %d wordNumber" % g_word_counter
                g_word_counter += 1
        if False == found_numberless_word:
            break

    merged_boxes = []

    #minimum_word_total_pixel_count = int(float(im_width) / 8.0)
    minimum_word_total_pixel_count = int(float(im_width) / 15.0)
    print "zzz: minimum pixel count to keep word bbox is %d" % minimum_word_total_pixel_count

    
    
    #create the bboxes of the full words
    for i in xrange(g_word_counter):
        first = True
        merged_box = None
        for b in boxes:
            if b.wordNumber == i:
                if True == first:
                    merged_box = b.clone()
                    first = False
                else:                    
                    merged_box.merge_with(b)
        if merged_box == None:
            print "zomg!"
            
        #discard word bbox if it's too small
        area = (merged_box.x2-merged_box.x1+1)*(merged_box.y2-merged_box.y1+1)
        if area < minimum_word_total_pixel_count:
            print "discarded for too small word pixel count (%d) min allowed is %d" % (area, minimum_word_total_pixel_count) ,
            print merged_box
            mark_box_on_image(im_data,merged_box,(255,0,0))
            continue
        merged_boxes.append(merged_box)                
    
    
    for b in merged_boxes:
        if b.y1 > b.y2:
            print "Error! b.y1 > b.y2 !!! ",
            print b
    
    bw_im_with_read_write_access = bw_im.convert('L')
    bw_im_with_read_write_access_data = bw_im_with_read_write_access.load()
    
    for b in merged_boxes:
        find_chars_in_word(b,im_data,bw_im_with_read_write_access_data)
        #print "dbg"
        

    zedusBoxes.save_box_file(box_file+".%d_%d_boxFullWords" % (im_width, im_height), merged_boxes)
                
        
    char_boxes_inside_words = []
    print "Discarding character boxes that aren't inside a word bbox."
    #dbg_count = 0
    for b in boxes: # for every char bbox
        
        #if dbg_count%30 == 0:
        #    print dbg_count
        
        if check_if_box_intersect_any(merged_boxes,b) == True:        
            char_boxes_inside_words.append(b)
        
        #dbg_count+=1
        
                        
    mark_boxes_on_image(im_data,char_boxes_inside_words)
    zedusBoxes.save_box_file(box_file+".%d_%d_boxCharsAfterDiscardingNonWordsBBoxes" % (im_width, im_height), char_boxes_inside_words)
    
    im.save(box_file+"_Histogram.png")    
    bw_im.save(box_file+"_PerWordMedianThreshold.png")
                
    print "Done grouping boxes into words."
                
    return im_width, im_height
                
    
def stretch_char_boxes_based_on_word_boxes(words_box_file , chars_box_file):
    words_boxes = zedusBoxes.read_box_file(words_box_file)
    chars_boxes = zedusBoxes.read_box_file(chars_box_file)    
    res = []    
    for w in words_boxes:
        curr_word = []
        for c in chars_boxes:                        
            if c.selected == True:
                continue
            if check_intersection(w,c) == True:
                c.selected = True
                c.groupNumber = w.groupNumber #shouldn't this be wordNum ??
                
                height = c.y2-c.y1
                if height < float(w.y2-w.y1) * 0.65:                                    
                    stretch_height(c, w)
                curr_word.append(c)                
        #here i should sort this word chars
        res.extend(curr_word)    
    return res


#starting from an optional column separator (between sticked chars)
#try to choose the best suitable column seperator in the close environment

def select_most_likely_separator_column(bw_data,x_candidate, start_y, range_x, height):
    global im_width,im_height
    max_gray_column = -1
    max_column_x = -1
    for i in xrange(range_x*2):
        x_off = i - range_x
        curr_x = x_candidate+x_off
        #make sure we're not out of bounds
        curr_x = max(0,curr_x)
        curr_x = min(im_width-1,curr_x)
        lum_sum = 0
        for y in xrange(start_y, start_y+height+1):
            lum_sum += int(bw_data[curr_x,y])
        
        if lum_sum > max_gray_column:
            max_gray_column = lum_sum
            max_column_x = curr_x
    
    return max_column_x              

def split_box_horizontally(b, splits_num):
    print "Splitting: ",
    print b                
    #splits_num = math.floor((width / avg)+0.75)
    #splits_num = int(splits_num)
    
    part_width = int (float(b.x2-b.x1) / float(splits_num))

    print "Into %d boxes:" % splits_num
    
    split_boxes = []
    prev_start = b.x1 - 1
    for i in xrange(splits_num):
        partial_box = b.clone()                    
        #curr_width = math.floor(part_width+0.5)                    
        
        partial_box.x1 = prev_start+1
        partial_box.x2 = partial_box.x1+int(part_width)
        if partial_box.x2 >= b.x2:
            partial_box.x2 = b.x2
        elif i==splits_num-1:
            print "Error! partial_box.x2=%d and b.x2=%d" % (partial_box.x2, b.x2)
            partial_box.x2 = b.x2
         
        prev_start = partial_box.x2
        
        #make sure we don't go above the original box
        partial_box.x2 = min(partial_box.x2, b.x2)
        partial_box.x1 = min(partial_box.x1, b.x2)
                            
        #do small corrections to make sure we are in a reasonable split point
        #if i!=splits_num-1:
        #    partial_box.x2 = select_most_likely_separator_column(bw_data,partial_box.x2,partial_box.y1,2,partial_box.y2-partial_box.y1)
        
        if partial_box.x2 < partial_box.x1:
            partial_box.x2 = partial_box.x1
        
        partial_box.x2 = min(partial_box.x2, b.x2)
        partial_box.x1 = min(partial_box.x1, b.x2)
        
        prev_start = partial_box.x2
        
        print "\t",
        print partial_box
        
        if b.x1 > b.x2:
            print "Error! b.x1 > b.x2"
        
        split_boxes.append(partial_box)
    return split_boxes

    
    
def split_box_vertically(b, splits_num):
    print "Splitting vertically: ",
    print b                
    #splits_num = math.floor((width / avg)+0.75)
    #splits_num = int(splits_num)
    print "Into %d boxes:" % splits_num
    
    part_width = int (float(b.y2-b.y1) / float(splits_num))
    
    split_boxes = []
    prev_start = b.y1 - 1
    for i in xrange(splits_num):
        partial_box = b.clone()                    
        #curr_width = math.floor(part_width+0.5)                    
        
        partial_box.y1 = prev_start+1
        partial_box.y2 = partial_box.y1+int(part_width)
        if partial_box.y2 >= b.y2:
            partial_box.y2 = b.y2
        elif i==splits_num-1:
            print "Error! partial_box.y2=%d and b.y2=%d" % (partial_box.y2, b.y2)
            partial_box.y2 = b.y2
         
        prev_start = partial_box.y2
        
        #make sure we don't go above the original box
        partial_box.y2 = min(partial_box.y2, b.y2)
        partial_box.y1 = min(partial_box.y1, b.y2)
                            
        #do small corrections to make sure we are in a reasonable split point
        # remember - this had a nice effect! consider restoring it !!
        #if i!=splits_num-1:
        #    partial_box.y2 = select_most_likely_separator_column(bw_data,partial_box.y2,partial_box.y1,2,partial_box.y2-partial_box.y1)
        
        if partial_box.y2 < partial_box.y1:
            partial_box.y2 = partial_box.y1
        
        partial_box.y2 = min(partial_box.y2, b.y2)
        partial_box.y1 = min(partial_box.y1, b.y2)
        
        prev_start = partial_box.y2
        
        print "\t",
        print partial_box
        
        if b.y1 > b.y2:
            print "Error! b.y1 > b.y2"
        
        split_boxes.append(partial_box)
    return split_boxes
    


def fix_sticked_single_char_boxes(boxes):
    global im_width,im_height   

    average_width = 0
    average_height = 0
    
    for b in boxes:
        average_width += b.x2 - b.x1
        average_height += b.y2 - b.y1
            
    average_width = int(float(average_width) / float(len(boxes)))
    average_height = int(float(average_height) / float(len(boxes)))
    
    width_extra = 0.6
    height_extra = 0.6
    
    after_split_vertically_boxes = []    
    #fix vertically wise
    
    for b in boxes:
        height = b.y2-b.y1
        
        curr_to_avg_ratio_height = (float(height) / float(average_height)) - 0.6                        
        if curr_to_avg_ratio_height > 2.0:
            splits_num = int(  math.floor(curr_to_avg_ratio_height) )
            split_boxes = split_box_vertically(b,splits_num)
            after_split_vertically_boxes.extend(split_boxes)
        else:
            after_split_vertically_boxes.append(b)
            
    
    after_split_horizontally_boxes = []
    #fix width wise
    
    for b in after_split_vertically_boxes:
        width = b.x2-b.x1
       
        curr_to_avg_ratio_width = (float(width) / float(average_width)) - 0.6
        if curr_to_avg_ratio_width > 2.0:
        #if True == False:
            splits_num = int(  math.floor(curr_to_avg_ratio_width) )
            split_boxes = split_box_horizontally(b,splits_num)
            after_split_horizontally_boxes.extend(split_boxes)
        else:
            after_split_horizontally_boxes.append(b)

    return after_split_horizontally_boxes


def sort_boxes(boxes):
    lines_map = {}
    #separate first into lines
    for b in boxes:
        if not lines_map.has_key(b.groupNumber):
            lines_map[b.groupNumber] = []
        lines_map[b.groupNumber].append(b)
    
    #now sort every group within itself - go right to left
    #for k in lines_map:
    #    lines_map[k] = sorted(lines_map[k], key=lambda b: b.x1)
            
    lines = []
    for k in lines_map:        
        words_inside_line_map = {}
        
        words = lines_map[k]
        
        curr_chars = []
        
        if zedusConfig.sort_left_to_right == True:
            sorted_words = sorted(words, key= lambda b: b.x1)
        else:
            sorted_words = sorted(words, key= lambda b: -b.x1)
        
        lines.append((sorted_words,sorted_words[0].y1))
        
        """
        for w in words:
            if not words_inside_line_map.has_key(w.wordNumber):
                words_inside_line_map[w.wordNumber] = []
            words_inside_line_map[w.wordNumber].append(w)
        
        words_to_sort = []
        
        for x in words_inside_line_map:
            s = sorted(words_inside_line_map[x], key=lambda b: -b.x1)
            words_to_sort.append((s , words_inside_line_map[x][0].x1)) #should change to average
        
        #sort between words
        sorted_words = sorted(words_to_sort, key=lambda w: -w[1])        
        #sorted_words - each element is [list of boxes inside word, word x
        
        curr_line = []
        for w in sorted_words:
            #now sort inside word
            word_chars = w[0]
            sorted_word_chars = sorted(word_chars, key=lambda b: b.x1)
            
            curr_line.extend(sorted_word_chars)                        
                
        
        lines.append((curr_line,sorted_words[0][0][0].y1))
        
        """
        
        
                
        
        
        #now words_inside_line_map contains list of boxes per word, which are sorted right to left
        
        #curr_line_boxes = lines_map[k]        
        #simply using the first box height for now, should change to average height later
        #lines.append((curr_line_boxes, curr_line_boxes[0].y1))
        
        
        #curr_line_boxes
    
    #sort between lines
    lines = sorted(lines, key=lambda l: l[1])
    
    res = []
    for l in lines:
        for b in l[0]:
            res.append(b)
            
    return res        
    
    
def find_bboxes():    
    global images_path
    global im,im_width, im_height
    global calc_threshold
    global calc_find_boxes
    global calc_sort_boxes
    global conf_stretch_char_boxes
    global conf_split_sticked_chars_using_heuristics
    
    
    #convert from color jpg to greyscale bmp
    
    if True == calc_convert_to_greyscale:
        t1 = time.time()
        #print "Median threshold calculation start time: ",
        #print t
        for root, _, files in os.walk(images_path):
            for f in files:
                file_lwr = f.lower()
                spl = file_lwr.split('.')
                #if not 'jpg' in spl[-1] or 'WithBoxes' in f or 'Boxes_Data' in f or 'Local_Threshold' in f:
                #if not 'bmp' in spl[-1] or not 'receipt_grey_' in file_lwr or 'MedianThreshold' in f or 'WithBoxes' in f or 'Boxes_Data' in f or 'Local_Threshold' in f:
                if not 'jpg' in spl[-1] or 'MedianThreshold' in f or 'WithBoxes' in f or 'Boxes_Data' in f or 'Local_Threshold' in f:
                    continue
    
                print u'Working on: %s - Creating greyscale BMP version' % f
                fullpath = os.path.join(root, f)
        
                no_ext = fullpath[0:-4]
                               
                im = Image.open(fullpath)
                im = im.convert('L')
                im.save(no_ext+".bmp","BMP")
        t2 = time.time()
        #print "Median threshold calculation end time: ",
        #print t2
        print "zedusFindChars:calc_convert_to_greyscale:Convert to greyscale time: ",
        print (t2-t1) / 60.0 ,
        print "Minutes."
    
    #create thresholds - median
    
    if True == calc_threshold:
        t1 = time.time()
        #print "Median threshold calculation start time: ",
        #print t
        for root, _, files in os.walk(images_path):
            for f in files:
                file_lwr = f.lower()
                spl = file_lwr.split('.')
                #if not 'jpg' in spl[-1] or 'WithBoxes' in f or 'Boxes_Data' in f or 'Local_Threshold' in f:
                #if not 'bmp' in spl[-1] or not 'receipt_grey_' in file_lwr or 'MedianThreshold' in f or 'WithBoxes' in f or 'Boxes_Data' in f or 'Local_Threshold' in f:
                if not 'bmp' in spl[-1] or 'MedianThreshold' in f or 'WithBoxes' in f or 'Boxes_Data' in f or 'Local_Threshold' in f:
                    continue
    
                print u'Working on: %s - Creating Adaptive Threshold' % f
                fullpath = os.path.join(root, f)
        
                no_ext = fullpath[0:-4]
                
                """                
                im = Image.open(fullpath)
                im_width, im_height = im.size
                #bw = im.convert('L')            
                #threshold_image(bw, no_ext+u'_Local_Threshold.BMP')
                threshold_image(im, no_ext+u'_Local_Threshold.BMP')
                """                
                threshold_image_cuda(no_ext)
        t2 = time.time()
        #print "Median threshold calculation end time: ",
        #print t2
        print "zedusFindChars:Median threshold calculation time: ",
        print (t2-t1) / 60.0 ,
        print "Minutes."
    
    if True == calc_find_boxes:
        t1 = time.time()
        #print "Find boxes start time: ",
        #print t
        for root, _, files in os.walk(images_path):
            for f in files:
                file_lwr = f.lower()
                spl = file_lwr.split('.')
                #if not 'bmp' in spl[-1] or not 'Local_Threshold' in f:
                if not 'bmp' in spl[-1] or not 'MedianThreshold' in f:
                    continue
                
                if "WithBoxes" in f:
                    continue
                
                print u'Working on: %s - Finding BBoxes' % f
                fullpath = os.path.join(root, f)
        
                no_ext = fullpath[0:-4]
                
                find_boxes_cv2_based(no_ext)
        t2 = time.time()
        #print "Find boxes end time: ",
        #print t2
        print "zedusFindChars:Find boxes time: ",
        print (t2-t1) / 60.0 ,
        print "Minutes."

    ########################################################
    #find words
    #in this section i try to find full bboxes of words
    #I also use those full-word-bboxes to streth the single-char-bboxes
    #it helps detecting between yud,vav,nun sofit, dot etc.
    #because the char bbox is no longer too tight.
    ########################################################
    if True == calc_find_words:
        t1 = time.time()
        #print "Find words bboxes (and change single-char-boxes accordingly) start time: ",
        #print t
        for root, _, files in os.walk(images_path):
            for f in files:                
                #if not 'boxFillMe' in f:
                if not 'boxUnsorted' in f:
                    continue                
                #file_lwr = f.lower()
                #spl = file_lwr.split('.')                
                print u'Working on: %s - Finding words' % f            
                fullpath = os.path.join(root, f)
                
                spl = fullpath.split('.')
                extension_length = len(spl[-1])+1
                no_ext = fullpath[0:-extension_length]   
                im = Image.open(no_ext+".bmp")
                
                #find the full words
                #width,height = find_words(fullpath[0:-extension_length])
                
                #input_box_name = fullpath[0:-extension_length]+".%d_%d_boxFillMe" % (im.size[0], im.size[1])
                input_box_name = fullpath[0:-extension_length]+".%d_%d_boxUnsorted" % (im.size[0], im.size[1])
                
                chars_only_in_word_bbox_box_file_extension = "%d_%d_boxCharsAfterDiscardingNonWordsBBoxes" % (im.size[0], im.size[1])
                
                print 'About to run "BoxFindWords.exe"'
                
                exe_name = "BoxFindWords"                
                if os.name != 'posix':
                    exe_name+= ".exe"
                
                if zedusConfig.fully_local == True:
                    subprocess.call([exe_name, input_box_name, chars_only_in_word_bbox_box_file_extension])
                else:                    
                    subprocess.call([zedusConfig.path_to_BoxFindWords, input_box_name, chars_only_in_word_bbox_box_file_extension])
                                
                #sort the word-bboxes into lines                
                print "Calculating line numbers on the .boxFullWords"                                
                full_words_box_file_path = fullpath[0:-extension_length] + ".%d_%d_boxFullWords" % (im.size[0], im.size[1])
                
                print 'About to run "BoxOrganizer.exe"'
                
                exe_name = "BoxOrganizer"                
                if os.name != 'posix':
                    exe_name+= ".exe"
                
                if zedusConfig.fully_local == True:
                    subprocess.call([exe_name, full_words_box_file_path, "sortedFullWords", "1"])
                else:                    
                    subprocess.call([zedusConfig.path_to_BoxOrganizer, full_words_box_file_path, "sortedFullWords", "1"])
                
                
                words_boxes_file = fullpath[0:-(extension_length)]+".sortedFullWords"                
                single_chars_file  = fullpath[0:-(extension_length)]+(".%d_%d_boxCharsAfterDiscardingNonWordsBBoxes" % (im.size[0], im.size[1]))
                
                #stretch the single-char-bboxes based on the full-word-boxes (vertically)
                #also discard single-char-bboxes that aren't inside a full-word-box.
                """ #moved to doing this in C++
                if conf_stretch_char_boxes == True:
                    stretched_char_boxes = stretch_char_boxes_based_on_word_boxes(words_boxes_file , single_chars_file)
                else:
                    stretched_char_boxes = zedusBoxes.read_box_file(single_chars_file)
                """                
                stretched_char_boxes = zedusBoxes.read_box_file(single_chars_file)

                final_char_boxes_file = fullpath[0:-(extension_length)]+(".%d_%d_boxFindWordsResult" % (im.size[0], im.size[1]))
                #zedusBoxes.save_box_file(fully_sorted_char_boxes_file ,sorted_boxes)
                zedusBoxes.save_box_file(final_char_boxes_file ,stretched_char_boxes)
                
                
                
        t2 = time.time()
        #print "Find words bboxes (and change single-char-boxes accordingly) end time: ",
        #print t2
        print "Find words bboxes (and change single-char-boxes accordingly) time: ",
        print (t2-t1) / 60.0 ,
        print "Minutes."
        
        #sort
    if True == calc_sort_boxes:
        t1 = time.time()
        #print "Sort boxes start time: ",
        #print t
        for root, _, files in os.walk(images_path):
            for f in files:
                #file_lwr = f.lower()
                #spl = file_lwr.split('.')
                #if not 'boxUnsorted' in f:
                if not 'boxFindWordsResult' in f:
                    continue
                
                print u'Working on: %s - Sorting boxes' % f            
                fullpath = os.path.join(root, f)
                
                spl = fullpath.split(".")
                extension_length = len(spl[-1])+1
                no_ext = fullpath[0:-extension_length]                             
                im = Image.open(no_ext+".bmp")
                
                print 'About to run "BoxOrganizer.exe"'
                
                exe_name = "BoxOrganizer"                
                if os.name != 'posix':
                    exe_name+= ".exe"
                
                if zedusConfig.fully_local == True:
                    #subprocess.call([r"BoxOrganizer.exe", fullpath, "%d_%d_boxFillMe" % (im.size[0],im.size[1]), "1"])
                    subprocess.call([exe_name, fullpath, "boxWithWordsID", "1"])
                else:
                    #subprocess.call([r"C:\google_code\dooake\OCR\BoxOrganizer\Release\BoxOrganizer.exe", fullpath, "%d_%d_boxFillMe" % (im.size[0],im.size[1]), "0"])
                    subprocess.call([zedusConfig.path_to_BoxOrganizer, fullpath, "boxWithWordsID", "0"])
                    
                    
                #load -
                boxes = zedusBoxes.read_box_file(no_ext+".boxWithWordsID")
                #sort the single-char-bboxes
                sorted_boxes = sort_boxes(boxes)
                
                zedusBoxes.save_box_file(no_ext+".%d_%d_boxFillMe" % (im.size[0],im.size[1]), sorted_boxes)
                
                
        t2 = time.time()
        #print "Sort boxes end time: ",
        #print t2
        print "zedusFindChars:Sort boxes time: ",
        print (t2-t1) / 60.0 ,
        print "Minutes."
    
    """
    if True == calc_detect_chars_in_words_bbox:
        for root, _, files in os.walk(images_path):
            for f in files:                
                if not 'boxFillMe' in f:
                    continue                
                #file_lwr = f.lower()
                #spl = file_lwr.split('.')                
                #print u'Working on: %s - Finding words' % f            
                fullpath = os.path.join(root, f)
    """


def main(path):
    global images_path
    images_path = path
    find_bboxes()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "Not enough args!"
    main(sys.argv[1])


print "Done."

