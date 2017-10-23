import os
import sys
import cv2

def clip(folder, fileName, outputPath, clipNum):
    filePath = os.path.join(folder, fileName)
    im = cv2.imread(filePath)
    clipSize = im.shape[0] / clipNum
    for i in range(clipNum):
        for j in range(clipNum):
            clipimg = im[i*clipSize:i*clipSize+clipSize, j*clipSize:j*clipSize+clipSize, :]
            directory = os.path.join(outputPath, fileName[0:-4])
            if not os.path.exists(directory):
                os.makedirs(directory)
            cv2.imwrite(os.path.join(directory, '{}_{}.{}'.format(i,j, fileName[-3:])), clipimg)


def bulkClip(imageFolder, outputFolder, clipNum):
    imgExts = ["png", "bmp", "jpg"]
    for path, dirs, files in os.walk(imageFolder):
        for fileName in files:
            ext = fileName[-3:].lower()
            if ext not in imgExts:
                continue
            outputPath = path.replace(path[path.index(imageFolder):path.index(imageFolder)+len(imageFolder)],outputFolder)
            clip(path, fileName, outputPath, clipNum)

if __name__ == "__main__":
    imageFolder=sys.argv[1] # first arg is path to image folder
    outputFolder = sys.argv[2]
    clipNum = 6
    bulkClip(imageFolder, outputFolder, clipNum)
