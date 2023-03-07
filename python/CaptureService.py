# https://github.com/Chr1ll3D/real-time-whiteboard-app-main/blob/2df884466eac2a3e5848b14fc2db9682afc5f45f/app/src/main/java/com/whiteboardapp/core/CaptureService.java
from segmentator import segmentator
from binarization import binarization
from changeDetector import changeDetector

import numpy as np
import time
import cv2

class CaptureService:

    TAG = "CaptureService"

    def __init__(self, defaultWidth:int, defaultHeight:int):
        self.currentModel:np.ndarray = np.ones((defaultWidth, defaultHeight), dtype=np.uint8)*255
        self.changeDetector = changeDetector((defaultWidth, defaultHeight))
        self.binarization = binarization()
    

    # Runs image through the image processing pipeline
    def capture(self, imgBgr:np.ndarray) -> np.ndarray:

        # Segmentation
        print(f"{self.TAG} capture: Segmentation started.")
        matPerspectiveRgb:np.ndarray = cv2.cvtColor(imgBgr, cv2.COLOR_BGR2RGB)
        deeplab = segmentator()
        #Bitmap bitmapRgb = MatConverter.matToBitmap(matPerspectiveRgb)
        imgSegMap:np.ndarray = deeplab.segmentate(matPerspectiveRgb)
        print(f"{self.TAG} capture: Segmentation done.")

        # Binarize a gray scale version of the image.
        imgWarpGray:np.ndarray = cv2.cvtColor(imgBgr, cv2.COLOR_BGR2GRAY)
        imgBinarized:np.ndarray = self.binarization.binarize(imgWarpGray)

        # Remove segments before change detection.
        currentModelCopy:np.ndarray = self.removeSegmentArea(imgBinarized, self.currentModel, imgSegMap, imgBgr)

        # Change detection
        imgPersistentChanges:np.ndarray = self.changeDetector.detectChanges(imgBinarized, currentModelCopy)

        # Update current model with persistent changes.
        self.updateModel(imgBinarized, imgPersistentChanges)
        return self.currentModel

    

    # Removes segment area from image
    def removeSegmentArea(self, binarizedImg:np.ndarray, currentModel:np.ndarray, imgSegMap:np.ndarray, imgPerspective:np.ndarray) -> np.ndarray:
        currentModelCopy:np.ndarray = currentModel.copy()

        startTime = (time.time() / 1000)

        # bufferBinarized = binarizedImg.copy()
        # bufferSegmap = imgSegMap.copy()
        # bufferModel = currentModelCopy.copy()

        # for i in range(len(bufferBinarized)):
        #     if (bufferSegmap[i] == -1):
        #         bufferBinarized[i] = -1
        #         bufferModel[i] = -1

        # binarizedImg.put(0, 0, bufferBinarized)
        # currentModelCopy.put(0, 0, bufferModel)

        binarizedImg = cv2.bitwise_and(binarizedImg, binarizedImg, mask=imgSegMap)
        currentModelCopy = cv2.bitwise_and(currentModelCopy, currentModelCopy, mask=imgSegMap)

        endTime = (time.time() / 1000)
        print(f"Remove segment loop took: {(endTime - startTime)} milliseconds")
        return currentModelCopy
    

    def updateModel(self, imgBinarized:np.ndarray, imgPersistentChanges:np.ndarray) -> None:

        startTime = (time.time() / 1000)

        # bufferModel = self.currentModel.copy()
        # bufferChanges = imgPersistentChanges.copy()
        # bufferBinarized = imgBinarized.copy()

        # for i in range(len(bufferModel)):
        #     if (bufferChanges[i] == -1):
        #         bufferModel[i] = bufferBinarized[i]

        # self.currentModel.put(0, 0, bufferModel)

        self.currentModel = cv2.bitwise_and(self.currentModel, self.currentModel, mask=cv2.bitwise_not(imgPersistentChanges))
        imgBinarized = cv2.bitwise_and(imgBinarized, imgBinarized, mask=imgPersistentChanges)
        self.currentModel = cv2.bitwise_or(self.currentModel, imgBinarized)

        endTime = (time.time() / 1000)
        print(f"Update model loop took: {(endTime - startTime)} milliseconds")