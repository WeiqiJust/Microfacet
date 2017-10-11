import random, os, sys, time
working_path = os.path.dirname(os.path.realpath(__file__))
root_path = os.path.dirname(working_path)
sys.path.append(root_path + r'/Utils')

import caffe
from utils import save_pfm, load_pfm, pfmFromBuffer, pfmToBuffer, DataLoader_grid_plane
import numpy as np
import logging
import matplotlib
import matplotlib.pyplot as plt
import json
import glob
import math
import shutil
import cv2
import jinja2
import pickle

from skimage.measure import compare_ssim as ssim
from configparser import ConfigParser
from utils import load_and_clip

matplotlib.rcParams.update({'font.size': 14})

def test_single_channel(testnet, img):
    roughness = []
    diffuse = []
    scale = []
    x = []
    y = []
    for i in range(3):
        testnet.blobs['Data_Image'].data[...] = img[0,i,:,:]
        testnet.forward()

        roughness.append(testnet.blobs['Out_Roughness'].data.flatten()[0])
        diffuse.append(testnet.blobs['Out_Diffuse'].data.flatten()[0])
        scale.append(testnet.blobs['Out_Scale'].data.flatten()[0])
        x.append(testnet.blobs['Out_X'].data.flatten()[0])
        y.append(testnet.blobs['Out_Y'].data.flatten()[0])
                                   
    return roughness, diffuse, scale, x, y


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
    width = 128
    height = 128

    r = []
    g = []
    b = []
    testPath = predictDataset
    for filename in glob.glob(os.path.join(testPath, '*.jpg')):
        fullpath = filename
        #print('Test {}\n'.format(filename.strip()))

        img = cv2.imread(fullpath)
        rawheight = img.shape[0]
        rawwidth = img.shape[1]
        if(randomClip):
            clip_left = np.random.randint(0, rawwidth - 1 - width)
            clip_top = np.random.randint(0, rawheight - 1 - height)
        else:
            clip_left = rawwidth / 2 - width / 2
            clip_top = rawheight / 2 - height / 2
        clipPos = [clip_left, clip_top]
        img = load_and_clip(fullpath, int(clip_left), int(clip_top), width, height)
        #need to handle different channel

        img_in[0,:,:,:] = img.transpose((2,0,1))
        roughness, diffuse, scale, x, y = test_single_channel(testnet, img_in)
        r.append([roughness[0], diffuse[0], scale[0], x[0], y[0]])
        g.append([roughness[1], diffuse[1], scale[1], x[1], y[1]])
        b.append([roughness[2], diffuse[2], scale[2], x[2], y[2]])

    if (testPath != ''):
        with open(testPath + r'/result_r.txt', 'w') as f1:
            for l in r:
                f1.write(' '.join(map(str, l)) + '\n')

        with open(testPath + r'/result_g.txt', 'w') as f1:
            for l in g:
                f1.write(' '.join(map(str, l)) + '\n')

        with open(testPath + r'/result_b.txt', 'w') as f1:
            for l in b:
                f1.write(' '.join(map(str, l)) + '\n')