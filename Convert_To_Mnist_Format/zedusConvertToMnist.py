#!/usr/bin/env python
from PIL import Image
import ImageDraw
import ImageFont
import os
import sys
import struct
import zedusBoxes
from array import array
import zedusConfig
if zedusConfig.support_gui == True:
	import Tkinter, tkFileDialog, tkMessageBox
import cv2
import numpy as np
from zedusCharacterDict import *
#import subprocess
from ctypes import *
#from docx import *


#########################################

znnLib = None

train_on_white_space = False
save_dbg_images = False

save_visualization_images = True

if zedusConfig.support_gui == True:
	root = Tkinter.Tk()

test_cases_total = 0

visualization_factor = 1

db_file_img = None
db_file_label = None
ai_results_list = None

results_file_opened = False

test_case_width = 28
test_case_height = 28
#test_case_extra_features = 2

current_character_global_index = 0

current_resolution = (0,0)

test_data_dir = ""
NN_file = ''


def draw_box(im, box, col):
    global visualization_factor
    global current_resolution
    #return
    for i in xrange(box.x1*visualization_factor,(box.x2+1)*visualization_factor):
        x = max(0, i)
        x = min(x,current_resolution[0]-1)
        y2 = max(0, (box.y2+1))
        y2 = min(y2,current_resolution[1]-1)
        im.putpixel((i,y2*visualization_factor), col)        
        y1 = max(0, (box.y1))
        y1 = min(y1,current_resolution[1]-1)
        im.putpixel((i,y1*visualization_factor), col)
    
    for i in xrange((box.y1)*visualization_factor,(box.y2+1)*visualization_factor):
        y = max(0, i)
        y = min(y,current_resolution[1]-1)        
        x1 = max(0, box.x1)
        x1 = min(x1,current_resolution[0]-1)        
        im.putpixel((x1*visualization_factor,y), col)
        x2 = max(0, box.x2+1)
        x2 = min(x2,current_resolution[0]-1)        
        im.putpixel(((x2)*visualization_factor,y), col)
    

"""
d = docx.open('template.docx') # Open an (optional) template where we will add our document at the end,
d += docx.Header(1, 'Introduction') # Header, levels 1..4
d += 'This is an example of a paragraph'
d += docx.newPage()
d += docx.Header(1, 'Summary')
d += 'Example of a list:'
d += docx.List([
'Item1', 'Item2',
docx.List(['Subitem1', 'Subitem2'])
])
d += docx.Table([
[ docx.Paragraph(docx.Text('H1', bold=True), align='center'),
docx.Paragraph(docx.Text('H2', bold=True), align = 'center')],
['1-1', '2-2'], ['2-1', '2-2'] ], caption = 'A table')
docx.writeto('Dokument.docx')
docx.close()
"""

