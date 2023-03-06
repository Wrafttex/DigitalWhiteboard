# https://github.com/Chr1ll3D/real-time-whiteboard-app-main/blob/master/app/src/main/java/com/whiteboardapp/core/pipeline/CornerDetector.java

import cv2
import math
import numpy as np

class cornerDetrctor():
    TAG = "CornerDetector"
    THRESHOLD_CANNY_MIN = 20
    THRESHOLD_CANNY_MAX = 150

    BLUR_KERNEL_SIZE = 5
    GAUSS_SIGMA = 1

    DILATION_KERNEL_SIZE = 3

    def __init__(self) -> None:
        pass

    def findCorners(self, imgBgr:np.ndarray):
        imgEdges = self.makeEdgeImage(imgBgr)
        cornerPoints = self.getCorners(imgEdges)
        if (cornerPoints is not None and cornerPoints.shape[0] == 4):
            cornerPoints = self.orderPoints(cornerPoints)

        return cornerPoints

    def makeEdgeImage(self, imgBgr:np.ndarray) -> np.ndarray:
        # Convert to gray scale.
        imgGray = cv2.cvtColor(imgBgr, cv2.COLOR_BGR2GRAY)

        # Blur the image to remove noise.
        imgBlur = cv2.GaussianBlur(imgGray, (self.BLUR_KERNEL_SIZE, self.BLUR_KERNEL_SIZE), self.GAUSS_SIGMA)

        # Find edges.
        imgEdgesCanny = cv2.Canny(imgBlur, self.THRESHOLD_CANNY_MIN, self.THRESHOLD_CANNY_MAX)

        # Enhance edges with dilation
        dilationKernel = np.ones((self.DILATION_KERNEL_SIZE, self.DILATION_KERNEL_SIZE), np.uint8)

        imgEdgesDilated = cv2.dilate(imgEdgesCanny, dilationKernel)

        return imgEdgesDilated

    def getCorners(self, imgEdges:np.ndarray) -> np.ndarray:
        # Find contours
        #contours:list[np.NDArray]
        contours, hierarchy = cv2.findContours(imgEdges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        if (len(contours) == 0):
            return None

        # Find largest shape
        shapePoints = self.findLargestShapePoints(contours)
        cornerPoints = self.approxCornerPoints(shapePoints, imgEdges)
        return cornerPoints

    def findLargestShapePoints(self, contours:np.ndarray):
        PERIMETER_MARGIN_PERCENT = 0.02
        maxPerimeter = 0
        shapePoints = None

        for contour in contours:
            contour2f = np.array(contour)
            # Find contour with the largest perimeter.
            perimeter = cv2.arcLength(contour2f, False)
            if (perimeter > maxPerimeter):
                shapePoints = cv2.approxPolyDP(contour, (PERIMETER_MARGIN_PERCENT * perimeter), False)
                maxPerimeter = perimeter

        print(f"{self.TAG} findLargestShapePoints: Largest shape point count: {shapePoints.shape[0]} (h) {shapePoints.shape[1]} (w)")
        return shapePoints

    def approxCornerPoints(self, shapePoints, img:np.ndarray):
        imgCorners = np.array([[0, 0], [img.shape[1], 0], [0, img.shape[0]], [img.shape[1], img.shape[0]]])

        for i in range(imgCorners.__len__()):
            if shapePoints.__len__() == 0:
                break

            minDistanceIndex = self.getMinDistanceIndex(shapePoints, imgCorners[i])

            imgCorners[i] = shapePoints[minDistanceIndex][0]
            shapePoints = np.delete(shapePoints, minDistanceIndex, 0)

        return imgCorners

    def getMinDistanceIndex(self, points, targetPoint) -> int:
        minDistance = math.inf
        minDistanceIndex = 0
        for i in range(points.__len__()):
            distance = math.sqrt((points[i][0][0] - targetPoint[0]) ** 2 + (points[i][0][1] - targetPoint[1]) ** 2)
            if minDistance > distance:
                minDistance = distance
                minDistanceIndex = i

        return minDistanceIndex

    def orderPoints(self, cornerPoints):
        diffmin = math.inf
        diffMinIndex = 0
        diffMax = 0
        diffMaxIndex = 0
        sumMin = math.inf
        sumMinIndex = 0
        sumMax = 0
        sumMaxIndex = 0

        for i in range(cornerPoints.__len__()):
            diff = cornerPoints[i][0] - cornerPoints[i][1]
            sum = cornerPoints[i][0] + cornerPoints[i][1]

            if diff < diffmin:
                diffmin = diff
                diffMinIndex = i

            if diff > diffMax:
                diffMax = diff
                diffMaxIndex = i

            if sum < sumMin:
                sumMin = sum
                sumMinIndex = i

            if sum > sumMax:
                sumMax = sum
                sumMaxIndex = i

        orderedPoints = np.array([cornerPoints[sumMinIndex], cornerPoints[diffMaxIndex], cornerPoints[sumMaxIndex], cornerPoints[diffMinIndex]])
        return orderedPoints


    def drawCorners(self, cornerPoints, imgBgr:np.ndarray) -> None:
        for p in cornerPoints:
            cv2.circle(imgBgr, p, 30, (0, 255, 0), -1)
