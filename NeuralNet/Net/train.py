import random, os, sys, time, json, pickle, glob, logging, math, shutil, itertools
working_path = os.path.dirname(os.path.realpath(__file__))
root_path = os.path.dirname(working_path)
sys.path.append(root_path + r'/Utils')

from configparser import ConfigParser, SafeConfigParser
from multiprocessing import Process
from multiprocessing import Queue as MultiQueue

import caffe
import numpy as np
import cv2
import matplotlib.pyplot as plt

from NetClass import BRDFNetClassLogLoss_Single_SplitChannal_New_Ratio
from utils import DataLoaderSimple

params_global = {}
params_global['outFolder'] = "T:/NeuralNet/Result"
params_global['scriptRoot'] = r'../Utils'

def loadParams(filepath):
    config = SafeConfigParser({'color':'0', 
                               'PreTrainNet':'',
                               'loopRestartFrequency':'-1'})
    config.read(filepath)

    params = {}
    #device
    params['randomSeed'] = config.getint('device', 'randomSeed')
    #solver
    params['SolverType'] = config.get('solver', 'SolverType')
    params['lr'] = config.getfloat('solver', 'lr')
    params['momentum'] = config.getfloat('solver', 'momentum')
    params['lrDecay'] = config.getfloat('solver', 'lrDecay')
    params['batchSize'] = config.getint('solver', 'batchSize')
    params['weightDecay'] = config.getfloat('solver', 'weightDecay')

    #stopping crteria
    params['nMaxEpoch'] = config.getint('stopping', 'nMaxEpoch')
    params['nMaxIter'] = config.getint('stopping', 'nMaxIter')
  
    #the SA training would alternating between 'normalBatchLength' iteration of normal training and 'loopBatchLength' of self-augment training
    params['normalBatchLength'] = config.getint('loop', 'normalBatchLength')     
    params['loopStartEpoch'] = config.getint('loop', 'loopStartEpoch')      
    params['loopStartIteration'] = config.getint('loop', 'loopStartIteration') #add loop after this number of normal training.
    params['loopBatchLength'] = config.getint('loop', 'loopBatchLength') #how many mini-batch iteration for ever loop optimize
    params['loopRestartFrequency'] = config.getint('loop', 'loopRestartFrequency')
    #network structure
    params['NetworkType'] = config.get('network', 'NetworkType')
    params['Channal'] = config.get('network', 'Channal')
    params['BN'] = config.getboolean('network', 'BN')
    params['color'] = config.getboolean('network', 'color')
    params['PreTrainNet'] = config.get('network', 'PreTrainNet')
    #dataset
    params['NormalizeInput'] = config.getboolean('dataset', 'NormalizeInput')
    params['dataset'] = config.get('dataset', 'dataset')
    params['testDataset'] = config.get('dataset', 'testDataset')
        
    params['testalbedoRange'] = list(map(int, config.get('dataset','testalbedoRange').split(',')))
    params['testspecRange'] = list(map(int, config.get('dataset','testspecRange').split(',')))
    params['testroughnessRange'] = list(map(int, config.get('dataset','testroughnessRange').split(',')))
     
    #display and testing
    params['displayStep'] = config.getint('display', 'displayStep')
    params['loopdisplayStep'] = config.getint('display', 'loopdisplayStep')
    params['checkPointStepIteration'] = config.getint('display', 'checkPointStepIteration')
    params['checkPointStepEpoch'] = config.getint('display', 'checkPointStepEpoch')

    return params


def DataLoadProcess(queue, datasetfolder, params, unlabel = 0):
    fullBrdfList = []
    roughness = [0.3]#[0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9]
    diffuse = [0.05, 0.2, 0.4, 0.6, 0.8]
    scale = [0, 0.05, 0.1]
    x_range = [0.1, 0.2, 0.4, 0.6, 0.8]
    y_range = [0.1, 0.2, 0.4, 0.6, 0.8]
    for r in roughness:
        for d in diffuse:
            for s in scale:
                if (s != 0):
                    for x in x_range:
                       for y in y_range:
                            fullBrdfList.append([r,d,s,x,y])
                else:
                    fullBrdfList.append([r,d,s,0,0])

    batchSize = params['batchSize']
    dataset = DataLoaderSimple(datasetfolder, fullBrdfList, 128, 128, 128, 128, False) #128,128,256,256,True
    dataset.buildSubDataset(roughness, diffuse, scale, x_range, y_range, 10, 10)
    dataset.shuffle(params['randomSeed'])

    queue.put(dataset.dataSize)
    queue.put(fullBrdfList)
    queue.put((roughness, diffuse, scale, x_range, y_range))

    counter = 0
    posInDataSet = 0
    epoch = 0  

    while(True):
        #load image and brdf batch from image file based on interative brdf values
        imgbatch, name = dataset.GetBatchWithName(posInDataSet, batchSize, params['color'])#dataset.GetBatch(posInDataSet, batchSize)
        queue.put((imgbatch, name))
        counter = counter + batchSize
        posInDataSet = (posInDataSet + batchSize) % dataset.dataSize
        newepoch = counter / dataset.dataSize
        if(newepoch != epoch):
            dataset.shuffle()
        epoch = newepoch