def add_boxes(im, boxes, kabala_name):
    global current_character_global_index
    global visualization_factor
    global current_resolution
    global save_visualization_images
    
    ai_results_boxes = []
    
    draw = ImageDraw.Draw(im)
    
    average_x_size = 0
    average_y_size = 0
    
    for b in boxes:
        average_x_size += b.x2-b.x1
        average_y_size += b.y1-b.y2
    
    average_x_size /= len(boxes)
    average_y_size /= len(boxes)
    
    """font_x = float(average_x_size) * 0.7
    font_x = int(font_x)
    font_y = float(average_y_size) * 0.7
    font_y = int(font_y)"""
    font_size = 15
        
    
    # Default set of relationshipships - the minimum components of a document
    #relationships = relationshiplist()

    # Make a new document tree - this is the main part of a Word document
    #document = newdocument()

    # This xpath location is where most interesting content lives
    #body = document.xpath('/w:document/w:body', namespaces=nsprefixes)[0]

    # Append two headings and a paragraph
    #body.append(heading("Welcome to Python's docx module", 1))
    #body.append(heading('Make and edit docx in 200 lines of pure Python', 2))    
    
    #text = ""
    curr_line = ""
    text_lines = []
    
    curr_group_num = -1
    curr_word_num= -1
    
    #font = ImageFont.truetype(u"C:/Windows/Fonts/AdobeHebrew-Regular.otf", 16, encoding="UTF-8")

    font_name = ""

    if os.name == 'posix':
        font_name = zedusConfig.home_path+"/fonts/arial.ttf"
    else:
        font_name = u"C:/Windows/Fonts/arial.ttf"

    print "About to load font file [%s]" % font_name

    font = ImageFont.truetype(font_name, font_size, encoding="UTF-8")    
    font_big = ImageFont.truetype(font_name, font_size, encoding="UTF-8")        
    
    first_character_in_line = True
    avg_line_box_width = 0
    current_line_boxes_num = 0
    prev_box = None
    first_char = None
    first_line = True
    
    #get average lines character widths
    lines_avg_char_width = []    
    
    global_average_char_width = 0
    global_char_count = 0
    
    curr_word_avg_char_width = 0
    for b in boxes:        
        global_char_count += 1
        if b.groupNumber != curr_group_num:             
            if  curr_group_num != -1:          
                avg_char_width = int (float(curr_word_avg_char_width) / float(current_line_boxes_num) )
                #print "%d distance on %d boxes = %d" % (curr_word_avg_char_width,current_line_boxes_num,avg_char_width)
                lines_avg_char_width.append( avg_char_width)
                current_line_boxes_num = 0
                curr_word_avg_char_width = 0
            curr_group_num = b.groupNumber
        
        curr_word_avg_char_width += b.x2-b.x1
        global_average_char_width += b.x2-b.x1
        current_line_boxes_num+= 1
    
    
    global_average_char_width = int (float(global_average_char_width) / float(global_char_count) )
    #global_average_char_width = int(1.1 * float(global_average_char_width))
    
    lines_avg_char_width.append( int (float(curr_word_avg_char_width) / float(current_line_boxes_num) ))
    
    print "Found average character for %d lines." % len(lines_avg_char_width)
    
    print "Global average character width is %d" % global_average_char_width
    
    curr_group_num = -1
    curr_word_num= -1
    current_line_boxes_num = 0
    current_line_number = -1
    
    first_word_in_line = True
    
    first_word_in_line_id = -1
    
    for b in boxes:
        box_color = (0,0,255)
        
        #name = u''
        #name += b.char
        #draw.text((b.x1-5, b.y2-5), b.char, (255,0,0), font=font)
        
        if train_on_white_space == False:
            if b.char == " ":
                print "Abandoned BBox because it was whitespace."
                continue
        
        
        if b.char.decode('UTF-8') != u'@':
            db_char = b.char.decode("UTF-8")
            """if db_char == " ":
                db_char = "~"
            """
            if save_visualization_images == True:
                draw.text(( (b.x1)*visualization_factor-(font_size/2), (b.y2+2)*visualization_factor), db_char, (0,0,255), font=font)
                                    
        if True==results_file_opened:
            if (current_character_global_index > len(ai_results_list) -1 ):
                print "Error! not enough results on the ai_results file!"
                print "len(ai_results_list)=%d, current_character_global_index=%d" % (len(ai_results_list), current_character_global_index)
                sys.exit(0) 
            ai_result_key = ai_results_list[current_character_global_index]
            
            
            #we moved to a new line
            if curr_group_num != b.groupNumber:
                curr_group_num = b.groupNumber
                
                current_line_number += 1
                
                first_word_in_line_id = b.wordNumber
                
                #print "Current line number:%d" % current_line_number                
                #print "Line %d - Average char size: %d" % (current_line_number,lines_avg_char_width[current_line_number])
                
                if first_line == False:
                    #before we add a new line, add trailing space
                                                                                
                    
                    #space_boxes_in_the_end_of_line = (current_resolution[0]-prev_box.x1) / lines_avg_char_width[current_line_number]
                    space_boxes_in_the_end_of_line = (current_resolution[0]-prev_box.x1) / global_average_char_width
                    
                    #space_boxes_in_the_beginning_of_line = (first_char.x2) / lines_avg_char_width[current_line_number]
                    space_boxes_in_the_beginning_of_line = (first_char.x2) / global_average_char_width
                    
                    leading_space = ""
                    trailing_space = ""
                    
                    for w in xrange(space_boxes_in_the_beginning_of_line):
                        leading_space += " "
                                    
                    for w in xrange(space_boxes_in_the_end_of_line):
                        trailing_space += " "
                                    
                    if current_line_boxes_num > 2:
                        text_lines.append(leading_space+curr_line+trailing_space)
                    else:
                        #print "Discarded line"
                        pass
                    #text_lines.append(trailing_space+curr_line+leading_space)
                    curr_line = ""
                    
                    first_character_in_line = True
                    current_line_boxes_num = 0
                    #f.write("\r\n")
                else:
                    first_line = False
                
                first_word_in_line = True
                
                
                
                
            if first_character_in_line == True:
                first_char = b
                first_character_in_line = False
                
                
            avg_line_box_width += b.x2-b.x1
                
                
            if curr_word_num != b.wordNumber:
                curr_word_num = b.wordNumber
                
                if first_word_in_line == True:                    
                    first_word_in_line = False
                else:
                    #if prev_box.x1 > b.x2:
                    if b.wordNumber!=first_word_in_line_id:
                        #print "white space delta %d" % (b.x1 - prev_box.x2)
                        #space_boxes_from_prev_word = (b.x2 - prev_box.x1) / lines_avg_char_width[current_line_number]
                        space_boxes_from_prev_word = (b.x1 - prev_box.x2) / global_average_char_width
                        #print "spaces from prev word=%d" % space_boxes_from_prev_word
                        
                        for w in xrange(space_boxes_from_prev_word):
                            curr_line += " "
                
                #curr_line += " "
                
                
            
            if ubyte_to_unicode_mapper.has_key(ai_result_key):
                ai_result_string = ubyte_to_unicode_mapper[ai_result_key][0]
                                               
                #f.write(ai_result_string.encode('utf-8'))
                
                curr_line += ai_result_string.encode('utf-8')
                """if ai_result_string[0] >= '0' and ai_result_string[0] <= '9':
                    curr_line += ai_result_string.encode('utf-8')
                elif ai_result_string == ".":
                    curr_line += ai_result_string.encode('utf-8')
                else:
                    curr_line += "?"""""
                
                
                ai_res_col = (0,255,0)
                box_color = (0,255,0)
                if ai_result_string != b.char.decode('UTF-8'):
                    ai_res_col = (255,0,0)
                    box_color = (255,0,0)
                    
                    error_char = ai_result_string
                    
                    if ai_result_string == " ":
                        error_char = "~"
                    
                    if save_visualization_images == True:
                        #draw.text(( (b.x2)*visualization_factor+(font_size/2), (b.y2+2)*visualization_factor), error_char, (255,0,0), font=font)
                        draw.text(( (b.x1)*visualization_factor+(font_size/4), (b.y2+2)*visualization_factor), error_char, (255,0,0), font=font)
                    
                #draw.text(( (b.x1)*visualization_factor+font_size, (b.y2)*visualization_factor), ai_result_string, (255,0,0), font=font)
                    
                    #print ai_result_string.encode("UTF-8")
                
                ai_box = zedusBoxes.BBox(b.x1,b.y2,b.x2,b.y1, ai_result_string.encode('UTF-8'), 0, False)
                ai_results_boxes.append(ai_box)
            else:
                #f.write("?")
                curr_line += "?".encode('utf-8')
                
                #draw.text(( (b.x2-5)*visualization_factor, (b.y1-5)*visualization_factor), 'none'.decode("UTF-8"), (255,0,0), font=font)
                box_color = (255,0,0)
                
                ai_box = zedusBoxes.BBox(b.x1,b.y2,b.x2,b.y1, 'none'.encode('UTF-8'), 0, current_resolution, False)
                ai_results_boxes.append(ai_box)
        current_character_global_index += 1
        
        if save_visualization_images == True:
            draw_box(im, b, box_color)
        prev_box = b
        current_line_boxes_num += 1
        #ai_results_boxes.append()
    
    
    f = open(kabala_name+".txt","w")
    print "**** Final Result ***"
    for l in text_lines:
        print l
        #f.write("(התחלה) ")
        f.write(l)
        #f.write(" (סיום) ")
        f.write("\r\n".encode('utf-8'))
    #f.write()
    f.close()
    
    print "*********************"
        
    
    return im , ai_results_boxes

