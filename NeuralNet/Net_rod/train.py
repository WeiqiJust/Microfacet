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

from NetClass import rod_network
from utils import DataLoader_rod

params_global = {}
params_global['outFolder'] = r'Results'
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
    params['predictDataset'] = config.get('dataset', 'predictDataset')
     
    #display and testing
    params['displayStep'] = config.getint('display', 'displayStep')
    params['loopdisplayStep'] = config.getint('display', 'loopdisplayStep')
    params['checkPointStepIteration'] = config.getint('display', 'checkPointStepIteration')
    params['checkPointStepEpoch'] = config.getint('display', 'checkPointStepEpoch')

    return params

def buildParamList(roughness_range, diffuse_range, density_range, theta_range, phi_range):
    paramlist = []
    dataList = []
    for r in roughness_range:
        for d in diffuse_range:
            for de in density_range:
                for t in theta_range:
                    for p in phi_range:
                        paramlist.append([r,d,de,t,p])
                        dataList.append('{}_{}_{}_{}_{}'.format(r, d, de, t, p)) 
    return paramlist, dataList

def DataLoadProcess(queue, datasetfolder, params, roughness_range, diffuse_range, density_range, theta_range, phi_range, unlabel = 0):
    paramlist, dataList = buildParamList(roughness_range, diffuse_range, density_range, theta_range, phi_range)

    batchSize = params['batchSize']
    dataset = DataLoader_rod(datasetfolder, paramlist, 256, 256, 256, 256, True) #128,128,256,256,True
    dataset.buildSubDataset(dataList)
    dataset.shuffle(params['randomSeed'])

    queue.put(dataset.dataSize)
    queue.put(paramlist)
    queue.put((roughness_range, diffuse_range, density_range, theta_range, phi_range))

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
    testloss_de = 0
    testloss_t = 0
    testloss_p = 0

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
        testloss_de = testnet.blobs['DensityLoss'].data / testCount
        testloss_t += testnet.blobs['ThetaLoss'].data / testCount
        testloss_p = testnet.blobs['PhiLoss'].data / testCount
        testloss += (testloss_r + testloss_d + testloss_de + testloss_t + testloss_p) / testCount
    
    return testloss, testloss_r, testloss_d, testloss_de, testloss_t, testloss_p

