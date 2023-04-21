# https://github.com/Chr1ll3D/real-time-whiteboard-app-main/blob/2df884466eac2a3e5848b14fc2db9682afc5f45f/app/src/main/java/com/whiteboardapp/controller/CaptureActivity.java#L275
from CaptureService import CaptureService
from cornerDetector import cornerDetrctor
from perspectiveTransformer import perspectiveTransformer

import cv2

class CaptureActivity:
    cd = cornerDetrctor()
    pt = perspectiveTransformer()
    cs = None

    def setCorners(self, img):
        # Detect corners
        corners = self.cd.findCorners(img)
        if corners is None:
            return None
        elif corners.__len__() == 4:
            cv2.circle(img, (int(corners[0][0]), int(corners[0][1])), 10, (0, 0, 255), cv2.FILLED)
            cv2.circle(img, (int(corners[1][0]), int(corners[1][1])), 10, (0, 255, 0), cv2.FILLED)
            cv2.circle(img, (int(corners[2][0]), int(corners[2][1])), 10, (255, 0, 0), cv2.FILLED)
            cv2.circle(img, (int(corners[3][0]), int(corners[3][1])), 10, (0, 255, 255), cv2.FILLED)
            cv2.imshow("Corners", img)
            self.corners = corners

    def analyzeImage(self, img):
        # Transform perspective
        imgWarp = self.pt.getPerspective(img, self.corners)
        cv2.imshow("transformedImage", imgWarp)

        # Capture
        if imgWarp is not None:
            if self.cs is None:
                self.cs = CaptureService(imgWarp.shape[0], imgWarp.shape[1])
            currentModel = self.cs.capture(imgWarp)
            if currentModel is not None:
                cv2.imshow("Current Model", currentModel)
    
if __name__ == "__main__":
    ca = CaptureActivity()
    check = False

    webcam = cv2.VideoCapture(0)
    while True:
        ret, frame = webcam.read()
        if ret == True:
            h, w, _ = frame.shape
            if check == False:  
                ca.setCorners(frame)
            else:    
                out = ca.analyzeImage(frame)

        cv2.imshow("Webcam", frame)

        key = cv2.waitKey(1)
        if key == 27:
            break
        elif key == ord('c'):
            check = True

    webcam.release()
    cv2.destroyAllWindows()