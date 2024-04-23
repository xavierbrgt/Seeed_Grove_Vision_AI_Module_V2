from cmsis_stream.cg.scheduler import CGStaticType,SINT8,UINT32,CType,CStructType,GenericNode,GenericSink,GenericSource

from html import escape

from .node_types import *

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

class YUVToGray8(GenericNode):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        src_t = CImageType(w,h,CImageType.YUV)
        dst_t = CImageType(w,h,CImageType.GRAY8)

        self.addInput("i",src_t,src_t._nb_bytes)
        self.addOutput("o",dst_t,dst_t._nb_bytes)
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "YUVToGray8"


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