if __name__ == '__main__':
    #read params
    configFilePath = "config.ini"
    outTag = "microfacet_training"
    gpuid = 0

    date = time.strftime(r"%Y%m%d_%H%M%S")
    outfolder = params_global['outFolder'] + r'/{}_{}'.format(outTag, date)

    if(os.path.exists(outfolder) == False):
        os.makedirs(outfolder)

    logger = logging.getLogger(__name__)
    logger.setLevel(logging.INFO)
    fh = logging.FileHandler(outfolder + '/training_log_text.log')
    fh.setLevel(logging.DEBUG)
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s: %(message)s')
    ch.setFormatter(formatter)
    fh.setFormatter(formatter)
    logger.addHandler(ch)
    logger.addHandler(fh)

    logger.info('outfolder = {}'.format(outfolder))

    print(configFilePath)

    params = loadParams(configFilePath)

    random.seed(params['randomSeed'])
    np.random.seed(params['randomSeed'])
    logger.info('Setting Seed...')
    caffe.set_random_seed(params['randomSeed'])

    caffe.set_mode_gpu()
    logger.info('Setting GPU...')
    caffe.set_device(gpuid)
    logger.info('Done.')

    logger.info('Loading network and solver settings...')

    BRDFNet = BRDFNetClassLogLoss_Single_SplitChannal_New_Ratio()

    
    BRDFNet.createNet(params['batchSize'], 3, params['BN'], params['NormalizeInput'])

    BRDFNet.saveNet(outfolder)

    #use Adam Solver
    with open(params_global['scriptRoot'] + r'/solver_template.prototxt', 'r') as f:
        solverDescTmp = f.read()
        solverDesc = solverDescTmp.replace('#snapshotpath#', outfolder + '/')
        solverDesc = solverDesc.replace('#netpath#', outfolder + '/net.prototxt')
        solverDesc = solverDesc.replace('#base_lr#', str(params['lr']))
        solverDesc = solverDesc.replace('#momentum#', str(params['momentum']))
        solverDesc = solverDesc.replace('#gamma#', str(params['lrDecay']))
        solverDesc = solverDesc.replace('#weightDecay#', str(params['weightDecay']))
        solverDesc = solverDesc.replace('\\', '/')

    with open(outfolder + r'/solver_forward.prototxt', 'w') as f:
        f.write(solverDesc)

    #load network
    if(params['SolverType'] == 'Adam'):
        solver = caffe.AdamSolver(outfolder + r'/solver_forward.prototxt')
    elif(params['SolverType'] == 'SGD'):
        solver = caffe.SGDSolver(outfolder + r'/solver_forward.prototxt')

    net = solver.net
    testnet = caffe.Net(outfolder + r'/net_test.prototxt', caffe.TEST)

    #init dataset  
    trainingQueueLength = 200
    logger.info('Init dataset...')
    logger.info('Sync Loading queue size:{}'.format(trainingQueueLength))
    
    data_queue_train = MultiQueue(trainingQueueLength)
    data_queue_loop = MultiQueue(trainingQueueLength)

    rootPath, file = os.path.split(params['dataset'])

    logger.info('Labeled data: {}'.format(params['dataset']))
    logger.info('BRDF Range:')

    loader_train = Process(target = DataLoadProcess, args = (data_queue_train, params['dataset'], params, 0)) #load label dataset
    loader_train.daemon = True
    loader_train.start()

    logger.info('Waiting for load some data first...\n')
    time.sleep(60)

    datasize = data_queue_train.get()
    fullBrdfList = data_queue_train.get()
    roughness, diffuse, scale, x_range, y_range = data_queue_train.get()
    print (roughness)
    print (diffuse)
    print (scale)
    print (x_range)
    print (y_range)

