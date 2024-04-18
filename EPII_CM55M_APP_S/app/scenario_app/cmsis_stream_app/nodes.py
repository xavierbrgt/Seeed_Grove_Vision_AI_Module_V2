# Include definitions from the Python package
from cmsis_stream.cg.scheduler import CGStaticType,SINT8,UINT32,CType,CStructType,GenericNode,GenericSink,GenericSource

from html import escape


#tfType = CStructType("TfLiteTensor",10)
#posType = CStructType("struct_position",8)

#bufferType = CType(SINT8)
activateType = CType(UINT32)

class CImageType(CGStaticType):
    YUV = 1
    RGB = 2
    GRAY16 = 3
    GRAY8 = 4

    def __init__(self,w,h,t=YUV):
        CGStaticType.__init__(self)
        self._w = w 
        self._h = h
        self._pixel_type = t

    def __eq__(self, other):
      return(CGStaticType.__eq__(self,other) and 
             self._w == other._w and
             self._h == other._h and
             self._pixel_type == other._pixel_type)

    @property
    def width(self):
        return self._w

    @property
    def height(self):
        return self._h

    @property
    def format(self):
        return self._pixel_type
    
    
    
    @property
    def bytes(self):
        # The C code is using array of int8
        # This type is just used at Python / Graphviz level
        return 1
        #if self._pixel_type == CImageType.YUV:
        #   return(int(1.5*self._w*self._h))
        #if self._pixel_type == CImageType.RGB:
        #   return(int(3*self._w*self._h))
        # 
          
    @property
    def _nb_bytes(self):
        if self._pixel_type == CImageType.YUV:
           return(int(1.5*self._w*self._h))
        if self._pixel_type == CImageType.RGB:
           return(int(3*self._w*self._h))
        if self._pixel_type == CImageType.GRAY16:
           return(int(2*self._w*self._h))
        if self._pixel_type == CImageType.GRAY8:
           return(int(self._w*self._h))

    @property
    def ctype(self):
        return(CType(SINT8).ctype)

    @property
    def graphViztype(self):
        if self.format == CImageType.YUV:
           return(escape(f"YUV_{self.width}_{self.height}"))
        if self.format == CImageType.RGB:
           return(escape(f"RGB_{self.width}_{self.height}"))
        if self.format == CImageType.GRAY16:
           return(escape(f"GRAY16_{self.width}_{self.height}"))
        if self.format == CImageType.GRAY8:
           return(escape(f"GRAY8_{self.width}_{self.height}"))


class SendResult(GenericSink):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        self.addInput("jpeg",CType(SINT8),w*h+100)
        self.addInput("l",CType(UINT32),1)

        self.addVariableArg("env")
        self.addVariableArg("alg_result")
        self.addVariableArg("alg_fm_result")
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "SendResult"



class Camera(GenericSource):
    APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X = 1
    APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X = 2
    APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X = 3
    APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X = 4
    APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X = 5
    APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X = 6

    @property 
    def nb_bytes(self):
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X:
           return 921600 # 640*480*3
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X:
           return 230400 # 320*240*3
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X:
           return 57600 # 160*120*3

        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X:
           return 460800 # 640*480*1.5
        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X:
           return 115200 # 320*240*1.5
        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X:
           return 28800 # 160*120*1.5

    @property
    def width(self):
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X:
           return 640
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X:
           return 320
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X:
           return 160

        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X:
           return 640
        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X:
           return 320
        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X:
           return 160

    @property
    def height(self):
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X:
           return 480
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X:
           return 240
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X:
           return 120

        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X:
           return 480
        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X:
           return 240
        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X:
           return 120
    
    @property
    def image_type(self):
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X:
           return CImageType(self.width,self.height,CImageType.RGB)
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X:
           return CImageType(self.width,self.height,CImageType.RGB)
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X:
           return CImageType(self.width,self.height,CImageType.RGB)

        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X:
           return CImageType(self.width,self.height,CImageType.YUV)
        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X:
           return CImageType(self.width,self.height,CImageType.YUV)
        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X:
           return CImageType(self.width,self.height,CImageType.YUV)

    @property
    def camera_define(self):
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X:
           return "APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X"
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X:
           return "APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X"
        if self._mode == Camera.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X:
           return "APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X"

        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X:
           return "APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X"
        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X:
           return "APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X"
        if self._mode == Camera.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X:
           return "APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X"
    
    def __init__(self,name,mode=APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X):
        GenericSource.__init__(self,name)
        self._mode = mode
        img_t = self.image_type
        self.addOutput("o",img_t,img_t._nb_bytes)
        

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Camera"



class YUVToRGB(GenericNode):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        src_t = CImageType(w,h,CImageType.YUV)
        dst_t = CImageType(w,h,CImageType.RGB)

        self.addInput("i",src_t,src_t._nb_bytes)
        self.addOutput("o",dst_t,dst_t._nb_bytes)
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "YUVToRGB"

class YUVToGray16(GenericNode):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        src_t = CImageType(w,h,CImageType.YUV)
        dst_t = CImageType(w,h,CImageType.GRAY16)

        self.addInput("i",src_t,src_t._nb_bytes)
        self.addOutput("o",dst_t,dst_t._nb_bytes)
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "YUVToGray16"


class Gray16ToRGB(GenericNode):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        src_t = CImageType(w,h,CImageType.GRAY16)
        dst_t = CImageType(w,h,CImageType.RGB)

        self.addInput("i",src_t,src_t._nb_bytes)
        self.addOutput("o",dst_t,dst_t._nb_bytes)
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Gray16ToRGB"

class Gray8ToRGB(GenericNode):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        src_t = CImageType(w,h,CImageType.GRAY8)
        dst_t = CImageType(w,h,CImageType.RGB)

        self.addInput("i",src_t,src_t._nb_bytes)
        self.addOutput("o",dst_t,dst_t._nb_bytes)
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Gray8ToRGB"

class CannyEdge(GenericNode):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        src_t = CImageType(w,h,CImageType.GRAY16)
        dst_t = CImageType(w,h,CImageType.GRAY8)

        self.addInput("i",src_t,src_t._nb_bytes)
        self.addOutput("o",dst_t,dst_t._nb_bytes)
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "CannyEdge"

class JPEGEncoder(GenericNode):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        src_t = CImageType(w,h,CImageType.RGB)

        self.addInput("i",src_t,src_t._nb_bytes)
        # JPEG header + max length buffer
        self.addOutput("jpeg",CType(SINT8),w*h+100)
        # Real size in buffer
        self.addOutput("l",CType(UINT32),1)

        self.addLiteralArg(w)
        self.addLiteralArg(h)
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "JPEGEncoder"