dbg_dict = {}



#encode (our own method) the unicode character into an unsigned byte
#note: the char might be hebrew character, english character and also a symbol (!,*,etc.)
def char_to_unsigned_byte(uni_char):
    global dbg_dict
    global ubyte_to_unicode_mapper
    global unicode_to_ubyte_mapper
            
            
    if len(uni_char.decode('UTF-8')) > 1:        
        print "Error!!! more than 1 character! [%s]. Should not have reached this stage. (Should have been filtered out in the bbox file reading phase)" % uni_char
        os.sys.exit(0)
            
    if not unicode_to_ubyte_mapper.has_key(uni_char.decode('UTF-8')):
        print "Error in mapping!!! Error mapping for key [%s] not found!" % uni_char
        #sys.exit(0)
        #return -1
        return 0
        
        
    return unicode_to_ubyte_mapper[uni_char.decode('UTF-8')]
    """
    if len(uni_char) > 2:        
        print "Error!!! more than 2 characters! [%s]. Should not have reached this stage. (Should have been filtered out in the bbox file reading phase)" % uni_char
        os.sys.exit(0)
        #return -1
    
    if (len(uni_char)>1):
        o = ord(uni_char[1])
        #if (o < 125):
        #    print "Error!!! <125"
            #sys.exit(0)
            
        if ubyte_to_unicode_mapper.has_key(o):
            if ubyte_to_unicode_mapper[o][0] != uni_char:
                print "Error in mapping!!! Error mapping [%s] The key value [%d] was already mapped to [%s]"  % (uni_char, o, ubyte_to_unicode_mapper[o][0])
                sys.exit(0)
            ubyte_to_unicode_mapper[o][1] = ubyte_to_unicode_mapper[o][1]+1
        else:
            print "Error in mapping!!! No mapping found for char [%s]"  % uni_char
            sys.exit(0)
            #ubyte_to_unicode_mapper[o] = [uni_char,1]
            
        dbg_dict[uni_char] = (ord(uni_char[0]), ord(uni_char[1]))        
        return o
    o = ord(uni_char)
    #if (o > 125):
    #    print "Error!!! o>125"
        #sys.exit(0)
    #dbg_dict[uni_char] = (ord(uni_char))
    if ubyte_to_unicode_mapper.has_key(o):
        if ubyte_to_unicode_mapper[o][0] != uni_char:
            print "Error!!!! Collision in mapping!!! Error mapping [%s] The key value [%d] was already mapped to [%s]"  % (uni_char, o, ubyte_to_unicode_mapper[o][0])
        ubyte_to_unicode_mapper[o][1] = ubyte_to_unicode_mapper[o][1]+1
    else:
        ubyte_to_unicode_mapper[o] = [uni_char,1]
    return o
    """


