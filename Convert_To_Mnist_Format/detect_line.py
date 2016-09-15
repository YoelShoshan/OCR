#!/usr/bin/env python

from PIL import Image
import ImageDraw
import sys
import os
import numpy
import random

#image_name = u"C:/OCR/train400/train400_heb2.ss0.exp0.jpg"

#vis = Image.open(image_name)
#vis_data = vis.load()

#im_col = Image.open(image_name)


#im_col_data = im_col.load()

#im = im_col.convert('L')
#im_data = im.load()
#im_width, im_height = im.size
vis_data = None
im_edge_data = None

helper_data = None

im_width = 0
im_height = 0


#draw vertical line
def create_edge_image(im_data):
    global im_width, im_height
    im_edge = Image.new('L', (im_width, im_height), 0)
    im_edge_data = im_edge.load()
    
    for y in xrange(im_height):
        for x in xrange(im_width):
            # horizontal
            if x>0:
                im_edge_data[x,y] += abs(im_data[x,y] - im_data[x-1,y])
            # vertical
            if y>0:
                im_edge_data[x,y] += abs(im_data[x,y] - im_data[x,y-1])
            
    #im_edge.show()
    return im_edge


#def draw_vert_line(y):
#    for x in xrange(0,im_width):
#        vis_data[x,y] = (255,vis_data[x,y][1], 0)

def draw_line_rgb( (x,y), (x2,y2) , pixels_data_array, opaque = False, col = (255,0,0)):
    coords = bresenham_line((x,y), (x2,y2))
    for c in coords:
        if (opaque == True):
            pixels_data_array[c[0],c[1]] = col
        else:
            p = pixels_data_array[c[0],c[1]] 
            pixels_data_array[c[0],c[1]] = (255,0,p[2])
        

def draw_line_l( (x,y), (x2,y2) , pixels_data_array):
    global im_width, im_height
    coords = bresenham_line((x,y), (x2,y2))
    for c in coords:
        #vis_data[c[0],c[1]] = (255,vis_data[c[0],c[1]][1],0)
        #if (c[0] > im_width-1):
        #    print "DrawLine:Error! x=%d is too big." % c[0]
        #if (c[1] > im_height-1):
        #    print "DrawLine:Error! y=%d is too big." % c[1]                
        pixels_data_array[c[0],c[1]] = (255)

def draw_coords( coords):
    for c in coords:
        vis_data[c[0],c[1]] = (255,vis_data[c[0],c[1]][1],0)        

def bresenham_line((x,y),(x2,y2)):
    """Brensenham line algorithm"""
    steep = 0
    coords = []
    dx = abs(x2 - x)
    if (x2 - x) > 0: sx = 1
    else: sx = -1
    dy = abs(y2 - y)
    if (y2 - y) > 0: sy = 1
    else: sy = -1
    if dy > dx:
        steep = 1
        x,y = y,x
        dx,dy = dy,dx
        sx,sy = sy,sx
    d = (2 * dy) - dx
    for i in range(0,dx):
        if steep: coords.append((y,x))
        else: coords.append((x,y))
        while d >= 0:
            y = y + sy
            d = d - (2 * dx)
        x = x + sx
        d = d + (2 * dy)
    coords.append((x2,y2))
    return coords    

def line_mean(y):
    mean = 0.0
    for x in xrange(0,im_width):
        mean += float(im_edge_data[x,y])
    mean /= float(im_edge_data)
    return mean

def line_count_above(coords,n):
    count = 0
    collided = False
    step = 0
    collided_on = -1
    for c in coords:
        if im_edge_data[c[0],c[1]] > n:
            count+= 1
            if collided == False:
                collided_on = step
                collided = True            
        step += 1
            
    return count, collided_on


def process_lines():
    draw_line((0,0),(100,200))
    for y in xrange(0,im_height):
        coords = bresenham_line((0,y),(im_width-1,y))
        above,collided_on = line_count_above(coords,10)
        if above < 15:
            draw_coords(coords)

def process_lines_angled():
    for y in xrange(0,im_height):
        range = 10
        for off in xrange(range*2):
            o = off - range
            dest_y = y+off
            if dest_y < 0:
                continue
            if dest_y > im_height-1:
                continue
            coords = bresenham_line((0,y),(im_width-1,dest_y))
            above = line_count_above(coords,10)
            if above < 35:
                draw_coords(coords)            

def merge(b1,b2):
    global im_width, im_height
    y1 = min(b2[0][1], b1[0][1])
    y2 = max(b2[1][1], b1[1][1])

    x1 = min(b2[0][0], b1[0][0])
    x2 = max(b2[1][0], b1[1][0])

    if x2 - x1 > im_width / 5:
        return None

    if y2 - y1 > 200:
        return None    

    return ((x1,y1), (x2,y2))    

