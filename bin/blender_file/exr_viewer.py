import numpy as np
import sys, os, cv2
os.environ['OPENCV_IO_ENABLE_OPENEXR'] = '1'

def read_image(img_path, blend_a=True, mapping_func = 'HDR'):
  img = cv2.imread(img_path, cv2.IMREAD_UNCHANGED)
  if img.shape[2] == 4:
      img = img[..., :3]*img[..., -1:]
  '''
  NOTE: we read HDR image, but still transfer it to LDR image
  this is to reduce the impact of the huge radiance,
  which makes low quality in dark region after tone mapping.
  Though after transfering, it seems like the same as LDR image,
  but the normal LDR image is discreted with (0~255)/255.0.
  The transfered HDR image will have continued range
  '''
  if mapping_func == 'None':
    pass
  elif mapping_func == 'HDR':
    img = img / (1+img)
    img = np.power(img, 1.0/2.2)
  elif mapping_func == 'Sigmoid':
    img = 1/(1 + np.exp(-img))
  elif mapping_func == 'Tanh':
    img = np.tanh(img)
  else:
    print('Warning: mapping func not found')
  # img[..., :3] = srgb_to_linear(img[..., :3])
  if img.shape[2] == 4: # blend A to RGB
    if blend_a:
      img = img[..., :3]*img[..., -1:]+(1-img[..., -1:])
    else:
      img = img[..., :3]*img[..., -1:]

  return img

hdr_imgs = os.listdir('./hdr_img')

for img_path in hdr_imgs:
  hdr_path = os.path.join('./hdr_img', img_path)
  img = read_image(hdr_path)
  img = (img*255).astype(np.uint8)
  ldr_path = img_path.split('.')[0]+'.png'
  ldr_path = os.path.join('./ldr_img', ldr_path)
  cv2.imwrite(ldr_path, img)