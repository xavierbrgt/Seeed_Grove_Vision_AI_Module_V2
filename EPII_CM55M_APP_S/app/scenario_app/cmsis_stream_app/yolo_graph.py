# Include definitions from the Python package to
# define datatype for the IOs and to have access to the
# Graph class
from cmsis_stream.cg.scheduler import *
# Include definition of the nodes
from nodes import * 

# Define the datatype we are using for all the IOs in this
# example
floatType=CType(F32)

CROP_LENGTH = 10 
PAD_LENGTH = 10 
RESIZE_LENGTH = 10 
EYE_LENGTH = 10

camera=Camera("ov5647")

fd=TFNode12("face_detect")
yolo_post=YoloPostProcess("post_process")
crop=CropCamera("crop_camera",CROP_LENGTH)
pad=PadImage("pad_image",CROP_LENGTH,PAD_LENGTH)
resize=ResizeImage("resize_image",PAD_LENGTH,RESIZE_LENGTH)
col=YYYY("color_convert",RESIZE_LENGTH)

fm=TFNode12("face_mesh")
fm_process=FaceMeshProcess("fm_process")

crop_eye_l = CropImage("cropL")

sink=SendResult("sink")

# Create a Graph object
the_graph = Graph()

# Connect the source to the processing node
the_graph.connect(camera.o,fd.i,buffer="env->fd_input.data")
the_graph.connect(fd.oa,yolo_post.ia,buffer="env->fd_output.data")
the_graph.connect(fd.ob,yolo_post.ib,buffer="env->fd_output2.data")
the_graph.connect(yolo_post.o,crop.s)
the_graph.connect(crop.o,pad.i)
the_graph.connect(pad.o,resize.i)
the_graph.connect(resize.o,col.i)

the_graph.connect(col.o,fm.i,buffer="env->fm_input.data")
the_graph.connect(fm.oa,fm_process.ia,buffer="env->fm_output.data")
the_graph.connect(fm.ob,fm_process.ib,buffer="env->fm_output2.data")

the_graph.connect(resize.o,col.i)

the_graph.connect(fm_process.o,sink.i)


