import sys, os, math, random, glob, re
from PIL import Image
from io import StringIO
import numpy as np
import cv2

def make_dir(folder):
    if(os.path.exists(folder) == False):
        os.makedirs(folder)

def autoExposure(img):
    maxValue = np.max(img) + 1e-6
    return img / maxValue

def toHDR(img):
    img = img / 255.0
    img_out = img ** (2.2)
    return img_out.astype(np.float32)

def toLDR(img):
    img_out = img ** (1.0 / 2.2)
    img_out = np.minimum(255, img_out * 255)
    return img_out.astype(np.uint8)

#PFM load and write
def pfmFromBuffer(buffer, reverse = 1):
    sStream = StringIO(buffer)

    color = None
    width = None
    height = None
    scale = None
    endian = None

    header = sStream.readline().rstrip()
    color = (header == 'PF')

    width, height = list(map(int, sStream.readline().strip().split(' ')))
    scale = float(sStream.readline().rstrip())
    endian = '<' if(scale < 0) else '>'
    scale = abs(scale)
    

    rawdata = np.fromstring(sStream.read(), endian + 'f')
    shape = (height, width, 3) if color else (height, width)
    sStream.close()
    if(len(shape) == 3):
        return rawdata.reshape(shape).astype(np.float32)[:,:,::-1]
    else:
        return rawdata.reshape(shape).astype(np.float32)

def pfmToBuffer(img, reverse = 1):
    color = None
    sStream = StringIO()
    img = np.ascontiguousarray(img)
    if(img.dtype.name != 'float32'):
        img = img.astype(np.float32)


    color = True if (len(img.shape) == 3) else False

    if(reverse and color):
        img = img[:,:,::-1]

    sStream.write('PF\n' if color else 'Pf\n')
    sStream.write('%d %d\n' % (img.shape[1], img.shape[0]))
    
    endian = img.dtype.byteorder
    scale = 1.0
    if endian == '<' or endian == '=' and sys.byteorder == 'little':
        scale = -scale

    sStream.write('%f\n' % scale)
    sStream.write(img.tobytes())
    outBuffer = sStream.getvalue()
    sStream.close()
    return outBuffer 
 
def save_pfm(filepath, img, reverse = 1):
    color = None
    file = open(filepath, 'w')
    if(img.dtype.name != 'float32'):
        img = img.astype(np.float32)

    color = True if (len(img.shape) == 3) else False

    if(reverse and color):
        img = img[:,:,::-1]

    file.write('PF\n' if color else 'Pf\n')
    file.write('%d %d\n' % (img.shape[1], img.shape[0]))
    
    endian = img.dtype.byteorder
    scale = 1.0
    if endian == '<' or endian == '=' and sys.byteorder == 'little':
        scale = -scale

    file.write('%f\n' % scale)
    img.tofile(file)
    file.close()

def load_pfm(filepath, reverse = 1):
    file = open(filepath, 'rb')
    color = None
    width = None
    height = None
    scale = None
    endian = None

    header = file.readline().decode('utf8').rstrip()
    color = (header == 'PF')

    (width, height) = list(map(int, file.readline().decode('utf8').strip().split(' ')))
    scale = float(file.readline().decode('utf8').rstrip())
    endian = '<' if(scale < 0) else '>'
    scale = abs(scale)

    rawdata = np.fromfile(file, endian + 'f')
    shape = (height, width, 3) if color else (height, width)
    file.close()

    if(color):  
        return rawdata.reshape(shape).astype(np.float32)[:,:,::-1]
    else:
        return rawdata.reshape(shape).astype(np.float32)


def load_and_clip(filepath, left, top, width, height, reverse = 1):
    name,ext = os.path.splitext(filepath)
    if(ext == '.pfm'):
        img = load_pfm(filepath)
    else:
        img = toHDR(cv2.imread(filepath))
    if(len(img.shape) == 3):
        return img[top:top+height,left:left+width,:]   
    else:
        return img[top:top+height,left:left+width] 

#utilties
def renormalize(normalMap):
    nOut = np.zeros(normalMap.shape)
    for i in range(0, normalMap.shape[0]):
        normal_1 = (2.0 * normalMap[i,:,:,:] - 1).transpose(1,2,0)
        length = np.linalg.norm(normal_1, axis = 2)
        normal_1 = normal_1 / np.dstack((length, length, length))
        nOut[i,:,:,:] = (0.5 * (normal_1 + 1)).transpose(2,0,1)
    return nOut

def normalizeAlbedoSpec(brdfbatch):
    for i in range(0, brdfbatch.shape[0]):
        factor = 0.5 / np.mean(np.linalg.norm(brdfbatch[i,0:3,:,:], axis = 0))
        brdfbatch[i,0:6,:,:] *= factor

    return brdfbatch