def output_mnist_files(kabala_name, im, boxes):
    global db_file_img
    global db_file_label
    global test_cases_total
    
    #test_cases_total+= len(boxes)
    
    #########################
    # images database file
    #########################
    
    print "Converting into MNIST format (%d boxes) ..." % len(boxes)
    
    print "Images file..."
    
    #db_file_img = open(kabala_name+u'-images.idx3-ubyte','wb')    
    
    width = test_case_width
    height = test_case_height
    
    
    
    box_num = 0
    
    for b in boxes:
        if (box_num%10==0):
            print "%d..." % (box_num) ,        
        
        if train_on_white_space == False:
            if b.char == " ":
                print "Abandoned BBox because it was whitespace."
                continue
        
        w = b.x2-b.x1+1
        h = b.y2-b.y1+1
        
        #hack for now:
        if h < 1:
            print "Error! had to use a hack! fixme. height == 0 !!!"
            b.y2 += 1
            h += 1
            
        
        #print "Debug: About to make an image at the size (%d,%d)" % (w,h)
        surf_orig_size = Image.new('RGB', (w, h))
        
        cut_box = (b.x1, b.y1, b.x2+1, b.y2+1)
        #cut_box = (b.x1, b.y1+1, b.x2+1, b.y2+1)
        region = im.crop(cut_box)
        
        if save_dbg_images and box_num<600: #if b.x1 == 219 and b.x2 == 223 and b.y2 == 339:
            print "Saving debug box of cropped(%d,%d),(%d,%d)" % (b.x1,b.y2,b.x2,b.y1)
            region.save(r"C:\temp\mnist_cropped_%d.bmp" % box_num, "BMP")
        
        #paste_box = (0, 0, b.x2-b.x1, b.y2-b.y1)
        #io = Image.open("template.png")
        #surf.paste(region,paste_box)
        surf_orig_size.paste(region)
        
        
        #trying to match the live editor
        #1. convert from PIL to cv2
        imcv = cv2.cvtColor(np.asarray(surf_orig_size), cv2.COLOR_RGB2BGR)
        #convert to greyscale
        imcv_bw = cv2.cvtColor(imcv, cv2.COLOR_BGR2GRAY)
        #2. resize using cv2's lanczos4                        
        resized_imcv = cv2.resize(imcv_bw, (28,28), interpolation= cv2.INTER_LANCZOS4)
        #3. convert back to PIL
        
        
        #orig - resize and then change into greyscale 
        #resized_im = surf_orig_size.resize((width, height), Image.ANTIALIAS)        
        #resized_im_greyscale = resized_im.convert('L')
                
        if save_dbg_images and box_num<600: #b.x1 == 219 and b.x2 == 223 and b.y2 == 339:
            print "Saving debug box of resized (%d,%d),(%d,%d)" % (b.x1,b.y2,b.x2,b.y1)
            #resized_im_greyscale.save(r"C:\temp\mnist_resized_%d.bmp" % box_num, "BMP")
            cv2.imwrite("C:\\temp\\mnist_resized_%d.bmp" %box_num,resized_imcv)
         
        
         
        #orig       
        #data = list(resized_im_greyscale.getdata())

        data = resized_imcv.flatten()

        #experiment - add aspect ratio instead of two pixels
        
        box_width = abs(b.x1-b.x2)
        box_height = abs(b.y2-b.y1)
        
        
        """
        
        if  box_height > box_width: #if height is bigger than width
            ratio = float(box_width) / float(box_height)
            ratio_ubyte = int(ratio * 255.0)
            data[392] = ratio_ubyte
        else:
            data[392] = 0
            
        """
                        
        data_array = array('B',data)
        
        data_array.tofile(db_file_img)
        
        #for x in xrange(width):
        #    for y in xrange(height):
        #        val = data[x+(width*y)]
        #        db_file_img.write(struct.pack('B',val))
                    
        #resized_im_greyscale.save("temp/output_%d.png" % box_num)
        
        box_num+=1
                
    #db_file_img.close()
    
    print "Done."
    
    print "Labels file..."
    #########################
    # labels database file
    #########################
    
    #db_file_label = open(kabala_name+u'-labels.idx1-ubyte','wb')    
    
    #if 
    
    for b in boxes:
        if train_on_white_space == False:
            if b.char == " ":
                print "Abandoned BBox because it was whitespace."
                continue
        db_file_label.write(struct.pack('B',char_to_unsigned_byte(b.char)))
    
    #db_file_label.close()
    
    print "Done."
    
    test_cases_total+= box_num
    