def is_box_left_neighbour(b1,b2):
    if b2[0][1]  > b1[1][1]:
        return None

    if b2[1][1]  < b1[0][1]:
        return None

    if b2[1][0] != b1[0][0] - 1 and b2[0][0] != b1[1][0] + 1:
        return None

    return merge(b1,b2)

def do_boxes_intersect(b1,b2):
    if b2[0][0] > b1[1][0]:
        return None

    if b2[1][0] < b1[0][0]:
        return None

    if b2[0][1] > b1[1][1]:
        return None

    if b2[1][1] < b1[0][1]:
        return None    
    
    return merge(b1,b2)

def is_box_top_neighbour(b1,b2):
    if b2[0][0]  > b1[1][0]:
        return None

    if b2[1][0]  < b1[0][0]:
        return None

    #if b2[0][1] < b1[1][1] + 10 and b2[0][1] > b1[1][1] - 10:
    #    print "potential blocks:"
    #    print "(%s)->(%s)" % (b1,b2)    

    if b2[1][1] != b1[0][1] - 1 and b2[0][1] != b1[1][1] + 1:
        return None     

    res = merge(b1,b2) 

    #print "Merged %s and %s into %s" % (b1,b2,res)

    return res

def is_box_intersecting(b1,b2):
    if b2[0][1]  > b1[1][1]:
        return None

    if b2[1][1]  < b1[0][1]:
        return None
    
    if b2[0][0]  > b1[1][0]:
        return None

    if b2[1][0]  < b1[0][0]:
        return None

    res = merge(b1,b2)
    
    return res
    
def print_block(block):
    print "Block = (%d,%d)->(%d,%d)" % (block[0][0],block[0][1],block[1][0],block[1][1])

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
            merged = is_box_left_neighbour(b,b2)
            if (None == merged):
                merged = is_box_top_neighbour(b,b2)
            if (None == merged):
                merged = do_boxes_intersect(b,b2)
            if (merged != None):
                #if (merged[1][1] == 832):
                #    pass
                #print_block(merged)
                #draw_block(merged) 
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
    
def draw_block(b, rand = True):
    #global vis_data
    #assuming even width and height
    block_size_x = b[1][0] - b[0][0] + 1
    block_size_y = b[1][1] - b[0][1] + 1
    color = (255,0,0)
    if rand == True:        
        color = (random.random()*255,random.random()*255,random.random()*255)
        color = (int(color[0]), int(color[1]), int(color[2]))
    #for j in xrange(block_size_y):
    #    top_left = (b[0][0],b[0][1]+j)
    #    bottom_right = (b[1][0],b[0][1]+j)
    #    #draw_line_l( top_left, bottom_right , helper_data)        
    #    draw_line_rgb( top_left, bottom_right , vis_data, col = color)
    
    top_left = (b[0][0],b[0][1])
    top_right = (b[0][0]+block_size_x-1,b[0][1])
    bottom_left = (b[0][0],b[0][1]+block_size_y-1)
    bottom_right = (b[0][0]+block_size_x-1,b[0][1]+block_size_y-1)
    
    draw_line_rgb( top_left, top_right , vis_data, col = color)
    draw_line_rgb( top_right, bottom_right , vis_data, col = color)
    draw_line_rgb( bottom_left, bottom_right , vis_data, col = color)
    draw_line_rgb( bottom_left, top_left , vis_data, col = color)
    

def process_initial_blocks():
    global vis
    blocks = []
    global helper_data
    block_size = 3
    for y in xrange(0,im_height/block_size):
        for x in xrange(0,im_width/block_size):
            spotted_edges = 0
            for i in xrange(block_size):
                for j in xrange(block_size):
                    abs_x = x*block_size+i
                    abs_y = y*block_size+j
                    if (abs_x > im_width-1):
                        continue
                    if (abs_y > im_height-1):
                        continue
                    
                    if im_edge_data[abs_x,abs_y] > 20:
                        spotted_edges += 1
            if spotted_edges > 2:
                #add block to our list
                block = ((x*block_size, y*block_size),(x*block_size+block_size-1, y*block_size+block_size-1))
                blocks.append(block )
                #draw block
                #for j in xrange(block_size):
                #    draw_line_l((x*block_size,y*block_size+j),(x*block_size+block_size-1,y*block_size+j), helper_data)
                draw_block(block)
    return blocks

