# https://github.com/Chr1ll3D/real-time-whiteboard-app-main/blob/master/app/src/main/java/com/whiteboardapp/core/pipeline/Binarization.java

import cv2
import numpy as np

class binarization():
    def __init__(self) -> None:
        pass

    MAX_THRESH_VALUE = 255
    BLOCK_SIZE = 21
    C = 4
    BLUR_KERNEL_SIZE = 3

    # Binarizes a gray scale image.
    def binarize(self, imgGray:np.ndarray) -> np.ndarray:
        imgBinarizedThreshold = cv2.adaptiveThreshold(imgGray, self.MAX_THRESH_VALUE, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY_INV, self.BLOCK_SIZE, self.C)

        # Clean image - removes most of the noise
        imgBinarizedThresholdBlur = cv2.medianBlur(imgBinarizedThreshold, self.BLUR_KERNEL_SIZE)

        # Invert image (black on white background)
        imgBinarizedFinal = cv2.bitwise_not(imgBinarizedThresholdBlur) #TODO

        return imgBinarizedFinal