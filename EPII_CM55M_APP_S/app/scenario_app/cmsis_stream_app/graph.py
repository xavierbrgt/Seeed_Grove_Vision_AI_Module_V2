# Include definitions from the Python package to
# define datatype for the IOs and to have access to the
# Graph class
from cmsis_stream.cg.scheduler import *
# Include definition of the nodes
from nodes import * 

# Define the datatype we are using for all the IOs in this
# example

CAMERA_FORMAT = Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X

CROP_IMAGE_WIDTH = 640
CROP_IMAGE_HEIGHT = 480

OUT_IMAGE_WIDTH = 640
OUT_IMAGE_HEIGHT = 480

FM_INPUT_TENSOR_WIDTH = 192
FM_INPUT_TENSOR_HEIGHT = 192


IL_INPUT_TENSOR_WIDTH = 64
IL_INPUT_TENSOR_HEIGHT = 64

camera=Camera("ov5647",mode=CAMERA_FORMAT)
image_t = camera.image_type

sink=SendResult("sink",image_t)

# Create a Graph object
the_graph = Graph()

# Connect the source to the processing node
the_graph.connect(camera.o,sink.i,buffer="(void*)app_get_raw_addr()")