def process_lines_angled_non_linear():
    for y in xrange(0,im_height):
        range = 4
        screen_parts = 9
        for sp in xrange(screen_parts):
            for off in xrange(range*2):
                o = off - range
                dest_y = y+off
                if dest_y < 0:
                    continue
                if dest_y > im_height-1:
                    continue

                screen_part_size = int(float(im_width) / float(screen_parts))                
                coords = bresenham_line(((sp*screen_part_size),y),(((sp+1)*screen_part_size)-1,dest_y))
                above,collided_on = line_count_above(coords,30)
                if above < 3: #should probably a function of the screen_parts
                    #if -1 == collided_on:
                    draw_coords(coords)
                    #else:
                    #    draw_coords(coords[0:collided_on])
    for x in xrange(0,im_width):
        range = 1
        screen_parts = 31
        for sp in xrange(screen_parts):
            for off in xrange(range*2):
                o = off - range
                dest_x = x+off
                if dest_x < 0:
                    continue
                if dest_x > im_width-1:
                    continue

                screen_part_size = int(float(im_height) / float(screen_parts))                
                #coords = bresenham_line(((sp*screen_part_size),y),(((sp+1)*screen_part_size)-1,dest_y))
                
                coords = bresenham_line((x,(sp*screen_part_size)), ((dest_x,((sp+1)*screen_part_size)-1)) )
                
                above,collided_on = line_count_above(coords,30)
                if above < 2: #should probably a function of the screen_parts
                    #if -1 == collided_on:
                    draw_coords(coords)
                    #else:
                    #    draw_coords(coords[0:collided_on])
    

def cover_space():
    global im_edge
    global im_edge_data
    global vis
    global im_width
    global im_height
    global vis_data
    global helper_data

    for x in xrange(im_width):
        for y in xrange(im_height):
            if 255 != helper_data[x,y]:
                vis_data[x,y] = (255,0,0)
    

def size_change(x):
    if x < 450:
        if x*2 >= 500:
            return 2
        if x*3 >= 500:
            return 3
        return 4
    return 1

def LoadKabalaImage(name):
    im = Image.open(name)
    width,height = im.size
    
    factor = size_change(width)
    
    if factor != 1:
        im = im.resize((width*factor,height*factor), Image.NEAREST) #simplest filter method on purpose

    return im



def is_box_reasonable(b):
    w = b[1][0] - b[0][0]
    h = b[1][1] - b[0][1]
    
    fact = 3
    
    #if w > h * 5:
    #    return False
    
    if h > w * 4:
        return False
    
    return True

def Process_Kabala(path, file):
    global im_edge
    global im_edge_data
    global vis
    global im_width
    global im_height
    global vis_data
    global helper_data

    vis = LoadKabalaImage(path+file)#Image.open(path+file)
    vis_data = vis.load()
    bw = vis.convert('L')

    helper = vis.convert('L')
    helper_data = helper.load()

    im_width, im_height = bw.size    

    bw_data = bw.load()
    
    #orig
    im_edge = create_edge_image(bw_data)
    
    #im_edge = LoadKabalaImage(u'C:/OCR/ridge/ridge_l.bmp')
    
    
    #im_edge.show()
    #im_edge.save(u'C:/temp/line/edge_'+f)
    im_edge_data = im_edge.load()        

    #process_lines_angled_non_linear()
    blocks = process_initial_blocks()

    #vis.save(u'C:/temp/line/initial_blocks_'+f)
    #vis.save(u'C:/temp/line/initial_blocks.bmp')          

    #helper.show()
    
    #vis.show()
    
    prev_blocks_found = 9999999
   
    for i in xrange(30):
        vis = LoadKabalaImage(path+file)
        vis_data = vis.load()
        blocks_found = len(blocks)
        if blocks_found == prev_blocks_found:
            print "No more block changes! Stopping"
            break
        prev_blocks_found = blocks_found
        print "Group blocks... step %d (%d blocks found)" % (i,len(blocks))                
        blocks = group_blocks(blocks)        
        print "Done grouping blocks."
    
    
    for b in blocks:
        if is_box_reasonable(b):
            draw_block(b)
    
    #vis.save(u'C:/temp/line/blocks_' + f+'.bmp')    
    
    #vis.show()    
   
    #cover_space()
    
#images_path = u"c:/ocr/train500/"
#images_path = u"c:/ocr/train250/"
#images_path = u"c:/ocr/big_list/"

#images_path = u"C:/OCR/full_db/"
images_path = u"C:/OCR/local_threshold_1/"

#C:\OCR\big_list

print "Started..."

for root, _, files in os.walk(images_path):
    for f in files:
        file_lwr = f.lower()
        spl = file_lwr.split('.')
        if not 'bmp' in spl[-1]:# or not 'train' in f:
            continue
        print u'Working on: %s' % f

        Process_Kabala(images_path,f)

          
        

print "Done."
       
#process_lines()
#process_lines_angled()
#process_lines_angled_non_linear()            
#try_1()

#im_col.show()
#vis.show()


    