import random, os, sys, time
working_path = os.path.dirname(os.path.realpath(__file__))
root_path = os.path.dirname(working_path)
sys.path.append(root_path + r'/Utils')

import caffe
from utils import save_pfm, load_pfm, pfmFromBuffer, pfmToBuffer, DataLoaderSimple
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

params = {}
matplotlib.rcParams.update({'font.size': 14})