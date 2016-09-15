#!/usr/bin/env python
import zedusFindChars
import zedusBoxEditor
import zedusConvertToMnist
import zedusConfig

if zedusConfig.support_gui == True:
	import Tkinter
	import tkFileDialog
import time
import sys
import os

if zedusConfig.support_gui == True:
	tk = Tkinter.Tk()
	tk.withdraw()

#import cProfile, pstats, io
#pr = cProfile.Profile()
#pr.enable()


def generate_mnist_files(images_path):
    t1 = time.time()
    zedusConvertToMnist.main(images_path, "", "", False)  # generate the mnist files
    t2 = time.time()
    print "*** Generate MNIST files time: ",
    print (t2 - t1) ,
    print "Seconds."
    
def run_NN_on_mnist_files(images_path, neural_network_path):
    t1 = time.time()
    zedusConvertToMnist.main(images_path, neural_network_path, "", False)
    t2 = time.time()
    print "*** Run NN time: ",
    print (t2 - t1) ,
    print "Seconds."
    
    
######### Main Options ########

def prepare_boxes_to_send_database_typing_job(images_path):
    images_path = images_path
    zedusConfig.sort_left_to_right = False

    t1 = time.time()
    zedusFindChars.main(images_path)
    t2 = time.time()
    print "*** Detect chars location time: ",
    print (t2 - t1) ,
    print "Seconds."
 
def get_results(images_path, neural_network_path):
    images_path = images_path

    t1 = time.time()
    zedusFindChars.main(images_path)
    t2 = time.time()
    print "*** Detect chars location time: ",
    print (t2 - t1) ,
    print "Seconds."
    # return   
    
    t1 = time.time()
    zedusBoxEditor.main(images_path, "Fast")  # dummy typing
    #zedusBoxEditor.main(images_path, "True") #dummy typing
    t2 = time.time()
    print "*** Box Editor time: ",
    print (t2 - t1) ,
    print "Seconds."
            
    generate_mnist_files(images_path)
    run_NN_on_mnist_files(images_path, neural_network_path)

    
def get_results_for_labeled_data(images_path, neural_network_path):    
    generate_mnist_files(images_path)
    run_NN_on_mnist_files(images_path, neural_network_path)
       
    
def manually_type_database(images_path):    
    zedusBoxEditor.main(images_path, "False") 

def main(desired_path):    
    # print "Start Time: ",
    # print t        

    # path = r"C:\OCR\SharingPrices\TEST_first_ellad_db\part_8"
    # path = r"C:\OCR\test"
    
    path = desired_path
    
    if path == None:    
	if zedusConfig.support_gui == True:
		path = tkFileDialog.askdirectory(parent=tk, initialdir='C:/', title='Select your receipts folder', mustexist=True)
		if path == "":
		    print "Did not select a proper dir! Exiting."
		    return
	else:
		print 'Please provide a DIRECTORY path containing the receipts in jpg format.\n Note: only ".jpg" extension is supported.'
	return
    
    #path = path.replace("/", "\\")
    
    # path = r"C:\OCR\prev 3\prev 2\train400"
    # nn_location = r"C:\session_winners\evolve_PID_4892_0x36D7931A\session_winner_PID_4892_19001_score_83.117_12.7308_In_784_Hidden_1_Hidden_Size_200_Output_124.org"
    # nn_location = r"C:\session_winners\evolved_PID_4404_0x0138C5D4\session_winner_PID_4404_1001_score_33.425_0.3225_In_784_Hidden_1_Hidden_Size_200_Output_124.org"    
    
#    if path[-1] == "\\" or path[-1] == "/":
#        path = path[:-1]	    

    print "User selected [%s]\n" % path

    t1 = time.time()    
    
    org_file = "session_winner_PID_4896_17001_score_490.274_80.1025_In_784_Hidden_1_Hidden_Size_200_Output_124.org"
    
    org_path = zedusConfig.home_path + "/neural_networks"
    
    """org_path = ""
    if os.name == 'posix':
        org_path = zedusConfig.home_path + "/neural_networks"
    else:
        org_path = zedusConfig.home_path + 
    """

    if zedusConfig.fully_local == True:
        nn_location = org_file
    else:    
        nn_location = org_path+"/"+org_file
    
    # fully local    

    #get_results_for_labeled_data(path,nn_location) #run OCR on a receipt when you have bboxes+labels
    get_results(path, nn_location)  # run OCR on a receipt when you don't have bboxes+labels
    #manually_type_database(path) #use this to type manually the database
    #prepare_boxes_to_send_database_typing_job(path) #use this to prepare the box file to send to the manual typer person.
    #generate_mnist_files(path)
    
    t2 = time.time()
    # print "End Time: ",
    # print t2
    print "Total time: ",
    print (t2 - t1) / 60.0 ,
    print "Minutes. (" ,
    print (t2 - t1),
    print "Seconds.)"

if __name__ == "__main__":
    desired_path = None
    if len(sys.argv) > 1:
        desired_path = sys.argv[1]
    main(desired_path) 
  
