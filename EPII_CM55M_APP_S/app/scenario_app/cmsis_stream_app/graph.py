# Include definitions from the Python package to
# define datatype for the IOs and to have access to the
# Graph class
from cmsis_stream.cg.scheduler import *
# Include definition of the nodes
from nodes import * 

# Define the datatype we are using for all the IOs in this
# example
floatType=CType(F32)


CROP_IMAGE_WIDTH = 640
CROP_IMAGE_HEIGHT = 480

OUT_IMAGE_WIDTH = 640
OUT_IMAGE_HEIGHT = 480

FM_INPUT_TENSOR_WIDTH = 192
FM_INPUT_TENSOR_HEIGHT = 192


IL_INPUT_TENSOR_WIDTH = 64
IL_INPUT_TENSOR_HEIGHT = 64

camera=Camera("ov5647")
jpeg=JPEGImage("jpeg",OUT_IMAGE_WIDTH,OUT_IMAGE_HEIGHT)
sink=SendResult("sink",OUT_IMAGE_WIDTH,OUT_IMAGE_HEIGHT)

# Create a Graph object
the_graph = Graph()

# Connect the source to the processing node
the_graph.connect(camera.o,jpeg.i)
the_graph.connect(jpeg.o,sink.j,buffer="(void*)app_get_jpeg_addr()")


#the_graph.connect(camera.o,raw.i)
#the_graph.connect(raw.o,sink.r,buffer="(void*)app_get_raw_addr()")