#normal map : [0,1] range!
def normalBatchToThetaPhiBatch(data):
    outBatch = np.zeros((data.shape[0], 2, data.shape[2], data.shape[3]))
    data_1 = data * 2 - 1.0
    outBatch[:,0,:,:] = np.arccos(data_1[:,0,:,:])
    outBatch[:,1,:,:] = np.arctan2(data_1[:,1,:,:], data_1[:,2,:,:])
    return outBatch


def thetaPhiBatchToNormalBatch(data):
    outBatch = np.zeros((data.shape[0], 3, data.shape[2], data.shape[3]))
    outBatch[:,0,:,:] = np.cos(data[:,0,:,:])
    outBatch[:,1,:,:] = np.sin(data[:,0,:,:]) * np.sin(data[:,1,:,:])
    outBatch[:,2,:,:] = np.sin(data[:,0,:,:]) * np.cos(data[:,1,:,:])

    outBatch = 0.5*(outBatch + 1.0)

    return outBatch
    #n*3*256*256

def findIndex(query, pList):
    out = np.zeros((len(query)))
    for id, p in enumerate(query):
        out[id] = np.argmin(np.abs(p - pList))

    return out

def listToStr(numlist):
    strList = ['{},'.format(x) for x in numlist]
    strList[-1] = strList[-1][:-1]
    return 

def dssim(img1, img2):
    img1_g = cv2.cvtColor(img1.astype(np.float32), cv2.COLOR_BGR2GRAY)
    img2_g = cv2.cvtColor(img2.astype(np.float32), cv2.COLOR_BGR2GRAY)
    return 0.5 * (1.0 - ssim(img1_g, img2_g))


def meanDownsample(img):
    out = 0.25*(img[0::2, 0::2] + img[1::2, 0::2] + img[0::2, 1::2] + img[1::2, 1::2])
    return out

