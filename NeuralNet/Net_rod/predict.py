import random, os, sys, time
working_path = os.path.dirname(os.path.realpath(__file__))
root_path = os.path.dirname(working_path)
sys.path.append(root_path + r'/Utils')

import caffe
from utils import save_pfm, load_pfm, pfmFromBuffer, pfmToBuffer, DataLoader_rod
import numpy as np
import logging
import matplotlib
import matplotlib.pyplot as plt
import json
import glob
import math
import shutil
import cv2

from skimage.measure import compare_ssim as ssim
from configparser import ConfigParser
from utils import load_img

matplotlib.rcParams.update({'font.size': 14})

def test_single_channel(testnet, img, color):
    roughness = []
    diffuse = []
    density = []
    t = []
    p = []
    if (color):
        for i in range(3):
            testnet.blobs['Data_Image'].data[...] = img[0,i,:,:]
            testnet.forward()

            roughness.append(testnet.blobs['Out_Roughness'].data.flatten()[0])
            diffuse.append(testnet.blobs['Out_Diffuse'].data.flatten()[0])
            density.append(testnet.blobs['Out_Density'].data.flatten()[0])
            t.append(testnet.blobs['Out_Theta'].data.flatten()[0])
            p.append(testnet.blobs['Out_Phi'].data.flatten()[0])
    else:
        testnet.blobs['Data_Image'].data[...] = img[0,0,:,:]
        testnet.forward()

        roughness.append(testnet.blobs['Out_Roughness'].data.flatten()[0])
        diffuse.append(testnet.blobs['Out_Diffuse'].data.flatten()[0])
        density.append(testnet.blobs['Out_Density'].data.flatten()[0])
        t.append(testnet.blobs['Out_Theta'].data.flatten()[0])
        p.append(testnet.blobs['Out_Phi'].data.flatten()[0])
                                   
    return roughness, diffuse, density, x, y


if __name__ == '__main__':

    #renderContext = {}
    #jinjiaEnv = jinja2.Environment(loader = jinja2.FileSystemLoader('./')).get_template('template.html')

    savedNet = sys.argv[1]
    predictDataset = sys.argv[2]
    outtag = sys.argv[3]
    gpuid = int(sys.argv[4])

    resultfolder, modelfile = os.path.split(savedNet)
    
    outputFolder = resultfolder + r'/test_{}'.format(outtag)
    if(os.path.exists(outputFolder) == False):
        os.makedirs(outputFolder)

    caffe.set_random_seed(23333)
    caffe.set_mode_gpu()
    caffe.set_device(gpuid)
    
    logger = logging.getLogger(__name__)
    logger.setLevel(logging.INFO)
    fh = logging.FileHandler(outputFolder + '/test_log_text.log')
    fh.setLevel(logging.DEBUG)
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s: %(message)s')
    ch.setFormatter(formatter)
    fh.setFormatter(formatter)
    logger.addHandler(ch)
    logger.addHandler(fh)

    testnet = caffe.Net(resultfolder + r'/net_test.prototxt', caffe.TEST)
    logger.info('Loading saved model: {}'.format(savedNet))
    testnet.copy_from(savedNet)

    randomClip = False
    width = 256
    height = 256

    albedo = []
    gray = []
    convert_gray = []
    testPath = predictDataset
    img_in = np.zeros((1,3, width,height))
    gray_img_in = np.zeros((1,1, width,height))
    count = 0
    imgExts = ["png", "bmp", "jpg"]
    for path, dirs, files in os.walk(testPath):
        for fileName in files:
            ext = fileName[-3:].lower()
            if ext not in imgExts:
                continue
    
            fullpath = os.path.join(path, fileName)
            filename = fileName[:-4]

            if (filename[-5:] == "color"):
                color = 1
            else:
                color = 0

            img = cv2.imread(fullpath)
            rawheight = img.shape[0]
            rawwidth = img.shape[1]

            img = load_img(fullpath, width, height)
            #need to handle different channel
            count = count + 1
            img_in[0,:,:,:] = img.transpose((2,0,1))
            roughness, diffuse, density, t, p = test_single_channel(testnet, img_in, color)
            if (color):
                albedo.append([filename[:-6], diffuse[2], diffuse[1],diffuse[0]])
                gray_image = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
                gray_img_in[0,0,:,:] = gray_image
                roughness, diffuse, density, t, p = test_single_channel(testnet, gray_img_in, 0)
                convert_gray.append([filename[:-6], roughness[0], diffuse[0], density[0], t[0], p[0]])
            else:
                gray.append([filename[:-5], roughness[0], diffuse[0], density[0], t[0], p[0]])


    if (testPath != ''):
         with open(testPath + r'/predict_gray_rod.txt', 'w') as f1:
            f1.write(str(len(gray)) + '\n')
            for l in gray:
                f1.write(' '.join(map(str, l)) + '\n')
         with open(testPath + r'/predict_color_rod.txt', 'w') as f1:
            f1.write(str(len(albedo)) + '\n')
            for l in albedo:
                f1.write(' '.join(map(str, l)) + '\n')
         with open(testPath + r'/predict_convert_gray_rode.txt', 'w') as f1:
            f1.write(str(len(convert_gray)) + '\n')
            for l in convert_gray:
                f1.write(' '.join(map(str, l)) + '\n')