def generate_boxes_location_image(kabala_name,im, boxes):
    im_width,im_height = im.size
    boxes_image = Image.new('RGB', (im_width*2, im_height*2), 0)
    width,height = boxes_image.size
    
    data = boxes_image.load()
    
    for j in xrange(height):
        for i in xrange(width):
            data[i,j] = (0,0,0)
    
    for b in boxes:
        data[(b.x1*2),(b.y1*2)] = (255,255,255)
        data[(b.x2*2)+1,(b.y1*2)] = (255,255,255)
        data[(b.x1*2),(b.y2*2)+1] = (255,255,255)
        data[(b.x2*2)+1,(b.y2*2)+1] = (255,255,255)
    
    boxes_image.save(kabala_name+u"_Boxes_Data.BMP",'BMP')

    

def process_kabala(kabala_name):    
    global current_resolution
    global visualization_factor
    global results_file_name
    global NN_file
    global generate_mnist_files
    global files_order_file
    global save_visualization_images
    im = Image.open(kabala_name+u'.jpg')
    #im = im.convert('RGB')
    current_resolution = im.size
    
    scaled_im = im      
    
    if generate_mnist_files == True:
        files_order_file.write(kabala_name+"\n")
    
    
    if generate_mnist_files == False:
        if save_visualization_images == True:
            while scaled_im.size[0] < 2500:
                visualization_factor += 1
                print "Upscaling x%d for visualization" % visualization_factor
                scaled_im = im.resize([im.size[0]*visualization_factor, im.size[1]*visualization_factor],Image.NEAREST)
                print "Upscaled into %dx%d" % (scaled_im.size[0],scaled_im.size[1])        
        im = scaled_im

    

    print "Resolution %d x %d" % (current_resolution[0],current_resolution[1])

    boxes = zedusBoxes.read_box_file(kabala_name+u".boxDataBase", current_resolution)
            
    #Creating mnist files only if no AI_RESULT or NN file found
    if generate_mnist_files == True:
        output_mnist_files(kabala_name, im, boxes)    
    
    
    #with_boxes = add_boxes(im, boxes) #draw labels on the image
    with_boxes, ai_results_boxes = add_boxes(scaled_im, boxes, kabala_name) #draw labels on the image
    
    with_boxes.save(kabala_name+u'_WithResults.png','PNG')
    
    
    #ai_results_boxes_sorted = sort_boxes(ai_results_boxes)
    
    zedusBoxes.save_box_file(kabala_name+u'.box', ai_results_boxes, current_resolution)
    
    
    #generate_boxes_location_image(kabala_name,im, boxes)
    
    visualization_factor = 1 # restore into default settings

