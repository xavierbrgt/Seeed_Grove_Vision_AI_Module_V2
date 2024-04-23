# Include definitions from the Python package to
# define datatype for the IOs and to have access to the
# Graph class
from cmsis_stream.cg.scheduler import *


from appnodes import * 

# Define the datatype we are using for all the IOs in this
# example

CAMERA_FORMAT = CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X


camera=Camera("ov5647",mode=CAMERA_FORMAT)
image_t = camera.image_type
width = image_t.width 
height = image_t.height


#rgb = YUVToRGB("rgb",width,height)

# Canny edge
to_gray8 = YUVToGray8("to_gray8",width,height)
gaussian = GaussianFilter("gaussian",width,height)
canny_edge = CannyEdge("canny",width,height)
to_rgb = Gray8ToRGB("to_rgb",width,height)

#

jpeg = JPEGEncoder("jpeg",width,height)
sink=SendResult("Serial",width,height)

# Create a Graph object
the_graph = Graph()

# Connect the source to the processing node
#the_graph.connect(camera.o,rgb.i,buffer="(void*)app_get_raw_addr()")
# Canny edge 
#
the_graph.connect(camera.raw,to_gray8.i)
the_graph.connect(to_gray8.o,gaussian.i)
the_graph.connect(gaussian.o,canny_edge.i)
the_graph.connect(canny_edge.o,to_rgb.i)
#the_graph.connect(to_gray.o,to_rgb.i)

#
the_graph.connect(to_rgb.o,jpeg.i)
# We can't reuse the buffer from the sensor framework since
# it is being used by the HW
the_graph.connect(jpeg.o,sink.jpeg)
the_graph.connect(jpeg.l,sink.l)


