from cmsis_stream.cg.scheduler import CGStaticType,SINT8,UINT32,CType,CStructType,GenericNode,GenericSink,GenericSource

from html import escape

from .node_types import *

class CameraCommon(GenericSource):
    APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X = 1
    APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X = 2
    APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X = 3
    APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X = 4
    APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X = 5
    APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X = 6

    @property 
    def nb_bytes(self):
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X:
           return 921600 # 640*480*3
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X:
           return 230400 # 320*240*3
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X:
           return 57600 # 160*120*3

        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X:
           return 460800 # 640*480*1.5
        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X:
           return 115200 # 320*240*1.5
        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X:
           return 28800 # 160*120*1.5

    @property 
    def nb_jpeg_max_bytes(self):
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X:
           return 76800 # 640*480
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X:
           return 19200 # 320*240

        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X:
           return 76800 # 640*480*1.5
        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X:
           return 19200 # 320*240*1.5

    @property
    def width(self):
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X:
           return 640
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X:
           return 320
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X:
           return 160

        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X:
           return 640
        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X:
           return 320
        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X:
           return 160

    @property
    def height(self):
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X:
           return 480
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X:
           return 240
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X:
           return 120

        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X:
           return 480
        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X:
           return 240
        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X:
           return 120
    
    @property
    def image_type(self):
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X:
           return CImageType(self.width,self.height,CImageType.RGB)
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X:
           return CImageType(self.width,self.height,CImageType.RGB)
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X:
           return CImageType(self.width,self.height,CImageType.RGB)

        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X:
           return CImageType(self.width,self.height,CImageType.YUV)
        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X:
           return CImageType(self.width,self.height,CImageType.YUV)
        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X:
           return CImageType(self.width,self.height,CImageType.YUV)

    @property
    def camera_define(self):
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X:
           return "APP_DP_RES_RGB640x480_INP_SUBSAMPLE_1X"
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X:
           return "APP_DP_RES_RGB640x480_INP_SUBSAMPLE_2X"
        if self._mode == CameraCommon.APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X:
           return "APP_DP_RES_RGB640x480_INP_SUBSAMPLE_4X"

        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X:
           return "APP_DP_RES_YUV640x480_INP_SUBSAMPLE_1X"
        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X:
           return "APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X"
        if self._mode == CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X:
           return "APP_DP_RES_YUV640x480_INP_SUBSAMPLE_4X"

    def __init__(self,name,mode=APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X):
        GenericSource.__init__(self,name)
        self._mode = mode
        


class Camera(CameraCommon):
    def __init__(self,name,mode=CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X):
        CameraCommon.__init__(self,name,mode)
        img_t = self.image_type
        self.addOutput("raw",img_t,img_t._nb_bytes)
        self.raw.setBufferConstraint(name="(void*)app_get_raw_addr()",mustBeArray=True,assignedByNode=False)
        

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Camera"

class CameraJPEG(CameraCommon):
    def __init__(self,name,mode=CameraCommon.APP_DP_RES_YUV640x480_INP_SUBSAMPLE_2X):
        CameraCommon.__init__(self,name,mode)
        img_t = self.image_type
        
        self.addOutput("jpeg",CType(SINT8),self.nb_jpeg_max_bytes)
        self.jpeg.setBufferConstraint(name="(void*)app_get_jpeg_addr()",mustBeArray=True,assignedByNode=False)
                
        # JPEG length
        self.addOutput("l",CType(UINT32),1)

        self.addOutput("raw",img_t,img_t._nb_bytes)
        self.raw.setBufferConstraint(name="(void*)app_get_raw_addr()",mustBeArray=True,assignedByNode=False)


    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "CameraJPEG"