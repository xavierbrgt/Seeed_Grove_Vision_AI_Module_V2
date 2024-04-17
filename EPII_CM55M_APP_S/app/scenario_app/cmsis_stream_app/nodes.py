# Include definitions from the Python package
from cmsis_stream.cg.scheduler import SINT8,UINT32,CType,CStructType,GenericNode,GenericSink,GenericSource

tfType = CStructType("TfLiteTensor",10)
posType = CStructType("struct_position",8)

bufferType = CType(SINT8)
activateType = CType(UINT32)

### Define new types of Nodes 

class TFNode11(GenericNode):
    def __init__(self,name):
        GenericNode.__init__(self,name)
        self.addInput("i",bufferType,1)
        self.addOutput("o",bufferType,1)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "TFNode11"

class TFNode12(GenericNode):
    def __init__(self,name):
        GenericNode.__init__(self,name)
        self.addInput("i",bufferType,1)
        self.addOutput("oa",bufferType,1)
        self.addOutput("ob",bufferType,1)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "TFNode12"

class YoloPostProcess(GenericNode):
    def __init__(self,name):
        GenericNode.__init__(self,name)
        self.addInput("ia",bufferType,1)
        self.addInput("ib",bufferType,1)
        self.addOutput("o",activateType,1)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "YoloPostProcess"

class CropCamera(GenericNode):
    def __init__(self,name,nb):
        GenericNode.__init__(self,name)
        self.addInput("s",activateType,1)
        self.addOutput("o",bufferType,nb)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "CropCamera"

class PadImage(GenericNode):
    def __init__(self,name,nba,nbb):
        GenericNode.__init__(self,name)
        self.addInput("i",bufferType,nba)
        self.addOutput("o",bufferType,nbb)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "PadImage"

class ResizeImage(GenericNode):
    def __init__(self,name,nba,nbb):
        GenericNode.__init__(self,name)
        self.addInput("i",bufferType,nba)
        self.addOutput("o",bufferType,nbb)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "ResizeImage"

class CropImage(GenericNode):
    def __init__(self,name,nba,nbb):
        GenericNode.__init__(self,name)
        self.addInput("i",bufferType,nba)
        self.addOutput("o",bufferType,nbb)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "CropImage"

class YYYY(GenericNode):
    def __init__(self,name,nba):
        GenericNode.__init__(self,name)
        self.addInput("i",bufferType,nba)
        self.addOutput("o",bufferType,3*nbb)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Y_to_YYY"

class FaceMeshProcess(GenericNode):
    def __init__(self,name):
        GenericNode.__init__(self,name)
        self.addInput("ia",bufferType,1)
        self.addInput("ib",bufferType,1)

        self.addInput("ea",posType,16)
        self.addInput("eb",posType,16)
        self.addOutput("o",activateType,1)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "FaceMeshProcess"

class SendResult(GenericSink):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        self.addInput("j",bufferType,w*h)
        self.addVariableArg("env")
        self.addVariableArg("alg_result")
        self.addVariableArg("alg_fm_result")
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "SendResult"

class RGB888Sink(GenericSink):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        self.addInput("r",bufferType,w*h*3)
        self.addVariableArg("env")
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "RGB888Sink"

class Camera(GenericSource):
    def __init__(self,name):
        GenericSource.__init__(self,name)
        self.addOutput("o",activateType,1)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Camera"


class RawYUVImage(GenericNode):
    def __init__(self,name,w,h):
        GenericSource.__init__(self,name)
        self.addInput("i",activateType,1)
        self.addOutput("o",bufferType,w*h*3)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "RawYUVImage"

class RawRGBImage(GenericNode):
    def __init__(self,name,w,h):
        GenericSource.__init__(self,name)
        self.addInput("i",activateType,1)
        self.addOutput("o",bufferType,w*h*3)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "RawRGBImage"

class JPEGImage(GenericNode):
    def __init__(self,name,w,h):
        GenericSource.__init__(self,name)
        self.addInput("i",activateType,1)
        self.addOutput("o",bufferType,w*h)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "JPEGImage"

class YUVToRGB(GenericNode):
    def __init__(self,name,w,h,scale):
        GenericNode.__init__(self,name)
        self.addInput("i",bufferType,w*h*3)
        self.addOutput("o",bufferType,(w//scale)*(h//scale)*4)
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        self.addLiteralArg(scale)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "YUVToRGB"