def main(path, _NN_file, _results_file_name, train_nn):
    global current_resolution
    global visualization_factor
    global results_file_name
    global NN_file
    global generate_mnist_files
    global files_order_file
    global db_file_img
    global db_file_label
    global results_file_opened
    global ai_results_list
    global current_character_global_index
    global znnLib
    
    print "About to load zedusNN.dll"
    
    if zedusConfig.fully_local == True:
        znnLib = cdll.LoadLibrary(zedusConfig.zedusnn_file)
    else:
        znnLib = cdll.LoadLibrary(zedusConfig.zedusnn_path +"/"+zedusConfig.zedusnn_file)
    prob_type = c_uint(1) #enum - 1 means mnist, which actually means right now 28x28 grey-scale character classification.


    if os.name == 'posix':
	znnLib.znnInit = getattr(znnLib,'_Z7znnInit12EProblemTypebPKc')
	znnLib.znnLoadNN = getattr(znnLib,'_Z9znnLoadNNPKc')
	znnLib.znnProcess = getattr(znnLib,'_Z10znnProcessPKcS0_')
	znnLib.znnProcessSingleCase = getattr(znnLib,'_Z20znnProcessSingleCasejPdjS_')
	znnLib.znnTrainOnDB = getattr(znnLib,'_Z12znnTrainOnDBPKcS0_')
	znnLib.znnTrainOnDB_StartFromProvidedNN = getattr(znnLib,'_Z32znnTrainOnDB_StartFromProvidedNNPKcS0_S0_')

    znnLib.znnInit.restype = c_char_p
    znnLib.znnLoadNN.restype = c_bool
    znnLib.znnProcess.restype = c_char_p
    znnLib.znnProcessSingleCase.restype = c_bool
    znnLib.znnTrainOnDB.restype = c_bool
    znnLib.znnTrainOnDB_StartFromProvidedNN.restype = c_bool
    
    try:    
        res = znnLib.znnInit(prob_type, c_bool(False), c_char_p(zedusConfig.home_path))
        print "Initialized zedusNN library. Result was [%s]" % res
    except:
        print "Problem initing zedusNN.dll!"
    
    current_character_global_index = 0
    
    NN_file = _NN_file
    results_file_name = _results_file_name
    
    if NN_file != "":
        #res = znnLib.znnLoadNN(c_char_p(r"C:\session_winners\evolve_PID_3648_0x305183EC\session_winner_PID_3648_2001_score_28.947_0.3373_In_784_Hidden_1_Hidden_Size_200_Output_124.org"))
        res = znnLib.znnLoadNN(c_char_p(NN_file))
        print "Loaded NN. Result was:",
        print res
    
    images_path = path
    
    finished_choosing_source_dir = False
    
    #test_data_dir = tkFileDialog.askdirectory(parent=root,initialdir=u"C:/",title=u'Please select database jpg images and box files directory')
    test_data_dir = path
    if '' == test_data_dir:
        print "No database path selected. Exiting."
        os.sys.exit(0)     
    
    #load results file (optional)
    
    ai_results_list = []
    results_file_opened = False
    
    #results_file_name = tkFileDialog.askopenfilename(parent=root,initialdir=u"C:/",title=u'Please select AI_RESULT file (optional)')
   
    
    if '' == results_file_name:
        #look for NN organism
        #NN_file = tkFileDialog.askopenfilename(parent=root,initialdir=u"C:/",title=u'Please select NeuralNetwork file - *.org (optional)')
        
        if ('' != NN_file):
            #we will generate the result file
            #params = 'mode load type mnist \"' + NN_file + '\" \"' + test_data_dir+'/mnist_style.images\" \"'+test_data_dir+'/mnist_style.labels\"'
            #subprocess.call([r'C:\OCR\CPP_Executables\NN_DoubleImage.exe', params])
            par1 = 'type'
            par2 = 'mnist'
            par3 = 'mode'
            par4 = 'load'
            par5 = NN_file
            par6 = test_data_dir+'/mnist_style.images'
            par7 = test_data_dir+'/mnist_style.labels'
            #subprocess.call([r"C:\google_code\zedusnn\Release\zedusNN_Client.exe", par1, par2, par3, par4, par5, par6, par7])
            
            res = znnLib.znnProcess(c_char_p(par6), c_char_p(par7))        
            
            print "znnLib.znnProcess Returned [%s]" % res
            
            print "Done running NN on data"
            #os.sys.exit(0)
            results_file_name = res
                            
    
    if '' != results_file_name:
        try:
            print "Loading results file..."
            ai_results_file = open(results_file_name,'rb')
            results_file_opened = True
            temp_list = []
            while (True):
                l = list(ai_results_file.read())
                if len(l)>0:
                    temp_list += l
                else:
                    break
            
            for e in temp_list:
                ai_results_list.append(ord(e))
            
            ai_results_file.close()
            print "Done."
        except IOError:
            print "No results file found."
    
    
    
    generate_mnist_files = False
    
    if results_file_name=="" and NN_file == "":
        generate_mnist_files = True
    
    #images_path = u'C:/OCR/ocrtraindata/train'
    images_path = test_data_dir
    
    files_order_file = None   
    
    if generate_mnist_files == True:
    	print "about to create .images file inside path [%s]\n" % test_data_dir
	
        db_file_img = open(test_data_dir+u'/mnist_style.images','wb')
        #magic num - images file
        db_file_img.write(struct.pack('>I',0x00000803))
        
        #number of images - will be replaced in the end of the writing (placeholder value for now)
        db_file_img.write(struct.pack('>I',0x1000))
         
        #number of rows
        db_file_img.write(struct.pack('>I', test_case_width))
        
        #number of columns
        db_file_img.write(struct.pack('>I', test_case_height))
        
        db_file_label = open(test_data_dir+u'/mnist_style.labels','wb')
        
         #magic num - labels file
        db_file_label.write(struct.pack('>I',0x00000801))
        
        #number of images - will be replaced in the end of the writing (placeholder value for now)
        db_file_label.write(struct.pack('>I',0x1000))
    
        files_order_file = open(images_path+r"\kabalot_order.txt",'wt')
    
     
        
    files_list = []
    
    if generate_mnist_files == True:
        for root, _, files in os.walk(images_path):
            for f in files:
                file_lwr = f.lower()
                spl = file_lwr.split('.')
                if not 'jpg' in spl[-1] or 'WithBoxes' in f or 'Boxes_Data' in f or 'MedianThreshold' in f:
                    continue
                print u'Working on: %s' % f
                fullpath = os.path.join(root, f)
                no_ext = fullpath[0:-4]        
                files_list.append(no_ext)
                #process_kabala(no_ext)
    else:
        files_order_file = open(images_path+r"\kabalot_order.txt",'rt')
        f_list =  files_order_file.readlines()
        for f in f_list:
            files_list.append(f[0:-1])
        
    for f in files_list:
        print u'Working on: %s' % f
        process_kabala(f)
    
    
    if generate_mnist_files == True:
        files_order_file.close()
    
    if generate_mnist_files == True:
        print "Fixing the entries num in both the mnist images and labels files. Total entries num is %d." % test_cases_total
        #fix the entries num
        db_file_img.seek(4)
        db_file_img.write(struct.pack('>I',test_cases_total))        
        db_file_img.close()
        
        #fix the entries num
        db_file_label.seek(4)
        db_file_label.write(struct.pack('>I',test_cases_total))
        db_file_label.close()
    
    #for key in dbg_dict.iterkeys():
    #    print key, " -> ", dbg_dict[key]
    
    print "Mapping Dictionary:"
    
    for key in ubyte_to_unicode_mapper.iterkeys():
        #print "%d -> ([%s] %d times)" % (key, ubyte_to_unicode_mapper[key][0], ubyte_to_unicode_mapper[key][1])
        print "%d -> ([%s])" % (key, ubyte_to_unicode_mapper[key].encode('UTF-8'))
    
       
    #process_kabala(u'heb2.rl0.exp0')
    #kabala_name = u'heb2.rl0.exp0'
    
    if generate_mnist_files == True:
        #if tkMessageBox.askyesno("NN Training" , "Do you want to Train a neural network on the generated mnist database?") == True:
        if train_nn == True:
            res = znnLib.znnTrainOnDB(c_char_p(test_data_dir+u'/mnist_style.images'), c_char_p(test_data_dir+u'/mnist_style.labels'))
            
            #res = znnLib.znnLoadNN(c_char_p(r"C:\session_winners\prev\evolve_PID_3388_0x27953A60\session_winner_PID_3388_18001_score_91.520_9.3242_In_784_Hidden_1_Hidden_Size_200_Output_124.org"))
            #print "Loaded NN. Result was:",
            print res
    
   
    
if __name__ == "__main__":
    if len(sys.argv) < 4:
        print "Not enough args!"
    main(*sys.argv[1:])
