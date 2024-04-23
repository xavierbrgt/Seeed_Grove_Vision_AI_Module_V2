from cmsis_stream.cg.scheduler import CGStaticType,SINT8,UINT32,CType,CStructType,GenericNode,GenericSink,GenericSource

from html import escape

from .node_types import *

class TF12(GenericNode):
    def __init__(self,name,addr,i,o1,o2):
        GenericSink.__init__(self,name)

        self.addInput("i",CType(SINT8),i)
        self.addOutput("o1",CType(SINT8),o1)
        self.addOutput("o2",CType(SINT8),o2)
        self.addLiteralArg(i)
        self.addLiteralArg(o1)
        self.addLiteralArg(o2)
        
        if isinstance(addr,str):
           self.addVariableArg(addr)
        else:
           self.addLiteral(addr)

        self.i.setBufferConstraint(mustBeArray=True,assignedByNode=True)
        self.o1.setBufferConstraint(mustBeArray=True,assignedByNode=True)
        self.o2.setBufferConstraint(mustBeArray=True,assignedByNode=True)

        

class TF11(GenericNode):
    def __init__(self,name,addr,i,o):
        GenericSink.__init__(self,name)

        self.addInput("i",CType(SINT8),i)
        self.addOutput("o",CType(SINT8),o)
        self.addLiteralArg(i)
        self.addLiteralArg(o)


        if isinstance(addr,str):
           self.addVariableArg(addr)
        else:
           self.addLiteral(addr)

        self.i.setBufferConstraint(mustBeArray=True,assignedByNode=True)
        self.o.setBufferConstraint(mustBeArray=True,assignedByNode=True)
  

class FaceDetection(TF12):
    def __init__(self,name):
        TF12.__init__(self,name,"fd_model_addr",160*160,10*10*18,5*5*18)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "FaceDetection"

class FaceMesh(TF12):
    def __init__(self,name):
        TF12.__init__(self,name,"fm_model_addr",192*192*3,1,1404)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "FaceMesh"

class IrisLoc(TF11):
    def __init__(self,name):
        TF11.__init__(self,name,"il_model_addr",64*64*3,213,15)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "IrisLoc"