def genMipMap(texCube):
    #assume width = height and is 2^x
    nLevel = int(min(10, math.log(texCube.shape[1], 2))) + 1
    texMipMapList = []
    texMipMapList.append(texCube)
    for k in range(1, nLevel):
        prevCube = texMipMapList[k-1]
        if(len(prevCube.shape) == 3):
            newCube = np.ones((6, prevCube.shape[1] // 2, prevCube.shape[2] // 2))
        else:
            newCube = np.ones((6, prevCube.shape[1] // 2, prevCube.shape[2] // 2, 4))

        for f in range(0, 6):
            newCube[f] = meanDownsample(prevCube[f])#cv2.pyrDown(prevCube[f])

        texMipMapList.append(newCube.astype(np.float32))
    return texMipMapList

def getTexCube(crossImg):
    #TOBGRA since cuda only accept float4 textures
    if(len(crossImg.shape) == 2):
        crossImg = np.dstack((crossImg, crossImg, crossImg))
    faceRes = crossImg.shape[1] // 4
    width = height = faceRes
    if(len(crossImg.shape) == 3):
        texCube = np.ones((6, faceRes, faceRes, 4))
        texCube[0, :, :, 0:3] = crossImg[faceRes:faceRes+height, 2*faceRes:2*faceRes+width,:]
        texCube[1, :, :, 0:3] = crossImg[faceRes:faceRes+height, 0:width,:]
        texCube[3, :, :, 0:3] = crossImg[0:height, faceRes:faceRes+width,:]
        texCube[2, :, :, 0:3] = crossImg[2*faceRes:2*faceRes+height, faceRes:faceRes+width,:]
        texCube[4, :, :, 0:3] = crossImg[faceRes:faceRes+height, faceRes:faceRes+width,:]
        texCube[5, :, :, 0:3] = crossImg[faceRes:faceRes+height, 3*faceRes:3*faceRes+width,:]
    else:
        texCube = np.ones((6, faceRes, faceRes))
        texCube[0, :, :] = crossImg[faceRes:faceRes+height, 2*faceRes:2*faceRes+width]
        texCube[1, :, :] = crossImg[faceRes:faceRes+height, 0:width]
        texCube[3, :, :] = crossImg[0:height, faceRes:faceRes+width]
        texCube[2, :, :] = crossImg[2*faceRes:2*faceRes+height, faceRes:faceRes+width]
        texCube[4, :, :] = crossImg[faceRes:faceRes+height, faceRes:faceRes+width]
        texCube[5, :, :] = crossImg[faceRes:faceRes+height, 3*faceRes:3*faceRes+width]

    for i in range(0, 6):
        texCube[i, :, :] = texCube[i, ::-1, :]


    return np.ascontiguousarray(texCube, dtype=np.float32)

    


#DataLoader class
def checkVaild(root, mid, lid, vid, oid):
    imgpath = root + r'/m_{}/{}_{}_{}_{}_image.pfm'.format(mid, mid, lid, vid, oid)
    apath = root + r'/m_{}/gt_{}_albedo.pfm'.format(mid, oid)
    spath = root + r'/m_{}/gt_{}_specalbedo.pfm'.format(mid, oid)
    rpath = root + r'/m_{}/gt_{}_roughness.pfm'.format(mid, oid)

    if(os.path.exists(imgpath) and os.path.exists(apath) and os.path.exists(spath) and os.path.exists(rpath)):# and os.path.exists(npath)):
        return True
    else:
        return False

class RealDataLoaderSVBRDF(object):
    dataSize = 0
    rootPath = '' 
    
    dataList = []
    cursorPos = 0
    
    width = 256
    height = 256

    def __init__(self, rootPath, imgListFile):
        with open(rootPath + r'/{}'.format(imgListFile), 'r') as f:
            self.dataList = f.read().strip().split('\n')

        self.rootPath = rootPath        
        self.dataSize = len(self.dataList)
        
        self.cursorPos = 0
        self.width = 256
        self.height = 256
 
    def shuffle(self, seed = []):
        if(seed == []):
            np.random.shuffle(self.dataList)
        else:
            np.random.seed(seed)
            np.random.shuffle(self.dataList)

    def GetImg(self, idx):
        path = r'{}/{}'.format(self.rootPath, self.dataList[idx]).strip()  #for the FUCKING CRLF difference between WINDOWS and Linux
        img = toHDR(cv2.imread(path)).transpose(2,0,1)# / 255.0
        return img[np.newaxis, :, :, :]

    def GetImgWithName(self, idx):
        img = self.GetImg(idx)
        name = self.dataList[idx]
        return img, name

    def GetBatchWithName(self, start, n):
        dataBatch = np.zeros((n, 3, self.height, self.width))
        nameList = []

        tmpSize = self.dataSize
        for i in range(0, n):
            idx = (start + i) % tmpSize
            dataBatch[i, :, :, :],  name = self.GetImgWithName(idx)
            nameList.append(name)

        return dataBatch, nameList

    def GetBatch(self, start, n):
        dataBatch = np.zeros((n, 3, self.height, self.width))

        tmpSize = self.dataSize
        for i in range(0, n):
            idx = (start + i) % tmpSize
            dataBatch[i, :, :, :] = self.GetImg(idx)

        return dataBatch

    def GetNextBatch(self, n):
        dataBatch = self.GetBatch(self.cursorPos, n, unlabel)
        self.cursorPos = (self.cursorPos + n) % self.dataSize

        return dataBatch    

class DataLoaderSimple(object):
    fulldataSize = 0
    dataSize = 0

    brdfCube = []
    fulldataList = []
    dataList = []
    rootPath = ''

    cursorPos = 0
    width = 128
    height = 128

    rawwidth = 256
    rawheight = 256
    randomClip = False


    def __init__(self, datasetfolder, fullBrdfList, width, height, rawwidth, rawheight, randomClip = True):
        self.brdfCube = fullBrdfList
        self.brdfCube_Masked = np.ma.array(self.brdfCube)
        self.brdfCube_Masked.mask = False

        self.rootPath = datasetfolder
        self.cursorPos = 0
        self.width = width
        self.height = height
        self.rawwidth = rawwidth
        self.rawheight = rawheight
        self.randomClip = randomClip

    '''
    def buildSubDataset_2(self, brdf_light_list, inverse = 0):#build a dataset with given lighting samples {a s r}/ {l v} folder/file
        dataList = []
        self.lightCount = -1
        self.brdfCube_Masked.mask = True
        
        for brdf_light in brdf_light_list:
            a, s, r, l, v = brdf_light
            self.brdfCube_Masked.mask[a, s, r, :] = False
            dataList.append('{}_{}_{}_{}_{}'.format(a, s, r, l, v))         

        if(inverse):
            self.dataList = list(set(self.fulldataList) - set(dataList))
            self.dataSize = len(self.dataList)
        else:
            self.dataList = dataList
            self.dataSize = len(self.dataList)

    #build a dataset with unknown lighting samples, generate lighting samples from range info
    def buildSubDataset_1(self, brdf_list, tid_list, pid_list, inverse = 0):
        dataList = []

        self.lightCount = len(tid_list) * len(pid_list)
        self.brdfCube_Masked.mask = True
        
        for brdf in brdf_list:
            a, s, r = brdf
            self.brdfCube_Masked.mask[a, s, r, :] = False
            for t in tid_list:
                for p in pid_list:
                    dataList.append('{}_{}_{}_{}_{}'.format(a, s, r, t, p))         

        if(inverse):
            self.dataList = list(set(self.fulldataList) - set(dataList))
            self.dataSize = len(self.dataList)
        else:
            self.dataList = dataList
            self.dataSize = len(self.dataList)
    '''   

    def buildSubDataset(self, dataList):#roughness, diffues, scale, x_range, y_range, cubemap_cnt, view_cnt):
        '''
        dataList = []
        for r in roughness:
            for d in diffues:
                for s in scale:
                    if (s != 0):
                        for x in x_range:
                            for y in y_range:
                                for c in range(0, cubemap_cnt):
                                    for v in range(0, view_cnt):
                                        dataList.append('{}_{}_{}_{}_{}_{}_{}'.format(r, d, s, x, y, c, v))
                                        #dataList.append('ward_0{}_{}/{}_{}_{}/{}_{}.jpg'.format(r, d, s, x, y, c, v))
                    else:
                        for c in range(0, cubemap_cnt):
                            for v in range(0, view_cnt):
                                dataList.append('{}_{}_{}_{}_{}_{}_{}'.format(r, d, s, 0, 0, c, v))
        '''
        self.dataList = dataList
        print (len(dataList))
        self.dataSize = len(self.dataList)
        self.fulldataSize = len(self.dataList)     

    def shuffle(self, seed = []):
        if(seed == []):
            np.random.shuffle(self.dataList)
        else:
            np.random.seed(seed)
            np.random.shuffle(self.dataList)


    def GetItem(self, idx, color = False):
        r, d, s, x, y, c, v = list(map(float, self.dataList[idx].split('_')))
        return self.GetItemByID(r, d, s, x, y, c, v, color)#img, brdf

    def GetItemByID(self, r, d, s, x, y, c, v, color = False):
        if(self.randomClip):
            clip_left = np.random.randint(0, self.rawwidth - 1 - self.width)
            clip_top = np.random.randint(0, self.rawheight - 1 - self.height)
        else:
            clip_left = self.rawwidth / 2 - self.width / 2
            clip_top = self.rawheight / 2 - self.height / 2
        self.clipPos = [clip_left, clip_top]
        if(color):
            img = load_and_clip(self.rootPath + r'/ward_{}_{}/{}_{}_{}/{}_{}.jpg'.format(r, d, s, x, y, int(c), int(v)), int(clip_left), int(clip_top), self.width, self.height).transpose((2,0,1))
        else:
            img = load_and_clip(self.rootPath + r'/ward_{}_{}/{}_{}_{}/{}_{}.jpg'.format(r, d, s, x, y, int(c), int(v)), int(clip_left), int(clip_top), self.width, self.height)
            if(len(img.shape) == 3):
                img = img[:,:,0]
            img = img[np.newaxis,:,:]
        param = [r, d, s, x, y]
        param = np.array(param).reshape((1,5,1,1))
        return img, param

    def GetItemWithName(self, idx, color = False):
        img, param = self.GetItem(idx, color)
        name = list(map(float, self.dataList[idx].split('_')))
        return img, param, name

    def GetBatchWithName(self, start, n, color = False):
        if(color):
            dataBatch = np.zeros((n, 3, self.height, self.width)) 
        else:
            dataBatch = np.zeros((n, 1, self.height, self.width))
        paramBatch = np.zeros((n, 5, 1, 1))
        nameList = []
        tmpSize = self.dataSize
        for i in range(0, n):
            idx = (start + i) % tmpSize
            dataBatch[i, :, :, :], paramBatch[i,:,:,:], name = self.GetItemWithName(idx, color)
            nameList.append(name)

        return dataBatch, paramBatch, nameList

    def GetBatch(self, start, n, color = False):
        if(color):
            dataBatch = np.zeros((n, 3, self.height, self.width)) 
        else:
            dataBatch = np.zeros((n, 1, self.height, self.width))
        paramBatch = np.zeros((n, 5, 1, 1))
        tmpSize = self.dataSize
        for i in range(0, n):
            idx = (start + i) % tmpSize
            dataBatch[i, :, :, :], paramBatch[i, :, :, :] = self.GetItem(idx, color)

        return dataBatch, paramBatch

    def GetNextBatch(self, n, color = False):
        dataBatch, paramBatch = self.GetBatch(self.cursorPos, n, color)
        self.cursorPos = (self.cursorPos + n) % self.dataSize

        return dataBatch, paramBatch