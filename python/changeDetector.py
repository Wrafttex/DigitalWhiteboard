# https://github.com/Chr1ll3D/real-time-whiteboard-app-main/blob/master/app/src/main/java/com/whiteboardapp/core/pipeline/ChangeDetector.java

import cv2
import numpy as np

class changeDetector():

    def __init__(self, size) -> None:
        self.prevImgChanges = np.ones(size, dtype=np.uint8)

    # Detects changes
    # NB: cv2.Mats' should be binary images ie. 1 channel
    def detectChanges(self, img1:np.ndarray, img2:np.ndarray) -> cv2.Mat:

        currentImgChanges:np.ndarray = cv2.absdiff(img1, img2).astype(np.uint8)
        cv2.imshow("currentImgChanges", currentImgChanges)
        imgPersistentChanges = self.getPersistentChanges(self.prevImgChanges, currentImgChanges)

        self.prevImgChanges = currentImgChanges

        return imgPersistentChanges
    

    #Returns image with persistent changes if any. Result image will be black background with white changes.
    def getPersistentChanges(self, prevImgChanges:np.ndarray, currentImgChanges:np.ndarray) -> np.ndarray:

        # imgPersistentChanges:np.ndarray = np.zeros(size, np.uint8)

        # if (prevImgChanges != None):
        #     # Find changes that survived from previous round and set changes to white.
        #     # Make persistent changes white on black background.

        #     # for (int i = 0 i < prevImgChanges.rows() i++):
        #     #     for (int j = 0 j < prevImgChanges.cols() j++):
        #     #         if (prevImgChanges.get(i, j)[0] == 255 && currentImgChanges.get(i, j)[0] == 255):
        #     #             imgPersistentChanges.put(i, j, 255)

        #     bufferPrevChanges = prevImgChanges.copy()
        #     bufferCurrentChanges = currentImgChanges.copy()
        #     bufferPersistentChanges = imgPersistentChanges.copy()

        #     for i in range(len(bufferPrevChanges)):
        #         if (bufferPrevChanges[i] == -1 and bufferCurrentChanges[i] == -1):
        #             bufferPersistentChanges[i] = -1
                
            
        #     imgPersistentChanges[0,0] = bufferPersistentChanges

        imgPersistentChanges = cv2.bitwise_and(prevImgChanges, currentImgChanges)
        

        return imgPersistentChanges