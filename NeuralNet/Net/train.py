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

def buildParamList(roughness_range, diffuse_range, scale_range, x_range, y_range, cubemap_cnt, view_cnt):
    paramlist = []
    dataList = []
    for r in roughness_range:
        for d in diffuse_range:
            for s in scale_range:
                if (s != 0):
                    for x in x_range:
                       for y in y_range:
                            paramlist.append([r,d,s,x,y])
                            for c in range(0, cubemap_cnt):
                                for v in range(0, view_cnt):
                                    dataList.append('{}_{}_{}_{}_{}_{}_{}'.format(r, d, s, x, y, c, v))
                else:
                    paramlist.append([r,d,s,0,0])
                    for c in range(0, cubemap_cnt):
                            for v in range(0, view_cnt):
                                dataList.append('{}_{}_{}_{}_{}_{}_{}'.format(r, d, s, 0, 0, c, v))
    return paramlist, dataList

def DataLoadProcess(queue, datasetfolder, params, roughness_range, diffuse_range, scale_range, x_range, y_range, unlabel = 0):
    paramlist, dataList = buildParamList(roughness_range, diffuse_range, scale_range, x_range, y_range, 10, 10)

    batchSize = params['batchSize']
    dataset = DataLoaderSimple(datasetfolder, paramlist, 128, 128, 128, 128, False) #128,128,256,256,True
    dataset.buildSubDataset(dataList)#roughness_range, diffuse_range, scale_range, x_range, y_range, 10, 10)
    dataset.shuffle(params['randomSeed'])

    queue.put(dataset.dataSize)
    queue.put(paramlist)
    queue.put((roughness_range, diffuse_range, scale_range, x_range, y_range))

    counter = 0
    posInDataSet = 0
    epoch = 0  

    while(True):
        #load image and brdf batch from image file based on interative brdf values
        imgbatch, parambath, name = dataset.GetBatchWithName(posInDataSet, batchSize, params['color'])#dataset.GetBatch(posInDataSet, batchSize)
        queue.put((imgbatch, parambath, name))
        counter = counter + batchSize
        posInDataSet = (posInDataSet + batchSize) % dataset.dataSize
        newepoch = counter / dataset.dataSize
        if(newepoch != epoch):
            dataset.shuffle()
        epoch = newepoch

def dumpNetwork(outfolder, solver, filename, statusDict):
    stateFiles = glob.glob(outfolder + r'/*.solverstate')
    for stateFile in stateFiles:
        os.remove(stateFile)
    solver.snapshot()
    files = glob.glob(outfolder + r'/*.caffemodel')
    files.sort(key=os.path.getmtime)
    if(os.path.exists(outfolder + r'/{}.caffemodel'.format(filename))):
        os.remove(outfolder + r'/{}.caffemodel'.format(filename))
    os.rename(files[-1], outfolder + r'/{}.caffemodel'.format(filename))

    files = glob.glob(outfolder + r'/*.solverstate')
    files.sort(key=os.path.getmtime)
    if(os.path.exists(outfolder + r'/{}.solverstate'.format(filename))):
        os.remove(outfolder + r'/{}.solverstate'.format(filename))
    os.rename(files[-1], outfolder + r'/{}.solverstate'.format(filename))    

    #current solve status: (iteration, etc)
    statusFilename = outfolder + r'/{}.caffemodel'.format(filename).replace('.caffemodel', '_status.txt')
    with open(statusFilename, 'w') as f:
        f.write('{}\n'.format(statusDict['iteration']))
        f.write('{}\n'.format(statusDict['loopiteration']))
        f.write('{}\n'.format(statusDict['total_iter']))
        f.write('{}\n'.format(statusDict['epoch']))
        f.write('{}\n'.format(statusDict['loopepoch']))
        f.write('{}\n'.format(statusDict['posInDataset']))
        f.write('{}\n'.format(statusDict['posInUnlabelDataset']))
    np.savetxt(outfolder + r'/trainloss.txt', statusDict['trainlosslist'])
    np.savetxt(outfolder + r'/looploss.txt', statusDict['looplosslist'])
    np.savetxt(outfolder + r'/traintestloss.txt', statusDict['traintestlosslist'])
    np.savetxt(outfolder + r'/testloss.txt', statusDict['testlosslist'])
    np.savetxt(outfolder + r'/testlossfull.txt', statusDict['testlossFulllist'])




