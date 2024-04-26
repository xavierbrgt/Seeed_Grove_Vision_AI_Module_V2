from cmsis_stream.cg.scheduler import CGStaticType,SINT8,UINT32,CType,CStructType,GenericNode,GenericSink,GenericSource

from html import escape

from .node_types import *

class CannyEdge(GenericNode):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        src_t = CImageType(w,h,CImageType.GRAY16)
        dst_t = CImageType(w,h,CImageType.GRAY16)

        self.addInput("i",src_t,src_t._nb_bytes)
        self.addOutput("o",dst_t,dst_t._nb_bytes)
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        self.addVariableArg("params")
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "CannyEdge"


class MVCannyEdge(GenericNode):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name)
        src_t = CImageType(w,h,CImageType.GRAY16)
        dst_t = CImageType(w,h,CImageType.GRAY16)

        self.addInput("i",src_t,src_t._nb_bytes)
        self.addOutput("o",dst_t,dst_t._nb_bytes)
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        self.addVariableArg("params")
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "MVCannyEdge"