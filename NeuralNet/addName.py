import os
import sys

imageFolder = sys.argv[1] # first arg is path to image folder

imgExts = ["png", "bmp", "jpg"]

for path, dirs, files in os.walk(imageFolder):
    for fileName in files:
        ext = fileName[-3:].lower()
        if ext not in imgExts:
            continue
        oldName = os.path.join(path, fileName)
        filePath = oldName.replace(os.path.sep, '_')
        newName = os.path.join(path, filePath[filePath.index('ward_') + 5:])
        os.rename(oldName, newName)
        