def testFitting(testnet, testDataset, testCount):
    testloss = 0
    testloss_r = 0
    testloss_d = 0
    testloss_s = 0
    testloss_x = 0
    testloss_y = 0

    for i in range(0, testCount):
        img, param = testDataset.GetBatch(i, 1, params['color'])
        testnet.blobs['Data_Image'].data[...] = img
        
        img_params = np.zeros((1,5,1,1))
        img_params[:,0,:,:] = param[0,0,0,0]
        img_params[:,1,:,:] = param[0,1,0,0]
        img_params[:,2,:,:] = param[0,2,0,0]
        img_params[:,3,:,:] = param[0,3,0,0]
        img_params[:,4,:,:] = param[0,4,0,0]
        testnet.blobs['Data_BRDF'].data[...] = img_params
        
        testnet.forward()
   
        
        testloss_r += testnet.blobs['RoughnessLoss'].data / testCount
        testloss_d += testnet.blobs['DiffuseLoss'].data / testCount
        testloss_s = testnet.blobs['ScaleLoss'].data / testCount
        testloss_x += testnet.blobs['XLoss'].data / testCount
        testloss_y = testnet.blobs['YLoss'].data / testCount
        testloss += (testloss_r + testloss_d + testloss_s + testloss_x + testloss_y) / testCount
    
    return testloss, testloss_r, testloss_d, testloss_s, testloss_x, testloss_y

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

    
    BRDFNet.createNet(params['batchSize'], 0, params['BN'], params['NormalizeInput'])

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

    #init train dataset 
    roughness_range = [0.3]#[0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9]
    diffuse_range = [0.05, 0.2, 0.4, 0.6, 0.8]
    scale_range = [0, 0.05, 0.1]
    x_range = [0.1, 0.2, 0.4, 0.6, 0.8]
    y_range = [0.1, 0.2, 0.4, 0.6, 0.8]

    trainingQueueLength = 200
    logger.info('Init dataset...')
    logger.info('Sync Loading queue size:{}'.format(trainingQueueLength))
    
    data_queue_train = MultiQueue(trainingQueueLength)
    data_queue_loop = MultiQueue(trainingQueueLength)

    rootPath, file = os.path.split(params['dataset'])

    logger.info('Labeled data: {}'.format(params['dataset']))
    logger.info('BRDF Range:')

    loader_train = Process(target = DataLoadProcess, args = (data_queue_train, params['dataset'], params, roughness_range, diffuse_range, scale_range, x_range, y_range, 0)) #load label dataset
    loader_train.daemon = True
    loader_train.start()
    #init test dataset
    roughness_range_test = [0.3]#[0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9]
    diffuse_range_test = [0.2, 0.4]#[0.1, 0.3, 0.5, 0.7, 0.9]
    scale_range_test = [0, 0.05, 0.1]#[0.025, 0.075, 0.125]
    x_range_test = [0.1, 0.2, 0.4]#[0.1, 0.3, 0.5, 0.7, 0.9]
    y_range_test = [0.1, 0.2, 0.4]#[0.1, 0.3, 0.5, 0.7, 0.9]

    testParam, testDataList = buildParamList(roughness_range_test, diffuse_range_test, scale_range_test, x_range_test, y_range_test, 10, 1)
    testSet_Full = DataLoaderSimple(params['testDataset'], testParam, 128, 128, 128, 128, False)
    testSet_Full.buildSubDataset(testDataList)
    testSet_Full.shuffle()        

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

    params['checkPointStepEpochIteration'] = datasize / params['batchSize'] * params['checkPointStepEpoch']
    if(params['loopStartEpoch'] != -1):
       params['loopStartIteration'] = datasize / params['batchSize'] * params['loopStartEpoch']
       logger.info('Loop Start iteration = {}'.format(params['loopStartIteration']))

    iteration = 0
    loopiteration = 0
    total_iter = 0

    epoch = 0
    loopepoch = 0
    total_epoch = 0

    posInDataset = 0
    posInUnlabelDataset = 0

    avgLossEveryDisplayStep = 0
    avgLossEveryDisplayStep_r = 0
    avgLossEveryDisplayStep_d = 0
    avgLossEveryDisplayStep_s = 0
    avgLossEveryDisplayStep_x = 0
    avgLossEveryDisplayStep_y = 0

    avgLoopLossEveryDisplayStep = 0

    avgLossEveryCheckPoint = 0
    avgLossEveryCheckPoint_r = 0
    avgLossEveryCheckPoint_d = 0
    avgLossEveryCheckPoint_s = 0
    avgLossEveryCheckPoint_x = 0
    avgLossEveryCheckPoint_y = 0

    startTime = time.time()

    trainfigure = plt.figure(1)

    trainlosslist = []
    traintestlosslist = [[],[],[],[],[],[]]
    testlosslist = [[],[],[],[]]
    testlossFulllist = [[],[],[],[]]

    while(True):
        #labeled training
        for i in range(0, params['normalBatchLength']):
            if(total_iter == params['nMaxIter'] or epoch == params['nMaxEpoch']):
                break
            #set data (from training set)
            img_data, parambath, name = data_queue_train.get()
  
            net.blobs['Data_Image'].data[...] = img_data
            net.blobs['Data_BRDF'].data[...] = parambath        

            solver.step(1)
    
            iterLoss_r = net.blobs['RoughnessLoss'].data
            iterLoss_d = net.blobs['DiffuseLoss'].data
            iterLoss_s = net.blobs['ScaleLoss'].data
            iterLoss_x = net.blobs['XLoss'].data
            iterLoss_y = net.blobs['YLoss'].data
            iterLoss = iterLoss_r + iterLoss_d + iterLoss_s + iterLoss_x + iterLoss_y


            avgLossEveryDisplayStep += iterLoss / params['displayStep']
            avgLossEveryDisplayStep_r += iterLoss_r / params['displayStep']
            avgLossEveryDisplayStep_d += iterLoss_d / params['displayStep']
            avgLossEveryDisplayStep_s += iterLoss_s / params['displayStep']
            avgLossEveryDisplayStep_x += iterLoss_x / params['displayStep']
            avgLossEveryDisplayStep_y += iterLoss_y / params['displayStep']
                        
            avgLossEveryCheckPoint_r += iterLoss_r / params['checkPointStepIteration']
            avgLossEveryCheckPoint_d += iterLoss_d / params['checkPointStepIteration']
            avgLossEveryCheckPoint_s += iterLoss_s / params['checkPointStepIteration']
            avgLossEveryCheckPoint_x += iterLoss_x / params['checkPointStepIteration']
            avgLossEveryCheckPoint_y += iterLoss_y / params['checkPointStepIteration']
            avgLossEveryCheckPoint += iterLoss / params['checkPointStepIteration']

            #display training status
            if(total_iter % params['displayStep'] == 0 and total_iter > 0):
                endTime = time.time()
                displayLoss = 0
                displayLoss = avgLossEveryDisplayStep

                logger.info('Total Iter:{} / Normal Iter:{} / Unlabel Iter {}'.format(total_iter, iteration, loopiteration))
                logger.info('Normal Epoch: {}, Loop Epoch: {}'.format(epoch, loopepoch))
                logger.info('Avg Loss = {}, Time = {}'.format(displayLoss, endTime - startTime))
                startTime = time.time() #reset timer
                trainlosslist.append(displayLoss)
                plt.figure(1)
                plt.plot(trainlosslist, 'rs-', label = 'test')
                plt.savefig(outfolder + r'/train.png')

                plt.figure(128)
                plt.gcf().clear()
                plt.plot(trainlosslist[-10::], 'rs-', label = 'test')
                plt.savefig(outfolder + r'/train_last10.png')

                avgLossEveryDisplayStep = 0
                avgLossEveryDisplayStep_r = 0
                avgLossEveryDisplayStep_d = 0
                avgLossEveryDisplayStep_s = 0
                avgLossEveryDisplayStep_x = 0
                avgLossEveryDisplayStep_y = 0

            #snapshot
            if(total_iter % params['checkPointStepIteration'] == 0 and total_iter > 0):
                statusDict = {'iteration': iteration, 'loopiteration': loopiteration, 'total_iter': total_iter, 'epoch': epoch, 'loopepoch': loopepoch,
                            'posInDataset': posInDataset, 'posInUnlabelDataset': posInUnlabelDataset, 'trainlosslist': trainlosslist, 'looplosslist':looplosslist,
                            'traintestlosslist': traintestlosslist, 'testlosslist': testlosslist, 'testlossFulllist': testlossFulllist}
                dumpNetwork(outfolder, solver, 'iter_{}'.format(total_iter), statusDict)
                #test every epoch
                testnet.copy_from(outfolder + r'/iter_{}.caffemodel'.format(total_iter))

                traintestlosslist[0].append(avgLossEveryCheckPoint_r)
                traintestlosslist[1].append(avgLossEveryCheckPoint_d)
                traintestlosslist[2].append(avgLossEveryCheckPoint_s)
                traintestlosslist[3].append(avgLossEveryCheckPoint_x)
                traintestlosslist[4].append(avgLossEveryCheckPoint_y)
                traintestlosslist[5].append(avgLossEveryCheckPoint)

                logger.info('Testing on Full dataset...')
                testloss, testloss_r, testloss_d, testloss_s, testloss_x, testloss_y = testFitting(testnet, testSet_Full, 10000)
                displayTestLoss = 0

                displayTestLoss = testloss

                logger.info('Full loss = {}'.format(displayTestLoss))                  
                testlossFulllist[0].append(testloss_r)
                testlossFulllist[1].append(testloss_d)
                testlossFulllist[2].append(testloss_s)
                testlossFulllist[3].append(testloss_x)
                testlossFulllist[4].append(testloss_y)
                testlossFulllist[5].append(testloss)                 

                '''
                namelist = ['albedo','spec','roughness','total']
                for fid in range(3, 7):
                   plt.figure(fid)
                   plt.plot(traintestlosslist[fid-3], 'rs-', label = 'train')
                   plt.plot(testlosslist[fid-3], 'bs-', label = 'test')
                   plt.plot(testlossFulllist[fid-3], 'gs-', label = 'test_Full')                      
                   plt.savefig(outfolder + r'/test_{}.png'.format(namelist[fid-3]))
                    
                   fig = plt.figure(255 + fid)
                   plt.gcf().clear()
                   plt.plot(traintestlosslist[fid-3][-5::], 'rs-', label = 'train')
                   plt.plot(testlosslist[fid-3][-5::], 'bs-', label = 'test')
                   plt.plot(testlossFulllist[fid-3][-5::], 'gs-', label = 'test_Full')                      
                   plt.savefig(outfolder + r'/test_last5_{}.png'.format(namelist[fid-3]))
                '''

                avgLossEveryCheckPoint_r = 0
                avgLossEveryCheckPoint_d = 0
                avgLossEveryCheckPoint_s = 0
                avgLossEveryCheckPoint_x = 0
                avgLossEveryCheckPoint_y = 0
                avgLossEveryCheckPoint = 0               
                        
            if(iteration % params['checkPointStepEpochIteration'] == 0 and total_iter > 0 and iteration > 0 and params['checkPointStepEpoch'] != -1):
               statusDict = {'iteration': iteration, 'loopiteration': loopiteration, 'total_iter': total_iter, 'epoch': epoch, 'loopepoch': loopepoch,
                            'posInDataset': posInDataset, 'posInUnlabelDataset': posInUnlabelDataset, 'trainlosslist': trainlosslist, 'looplosslist':looplosslist,
                            'traintestlosslist': traintestlosslist, 'testlosslist': testlosslist, 'testlossFulllist': testlossFulllist}
               dumpNetwork(outfolder, solver, 'epoch_{}'.format(epoch), statusDict)

            iteration = iteration + 1
            total_iter = total_iter + 1
            posInDataset = (posInDataset + params['batchSize']) % datasize
            epoch = iteration * params['batchSize'] / datasize

        if(total_iter == params['nMaxIter'] or epoch == params['nMaxEpoch']):
            #write final net
            statusDict = {'iteration': iteration, 'loopiteration': loopiteration, 'total_iter': total_iter, 'epoch': epoch, 'loopepoch': loopepoch,
                         'posInDataset': posInDataset, 'posInUnlabelDataset': posInUnlabelDataset, 'trainlosslist': trainlosslist, 'looplosslist':looplosslist,
                         'traintestlosslist': traintestlosslist, 'testlosslist': testlosslist, 'testlossFulllist': testlossFulllist}
            dumpNetwork(outfolder, solver, 'final', statusDict)
            '''
            if(autoTest):
               logger.info('Visualizing...')
               os.system(r'python TestBRDF.py {}/final.caffemodel {} {}'.format(outfolder, testparampath, gpuid))
            break
            '''