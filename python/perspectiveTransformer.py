# https:#github.com/Chr1ll3D/real-time-whiteboard-app-main/blob/master/app/src/main/java/com/whiteboardapp/core/pipeline/PerspectiveTransformer.java

import cv2
import numpy as np

class perspectiveTransformer():

    CROP_MARGIN = 10

    # Limit dimensions to ensure no extreme outliers in the given cornerpoints.
    MIN_DIMENSION = 100
    MAX_DIMENSION = 5000

    def __init__(self) -> None:
        pass

    def hasExtremeDimensions(self, maxWidth:float, maxHeight:float) -> bool:
        return (maxWidth < self.MIN_DIMENSION) or (maxHeight < self.MIN_DIMENSION) or (maxWidth > self.MAX_DIMENSION) or (maxHeight > self.MAX_DIMENSION)

    def getPerspective(self, imgRgb:np.ndarray, cornerPoints:np.ndarray) -> np.ndarray:

        # Approx target corners.
        tl = cornerPoints[0]
        tr = cornerPoints[1]
        br = cornerPoints[2]
        bl = cornerPoints[3]

        widthBtm = np.linalg.norm(bl-br)
        widthTop = np.linalg.norm(tl-tr)
        maxWidth = int(max(widthBtm, widthTop))

        heightRight = np.linalg.norm(br-tr)
        heightLeft = np.linalg.norm(bl-tl)
        maxHeight = int(max(heightRight, heightLeft))

        # Ensure no extreme dimensions.
        if (self.hasExtremeDimensions(maxWidth, maxHeight)):
            return None

        # Create rectangle from approximated corners.
        targetCorners = np.array([
            (0, 0), # tl
            (maxWidth - 1, 0), # tr
            (maxWidth - 1, maxHeight - 1), # br
            (0, maxHeight - 1)], dtype=np.float32) # bl

        #	Get matrix for image perspective perspective
        #	Note: Target corners represents the "real"/actual dimensions of the input points i.e. actual
        #	ratio of paper.
        perspectiveMatrix = cv2.getPerspectiveTransform(cornerPoints.astype(np.float32), targetCorners)

        #	Perform perspective transformation.
        #	Note: The given width and height are for cropping only i.e. does not affect how image is transformed.
        imgPerspective = cv2.warpPerspective(imgRgb, perspectiveMatrix, (maxWidth, maxHeight))
        # cv2.imshow("imgPerspective", imgPerspective)
        croppedMat = self.cropAndResize(imgPerspective, self.CROP_MARGIN)

        return croppedMat

    # Crops image according to specified margin.
    def cropAndResize(self, imgPerspective:np.ndarray, cropMargin:int) -> np.ndarray:
        croppedMat = imgPerspective[cropMargin:imgPerspective.shape[0] - cropMargin, cropMargin:imgPerspective.shape[1] - cropMargin]
        return cv2.resize(croppedMat, (imgPerspective.shape[1], imgPerspective.shape[0]))