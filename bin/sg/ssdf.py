from server import*

import numpy as np
import matplotlib.pyplot as plt
import cv2
import torch
import struct
import skfmm
from sklearn.decomposition import PCA

def show_im(im, inGPU = True):
  plt.figure()
  if inGPU:
    plt.imshow(im.cpu().numpy())
  else:
    plt.imshow(im)
  plt.show()

def show_im_plt_con(im):
  plt.cla()
  plt.imshow(im)
  plt.pause(0.000001)

def show_im_cv(im, title = 'render'):
  im = (np.clip(im, 0, 1)*255).astype('uint8')
  cv2.imshow(title, im)
  cv2.waitKey(1)

# up is im, down is ims
def genCompareImg(im, ims, toInt8 = False):
  res = []
  for tt1, tt2 in zip(im, ims):
    tt3 = np.concatenate([tt1, tt2], 0)
    res.append(tt3)

  res = np.concatenate(res, 1)
  if toInt8:
    res = (np.clip(res, 0, 1)*255).astype(np.uint8)
  return res

class SSDF_server:
  def __init__(self, H = 128, W = 128):
    self.conn = Server("127.0.0.1", 8001)
    self.ssdfs = []
    self.H = H
    self.W = W

  def run(self):
    hW = self.W // 2
    while True:
      data = self.conn.receive()
      if len(data) <= 4:
        print('Receive complete')
        break
      envmap = np.frombuffer(data, np.float32).reshape(self.H,self.W)
      #show_im_plt_con(envmap)
      #envmap = np.int32(np.any(envmap[..., None], -1))
      # 0 means object occlude, 1 means not

      # if can not see object at all or occlude everywhere
      # means all value are the same
      envmap = envmap *2 -1
      t_s = np.concatenate([envmap[:,hW:],envmap,envmap[:,:hW]], 1)

      try:
        ssdf = skfmm.distance(t_s, dx = np.pi / 90)
        ssdf = ssdf[:,hW:self.W+hW]
      except:
        ssdf = np.ones_like(envmap)*np.pi/2

      self.ssdfs.append(ssdf)
      #show_im_plt_con(np.concatenate([ssdf, envmap], 1))
      self.conn.send(struct.pack('i', 0))
    
    self.save_path = self.conn.receive().decode()
    self.ssdfs = np.stack(self.ssdfs, 0)
  
  def process_PCA(self, n_components=32, visual_check = False):
    n_ims = self.ssdfs.shape[0]

    print('PCA transform...')
    pca = PCA(n_components)
    pca.fit(self.ssdfs.reshape(n_ims, -1))
    if visual_check:
      print('Check PCA')
      check_data = self.ssdfs[:4].reshape(4, -1)
      trans_data = pca.inverse_transform(pca.transform(check_data))
      tcmp = genCompareImg(check_data.reshape(4,self.H,self.W), 
        trans_data.reshape(4,self.H,self.W), False)
      show_im(tcmp, False)
    print('Save PCA')
    pca_coeff = pca.transform(self.ssdfs.reshape(n_ims, -1)) # n_ims, 32
    pca_component = pca.components_.reshape(n_components,self.H,self.W) # 32, 128, 128
    pca_mean = pca.mean_.reshape(1,self.H,self.W)
    save_data = {
      'coeff' : torch.Tensor(pca_coeff),
      'component' : torch.Tensor(pca_component),
      'mean' : torch.Tensor(pca_mean)
    }
    torch.save(save_data, self.save_path)


    
if __name__ == '__main__':
  server = SSDF_server(74, 148)
  server.run()
  server.process_PCA(128, True)