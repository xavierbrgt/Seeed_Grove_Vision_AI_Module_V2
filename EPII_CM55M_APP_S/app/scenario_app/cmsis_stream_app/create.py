from subprocess import run
# Include definition of the nodes
from nodes import * 
# Include definition of the graph
#from stream_only_graph import * 
#
from canny_edge_graph import * 

#from face_mesh_graph import * 

from cmsis_stream.cg.yaml import *

# Create a configuration object
conf=Configuration()
# Disable inclusion of CMSIS-DSP headers so that we don't have
# to recompile CMSIS-DSP for such a simple example
conf.CMSISDSP = False
conf.asynchronous = False
conf.callback = True
conf.heapAllocation = True

# Argument of the scheduler function.
# Variables used in the initialization of some nodes
conf.cOptionalArgs=["stream_env_t *env",
                    "struct_algoResult *alg_result",
                    "struct_fm_algoResult_with_fps *alg_fm_result"]

# Name of files included by the scheduler
conf.appNodesCName="AppNodes.h"
conf.customCName="cv_app_settings.h"
conf.schedulerCFileName="cv_scheduler"
# Prefix for FIFO buffers
conf.prefix="cv_"
# Name of scheduler function
conf.schedName="cv_scheduler"
conf.bufferAllocation = True
conf.memoryOptimization = True 
conf.horizontal = True 
#conf.nodeIdentification=True

# Compute a static scheduling of the graph 
# The size of FIFO is also computed

with open("pre_schedule_cv.dot","w") as f:
    the_graph.graphviz(f)

run(["dot.exe","-Tpdf","-o","pre_schedule_cv.pdf","pre_schedule_cv.dot"])

scheduling = the_graph.computeSchedule(config=conf)

# Print some statistics about the compute schedule
# and the memory usage
print("Schedule length = %d" % scheduling.scheduleLength)
print("Memory usage %d bytes" % scheduling.memory)

# Generate the C++ code for the static scheduler
scheduling.ccode(".",conf)

# The generated code is including GenericNodes.h and 
# cg_status.h
# Those files can either be used from the CMSIS-Stream 
# repository or they can be generated from the Python 
# package so that it is easier to start using CMSIS-Stream

# GenericNodes.h is created in the folder "generated"
generateGenericNodes(".")

# cg_status.h is created in the folder "generated"
#generateCGStatus(".")

class MyStyle(Style):
    def edge_label(self,fifo,typeName,length):
        #return f"{typeName}<BR/>({length})"
        return f"{typeName}"

myStyle = MyStyle()

# Generate a graphviz representation of the graph
with open("cv.dot","w") as f:
    scheduling.graphviz(f,config=conf,style=myStyle)

run(["dot.exe","-Tpdf","-o","cv.pdf","cv.dot"])
run(["touch.exe","cmsis_stream_app.c"])

with open("python_config.h","w") as h:
    print(f"""#ifndef PYTHON_CONFIG_H
#define PYTHON_CONFIG_H 

#define CAMERA_FORMAT {camera.camera_define}

#endif
""",file = h )