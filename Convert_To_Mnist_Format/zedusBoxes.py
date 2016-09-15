#!/usr/bin/env python


#groupNumber means line number.
class BBox:
    def __init__(self, x1, y1, x2, y2, char, groupNumber, mirror_y=True):
        #self.current_resolution = current_resolution
        self.x1 = x1
        self.x1 = max(0,self.x1)
        #self.x1 = min(current_resolution[0]-1,self.x1)
        
        """
        if True == mirror_y:                    
            self.y1 = current_resolution[1]-y2-1            
            self.y2 = current_resolution[1]-y1-1
           
        else:     
            self.y1 = y1
            self.y2 = y2
        """
        self.y1 = y1
        self.y2 = y2        
        
        self.y1 = max(0,self.y1) #somehow you can get width values (max was supposed to be height-1)
        #self.y1 = min(current_resolution[1]-1,self.y1)
        self.y2 = max(0,self.y2)
        #self.y2 = min(current_resolution[1]-1,self.y2)
            
        self.x2 = x2
        self.x2 = max(0,self.x2)
        #self.x2 = min(current_resolution[0]-1,self.x2)        
        
        self.char = char
        self.selected = False #helper in sorting
        self.ignore = False
        self.groupColor = (255,0,0)
        #self.groupNumber = -1
        self.groupNumber = groupNumber        
        self.wordNumber = -1
    
    def clone(self):
        #self.current_resolution = current_resolution
        res = BBox(0,0,0,0,0,0,(0,0))
        res.x1 = self.x1
        res.x2 = self.x2
        res.y1 = self.y1
        res.y2 = self.y2            
        res.char = self.char
        res.selected = self.selected
        res.ignore = self.ignore
        res.groupColor = self.groupColor
        res.groupNumber = self.groupNumber        
        res.wordNumber = self.wordNumber
        return res
        
    def merge_with(self,b):
        self.x1 = min(self.x1,b.x1)
        self.y1 = min(self.y1,b.y1)
        self.x2 = max(self.x2,b.x2)
        self.y2 = max(self.y2,b.y2)        
        
        
        
    def point_in_me(self, x,y):
        if x < self.x1 or x > self.x2:
            return False
        
        if y < self.y1 or y > self.y2:
            return False
        
        return True
    def __str__(self):
        return "Box: (%d,%d),(%d,%d)" % (self.x1,self.y1,self.x2,self.y2)

def read_box_file(box_filename, mirror_y = True):
    boxes = []
    
    f = open(box_filename)
    
    lines = f.readlines()
            
    for l in lines:
        spl = l.split()
        
        char = " "
        
        base_ind = 1
        
        if len(spl) == 7:        
            if (len(spl[0].decode('UTF-8')) > 1):
                print "Error!!! more than 1 character! [%s]. Discarding bbox." % spl[0]
                continue
            
            if (len(spl[0]) > 2):
                print "Error!!! more than 2 characters in the raw representation! Suspecting NIKKUD in [%s]. Discarding bbox." % spl[0]
                continue
            
            char = spl[0]
        elif len(spl) == 6:
            # since space isn't a token, if we have 6 tokens we assume that this bbox represents whitespace.
            base_ind = 0
        else:
            print "read_box_file::Error! Wrong num of tokens in line!"
        

        bbox = BBox(int(spl[base_ind]),int(spl[base_ind+1]),int(spl[base_ind+2]),int(spl[base_ind+3]),char, int(spl[base_ind+4]), mirror_y)
        bbox.groupNumber = int(spl[base_ind+4])
        bbox.wordNumber = int(spl[base_ind+5])
        boxes.append(bbox)
    
    #print text #.encode('utf-8')
    return boxes

def save_box_file(box_filename, boxes,mirror_y = True, force_group_zero = False):
    f = open(box_filename, 'w')
    
    for b in boxes:        
        #f.write(b.char.encode('UTF-8'))
        f.write(b.char)
        
        groupNum = b.groupNumber        
        if force_group_zero == True:
            groupNum = 0
        
        """if mirror_y == True:
            box_loc_str = u' %d %d %d %d %d %d\r\n' % (b.x1, current_resolution[1]-b.y2-1, b.x2, current_resolution[1]-b.y1-1, groupNum, b.wordNumber)
        else:
            box_loc_str = u' %d %d %d %d %d %d\r\n' % (b.x1, b.y1, b.x2, b.y2, groupNum, b.wordNumber)"""
        
        box_loc_str = u' %d %d %d %d %d %d\r\n' % (b.x1, b.y1, b.x2, b.y2, groupNum, b.wordNumber)
        
        #box_loc_str = u' %d %d %d %d %d\r\n' % (b.x1,b.y1, b.x2, b.y2, b.groupNumber)        
        f.write(box_loc_str.encode('UTF-8'))

    f.close()

def save_as_tesseract_format(box_filename, boxes, image_height):
    f = open(box_filename, 'w')
    
    for b in boxes:        
        #f.write(b.char.encode('UTF-8'))
        f.write(b.char)
               
        box_loc_str = u' %d %d %d %d 0\r\n' % (b.x1, image_height-b.y1, b.x2, image_height-b.y2)
        
        #box_loc_str = u' %d %d %d %d %d\r\n' % (b.x1,b.y1, b.x2, b.y2, b.groupNumber)        
        f.write(box_loc_str.encode('UTF-8'))

    f.close()

def draw_box(im, box, col):
    for i in xrange(box.x1,box.x2+1):        
        im.putpixel((i,box.y1), col)
        im.putpixel((i,box.y2), col)
    for i in xrange(box.y1,box.y2+1):        
        im.putpixel((box.x1,i), col)
        im.putpixel((box.x2,i), col)

