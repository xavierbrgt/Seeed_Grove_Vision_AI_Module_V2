# Include definitions from the Python package to
# define datatype for the IOs and to have access to the
# Graph class
from cmsis_stream.cg.scheduler import *
# Include definition of the nodes
from nodes import * 

# Define the datatype we are using for all the IOs in this
# example

CAMERA_FORMAT = Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X


camera=Camera("ov5647",mode=CAMERA_FORMAT)
image_t = camera.image_type
width = image_t.width 
height = image_t.height


rgb = YUVToRGB("rgb",width,height)
jpeg = JPEGEncoder("jpeg",width,height)

sink=SendResult("sink",width,height)

# Create a Graph object
the_graph = Graph()

# Connect the source to the processing node
the_graph.connect(camera.raw,rgb.i)
the_graph.connect(rgb.o,jpeg.i)
# We can't reuse the buffer from the sensor framework since
# it is being used by the HW
the_graph.connect(jpeg.o,sink.jpeg)
the_graph.connect(jpeg.l,sink.l)


