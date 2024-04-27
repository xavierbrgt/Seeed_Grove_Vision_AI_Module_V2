from cmsis_stream.cg.scheduler import CGStaticType,SINT8,UINT32,CType,CStructType,GenericNode,GenericSink,GenericSource

from html import escape

from .node_types import *

class CannyEdge(GenericNode):
    def __init__(self,name,w,h,firstAlgo=True):
        GenericSink.__init__(self,name)
        src_t = CImageType(w,h,CImageType.GRAY16)
        dst_t = CImageType(w,h,CImageType.GRAY16)

        self.addInput("i",src_t,src_t._nb_bytes)
        self.addOutput("o",dst_t,dst_t._nb_bytes)
        self.addLiteralArg(w)
        self.addLiteralArg(h)
        if firstAlgo:
           self._paramBlock = 1
           self.addVariableArg("params1")
        else:
           self._paramBlock = 2
           self.addVariableArg("params2")
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "CannyEdge"

    @property
    def paramBlock(self):
        return self._paramBlock

    @property
    def params(self):
        return {"name":"CMSIS-CV Canny Edge", 
                "values":
                  [{"value":2500,"name":"low","min":0,"max":3000,"step":10},
                   {"value":1060,"name":"high","min":0,"max":4500,"step":10}
                  ]
                }


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