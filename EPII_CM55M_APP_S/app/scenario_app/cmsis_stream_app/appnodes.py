from cmsis_stream.cg.scheduler import CGStaticType,SINT8,UINT32,CType,CStructType,GenericNode,GenericSink,GenericSource

from html import escape

from nodes import *

class SendResult(GenericSink):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name,identified=False)
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







class JPEGEncoder(GenericNode):
    def __init__(self,name,w,h):
        GenericSink.__init__(self,name,identified=False)
        src_t = CImageType(w,h,CImageType.RGB)

        self.addInput("i",src_t,src_t._nb_bytes)

        # Real size in buffer
        self.addOutput("l",CType(UINT32),1)

        # JPEG header + max length buffer
        self.addOutput("o",CType(SINT8),w*h+100)
        

        self.addLiteralArg(w)
        self.addLiteralArg(h)
        
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "JPEGEncoder"