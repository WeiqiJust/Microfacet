import os
import sys
import cv2

def resize(folder, fileName, factor):
    filePath = os.path.join(folder, fileName)
    im = cv2.imread(filePath)
    newIm = cv2.resize(im, (factor, factor), interpolation = cv2.INTER_AREA)
    if (fileName[-3:].lower() == "jpg"):
        os.remove(filePath)
    filePath = filePath[:-3] + 'png'
    cv2.imwrite(filePath, newIm)

def bulkResize(imageFolder, factor):
    imgExts = ["png", "bmp", "jpg"]
    for path, dirs, files in os.walk(imageFolder):
        for fileName in files:
            ext = fileName[-3:].lower()
            if ext not in imgExts:
                continue

            resize(path, fileName, factor)

if __name__ == "__main__":
    imageFolder=sys.argv[1] # first arg is path to image folder
    resizeFactor=256# 2nd is resize in %
    bulkResize(imageFolder, resizeFactor)
