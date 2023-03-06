# https:#github.com/Chr1ll3D/real-time-whiteboard-app-main/blob/master/app/src/main/java/com/whiteboardapp/core/pipeline/Segmentator.java

import cv2
import numpy as np
import torch
import torchvision
import time
from PIL import Image
import matplotlib.pyplot as plt

class segmentator():

    TAG = "SegmentationTask"
    NUM_THREADS = 4
    SEGMENTATION_MODEL_NAME = "deeplabv3_257_mv_gpu.tflite"
    ALPHA_VALUE = 128

    maskTensor:np.ndarray
    pixelsGlobal:list[int]

    DEVICE = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    TRANSFORM = torchvision.models.segmentation.DeepLabV3_MobileNet_V3_Large_Weights.COCO_WITH_VOC_LABELS_V1.transforms()
    # torchvision.transforms._presets.SemanticSegmentation
    
    def __init__(self) -> None:
        try:
            self.imageSegmenter = torchvision.models.segmentation.deeplabv3_mobilenet_v3_large(weights='DEFAULT').to(self.DEVICE).eval()
        except Exception as e:
            raise Exception("Exception occurred while loading model from file {e}")

    # Performs segmentation of the given image. 
    # Returns a Mat representing the resulting segmentation map 
    def segmentate(self, image:np.ndarray) -> np.ndarray:
        fullTimeExecutionTime = (time.time() / 1000)
        

        # Do segmentation
        imageSegmentationTime = (time.time() / 1000)
        image_tensor = torch.tensor(image, device=self.DEVICE).permute(2,0,1).unsqueeze(0)
        # image_tensor = image_tensor/255.0
        with torch.inference_mode(True):
            pre = self.TRANSFORM(image_tensor)
            # pre = self.TRANSFORM(image).to(self.DEVICE).unsqueeze(0)
            results = self.imageSegmenter(pre)["out"]

        imageSegmentationTime = (time.time() / 1000) - imageSegmentationTime
        print(f"{self.TAG} Time to run the segmentation model: {imageSegmentationTime} ms")

        output_predictions = results[0].argmax(0)
        
        # palette = torch.tensor([2 ** 25 - 1, 2 ** 15 - 1, 2 ** 21 - 1])
        # colors = torch.as_tensor([i for i in range(21)])[:, None] * palette
        # colors = (colors % 255).numpy().astype("uint8")
        h, w, _ = image.shape
        mask = (output_predictions != 0).byte().cpu().numpy().astype("uint8")*255
        mask = cv2.resize(mask, (w, h), interpolation=cv2.INTER_NEAREST)
        # mask = Image.fromarray(mask).resize(image.size, Image.NEAREST)
        # plt.imshow(mask)
        # plt.show()
        # print(mask)

        # # plot the semantic segmentation predictions of 21 classes in each color
        # r = Image.fromarray(output_predictions.byte().cpu().numpy()).resize(image.size)
        # r.putpalette(colors)
        # print(r.size, mask.size, image.size)
        # image.paste(r, mask=mask)

        fullTimeExecutionTime = (time.time() / 1000) - fullTimeExecutionTime
        print(f"{self.TAG} Total time in segmentation step: {fullTimeExecutionTime} ms")
        return mask

if __name__ == "__main__":
    webcam = cv2.VideoCapture(0)
    ret, frame = webcam.read()

    seg = segmentator()
    out = seg.segmentate(Image.fromarray(frame))

    plt.imshow(frame)
    plt.imshow(out)
    plt.show()

    