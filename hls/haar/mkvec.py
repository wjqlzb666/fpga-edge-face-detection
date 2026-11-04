import sys; sys.path.insert(0,'..')
import numpy as np, cv2, casc as C, ref4 as R
K=int(sys.argv[1]); name=sys.argv[2]; src=sys.argv[3]
g=cv2.imread(src,0); assert g.shape==(480,640)
casc=C.load('../haarcascade_frontalface_default.xml'); q=R.quant(casc,K)
ys,xs,_=R.first_k_fixed(g,q,step=1)
e=np.zeros((480,640),np.uint8); e[ys,xs]=255
g.tofile(name+'_in.raw'); e.tofile(name+'_exp.raw'); print(name,'survivors',len(ys),'of',(480-23)*(640-23))