if __name__ == '__main__':
    #read params
    configFilePath = "config.ini"
    outTag = "rod"
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

    BRDFNet = rod_network()

    
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
    roughness_range = [0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9]
    diffuse_range = [0.05, 0.2, 0.4, 0.6, 0.8]
    density_range = [1,2,3,4,5,6,7,8,9,10]
    theta_range = [0, 15, 30, 45, 60, 75, 90]
    phi_range = [0, 36, 72, 108, 144, 180, 216, 252, 288, 324]

    trainingQueueLength = 200
    logger.info('Init dataset...')
    logger.info('Sync Loading queue size:{}'.format(trainingQueueLength))
    
    data_queue_train = MultiQueue(trainingQueueLength)
    data_queue_loop = MultiQueue(trainingQueueLength)

    rootPath, file = os.path.split(params['dataset'])

    logger.info('Labeled data: {}'.format(params['dataset']))
    logger.info('BRDF Range:')

    loader_train = Process(target = DataLoadProcess, args = (data_queue_train, params['dataset'], params, roughness_range, diffuse_range, density_range, theta_range, phi_range, 0)) #load label dataset
    loader_train.daemon = True
    loader_train.start()

    #init test dataset
    roughness_range_test = [0.2, 0.4, 0.6, 0.8] 
    diffuse_range_test = [0.1, 0.3, 0.5, 0.7, 0.9]
    density_range_test = [1.5, 3.5, 5.5, 7.5, 9.5]
    theta_range_test = [20, 40, 60, 80]
    phi_range_test = [60, 120, 180, 240, 300]

    testParam, testDataList = buildParamList(roughness_range_test, diffuse_range_test, density_range_test, theta_range_test, phi_range_test)
    testSet_Full = DataLoader_grid_plane(params['testDataset'], testParam, 256, 256, 256, 256, True)
    testSet_Full.buildSubDataset(testDataList)
    testSet_Full.shuffle()        

    logger.info('Waiting for load some data first...\n')
    time.sleep(60)

    datasize = data_queue_train.get()
    fullBrdfList = data_queue_train.get()
    roughness, diffuse, denstiy, theta, phi = data_queue_train.get()
    print (roughness)
    print (diffuse)
    print (denstiy)
    print (theta)
    print (phi)

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
    avgLossEveryDisplayStep_de = 0
    avgLossEveryDisplayStep_t = 0
    avgLossEveryDisplayStep_p = 0

    avgLoopLossEveryDisplayStep = 0

    avgLossEveryCheckPoint = 0
    avgLossEveryCheckPoint_r = 0
    avgLossEveryCheckPoint_d = 0
    avgLossEveryCheckPoint_de = 0
    avgLossEveryCheckPoint_t = 0
    avgLossEveryCheckPoint_p = 0

    startTime = time.time()

    trainfigure = plt.figure(1)

    trainlosslist = []
    traintestlosslist = [[],[],[],[],[],[]]
    testlosslist = [[],[],[],[]]
    testlossFulllist = [[],[],[],[], [], []]
    looplosslist = []
    
    print ("Start Training......")

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
            iterLoss_de = net.blobs['DensityLoss'].data
            iterLoss_t = net.blobs['ThetaLoss'].data
            iterLoss_p = net.blobs['PhiLoss'].data
            iterLoss = iterLoss_r + iterLoss_d + iterLoss_de + iterLoss_t + iterLoss_p


            avgLossEveryDisplayStep += iterLoss / params['displayStep']
            avgLossEveryDisplayStep_r += iterLoss_r / params['displayStep']
            avgLossEveryDisplayStep_d += iterLoss_d / params['displayStep']
            avgLossEveryDisplayStep_de += iterLoss_de / params['displayStep']
            avgLossEveryDisplayStep_t += iterLoss_t / params['displayStep']
            avgLossEveryDisplayStep_p += iterLoss_p / params['displayStep']
                        
            avgLossEveryCheckPoint_r += iterLoss_r / params['checkPointStepIteration']
            avgLossEveryCheckPoint_d += iterLoss_d / params['checkPointStepIteration']
            avgLossEveryCheckPoint_de += iterLoss_de / params['checkPointStepIteration']
            avgLossEveryCheckPoint_t += iterLoss_t / params['checkPointStepIteration']
            avgLossEveryCheckPoint_p += iterLoss_p / params['checkPointStepIteration']
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
                avgLossEveryDisplayStep_de = 0
                avgLossEveryDisplayStep_t = 0
                avgLossEveryDisplayStep_p = 0

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
                traintestlosslist[2].append(avgLossEveryCheckPoint_de)
                traintestlosslist[3].append(avgLossEveryCheckPoint_t)
                traintestlosslist[4].append(avgLossEveryCheckPoint_p)
                traintestlosslist[5].append(avgLossEveryCheckPoint)

                logger.info('Testing on Full dataset...')
                testloss, testloss_r, testloss_d, testloss_de, testloss_t, testloss_p = testFitting(testnet, testSet_Full, 10000)
                displayTestLoss = 0

                displayTestLoss = testloss

                logger.info('Full loss = {}'.format(displayTestLoss))                  
                testlossFulllist[0].append(testloss_r)
                testlossFulllist[1].append(testloss_d)
                testlossFulllist[2].append(testloss_de)
                testlossFulllist[3].append(testloss_t)
                testlossFulllist[4].append(testloss_p)
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
                avgLossEveryCheckPoint_de = 0
                avgLossEveryCheckPoint_t = 0
                avgLossEveryCheckPoint_p = 0
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
               os.system(r'python test.py {}/final.caffemodel {} {} {}'.format(outfolder, params['predictDataset'], outtag, gpuid))
            '''
            break
            
