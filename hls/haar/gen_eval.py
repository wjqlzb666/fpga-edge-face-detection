import sys; sys.path.insert(0,'..')
import casc as C, ref4 as R
K=int(sys.argv[1]) if len(sys.argv)>1 else 4
casc=C.load('../haarcascade_frontalface_default.xml'); q=R.quant(casc,K)
o=[]
o.append('// 自动生成(gen_eval.py),请勿手改: 前 %d 级 Haar 级联的 stump 求值\n'%K)
o.append('#define G(Y,X) (((((Y)==0)&&top)||(((X)==0)&&left)) ? ap_uint<18>(0) : win[Y][X])\n')
o.append('#define RS(x,y,w,h) ((ap_uint<18>)(G((y)+(h),(x)+(w)) - G((y),(x)+(w)) - G((y)+(h),(x)) + G((y),(x))))\n')
o.append('#define NSTAGE %d\n'%K)
o.append('static bool eval_stages(const ap_uint<18> win[25][25], ap_uint<26> nfq, bool top, bool left)\n{\n')
o.append('    bool pass = true;\n')
for si,(th,ws) in enumerate(q):
    o.append('    { // ---- stage %d: %d stumps, threshold %d\n        ap_int<18> acc = 0;\n'%(si,len(ws),th))
    for (rects,t,l0,l1) in ws:
        ex=' + '.join('(ap_int<24>)RS(%d,%d,%d,%d)*(%d)'%(x,y,w,h,wt) for (x,y,w,h,wt) in rects)
        o.append('        { ap_int<24> raw = %s;\n'%ex)
        o.append('          ap_int<46> lhs = ((ap_int<46>)raw) << 22;\n')
        o.append('          ap_int<16> tq = %d; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;\n'%t)
        o.append('          acc += (lhs < rhs) ? %d : %d; }\n'%(l0,l1))
    o.append('        if (acc < %d) pass = false;\n    }\n'%th)
o.append('    return pass;\n}\n#undef G\n#undef RS\n')
open('haar_eval.h','w').write(''.join(o))
print('stumps',sum(len(w) for _,w in q))
