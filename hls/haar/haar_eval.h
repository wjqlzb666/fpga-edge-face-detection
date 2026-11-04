// 自动生成(gen_eval.py),请勿手改: 前 4 级 Haar 级联的 stump 求值
#define G(Y,X) (((((Y)==0)&&top)||(((X)==0)&&left)) ? ap_uint<18>(0) : win[Y][X])
#define RS(x,y,w,h) ((ap_uint<18>)(G((y)+(h),(x)+(w)) - G((y),(x)+(w)) - G((y)+(h),(x)) + G((y),(x))))
#define NSTAGE 4
static bool eval_stages(const ap_uint<18> win[25][25], ap_uint<26> nfq, bool top, bool left)
{
    bool pass = true;
    { // ---- stage 0: 9 stumps, threshold -5164
        ap_int<18> acc = 0;
        { ap_int<24> raw = (ap_int<24>)RS(6,4,12,9)*(-1) + (ap_int<24>)RS(6,7,12,3)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -516; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 2138 : -2270; }
        { ap_int<24> raw = (ap_int<24>)RS(6,4,12,7)*(-1) + (ap_int<24>)RS(10,4,4,7)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 203; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1908 : 1359; }
        { ap_int<24> raw = (ap_int<24>)RS(3,9,18,9)*(-1) + (ap_int<24>)RS(3,12,18,3)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 359; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1547 : 1088; }
        { ap_int<24> raw = (ap_int<24>)RS(8,18,9,6)*(-1) + (ap_int<24>)RS(8,20,9,2)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 94; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -896 : 1204; }
        { ap_int<24> raw = (ap_int<24>)RS(3,5,4,19)*(-1) + (ap_int<24>)RS(5,5,2,19)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 246; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -798 : 1291; }
        { ap_int<24> raw = (ap_int<24>)RS(6,5,12,16)*(-1) + (ap_int<24>)RS(6,13,12,8)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 1628; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 571 : -1919; }
        { ap_int<24> raw = (ap_int<24>)RS(5,8,12,6)*(-1) + (ap_int<24>)RS(5,11,12,3)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 45; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1732 : 451; }
        { ap_int<24> raw = (ap_int<24>)RS(11,14,4,10)*(-1) + (ap_int<24>)RS(11,19,4,5)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -309; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1512 : 454; }
        { ap_int<24> raw = (ap_int<24>)RS(4,0,7,6)*(-1) + (ap_int<24>)RS(4,3,7,3)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 98; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -880 : 873; }
        if (acc < -5164) pass = false;
    }
    { // ---- stage 1: 16 stumps, threshold -5104
        ap_int<18> acc = 0;
        { ap_int<24> raw = (ap_int<24>)RS(6,6,12,6)*(-1) + (ap_int<24>)RS(6,8,12,2)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -346; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1273 : -1609; }
        { ap_int<24> raw = (ap_int<24>)RS(6,4,12,7)*(-1) + (ap_int<24>)RS(10,4,4,7)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 334; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1659 : 1210; }
        { ap_int<24> raw = (ap_int<24>)RS(1,8,19,12)*(-1) + (ap_int<24>)RS(1,12,19,4)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 349; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1988 : 718; }
        { ap_int<24> raw = (ap_int<24>)RS(0,2,24,3)*(-1) + (ap_int<24>)RS(8,2,8,3)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 1502; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -570 : 1770; }
        { ap_int<24> raw = (ap_int<24>)RS(9,9,6,15)*(-1) + (ap_int<24>)RS(9,14,6,5)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 595; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 274 : -2236; }
        { ap_int<24> raw = (ap_int<24>)RS(5,6,14,10)*(-1) + (ap_int<24>)RS(5,11,14,5)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -313; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -2737 : 468; }
        { ap_int<24> raw = (ap_int<24>)RS(5,0,14,9)*(-1) + (ap_int<24>)RS(5,3,14,3)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 135; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1111 : 548; }
        { ap_int<24> raw = (ap_int<24>)RS(13,11,9,6)*(-1) + (ap_int<24>)RS(16,11,3,6)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 301; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -360 : 956; }
        { ap_int<24> raw = (ap_int<24>)RS(7,5,6,10)*(-1) + (ap_int<24>)RS(9,5,2,10)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -116; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 950 : -679; }
        { ap_int<24> raw = (ap_int<24>)RS(10,8,6,10)*(-1) + (ap_int<24>)RS(12,8,2,10)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -162; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1186 : -305; }
        { ap_int<24> raw = (ap_int<24>)RS(2,5,4,9)*(-1) + (ap_int<24>)RS(4,5,2,9)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 259; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -430 : 1390; }
        { ap_int<24> raw = (ap_int<24>)RS(18,0,6,11)*(-1) + (ap_int<24>)RS(20,0,2,11)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -339; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1494 : -202; }
        { ap_int<24> raw = (ap_int<24>)RS(0,6,24,13)*(-1) + (ap_int<24>)RS(8,6,8,13)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -2255; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1146 : -542; }
        { ap_int<24> raw = (ap_int<24>)RS(9,6,6,9)*(-1) + (ap_int<24>)RS(11,6,2,9)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 235; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -360 : 1172; }
        { ap_int<24> raw = (ap_int<24>)RS(7,18,10,6)*(-1) + (ap_int<24>)RS(7,20,10,2)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 168; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -623 : 789; }
        { ap_int<24> raw = (ap_int<24>)RS(5,7,14,12)*(-1) + (ap_int<24>)RS(5,13,14,6)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 1499; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 397 : -1549; }
        if (acc < -5104) pass = false;
    }
    { // ---- stage 2: 27 stumps, threshold -4767
        ap_int<18> acc = 0;
        { ap_int<24> raw = (ap_int<24>)RS(0,3,24,3)*(-1) + (ap_int<24>)RS(8,3,8,3)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 1143; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1037 : 1504; }
        { ap_int<24> raw = (ap_int<24>)RS(5,8,15,6)*(-1) + (ap_int<24>)RS(5,11,15,3)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 516; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1686 : 1024; }
        { ap_int<24> raw = (ap_int<24>)RS(9,6,5,14)*(-1) + (ap_int<24>)RS(9,13,5,7)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 234; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 476 : -1634; }
        { ap_int<24> raw = (ap_int<24>)RS(9,5,6,10)*(-1) + (ap_int<24>)RS(11,5,2,10)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 237; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -671 : 850; }
        { ap_int<24> raw = (ap_int<24>)RS(6,6,3,12)*(-1) + (ap_int<24>)RS(6,12,3,6)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -50; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1432 : 436; }
        { ap_int<24> raw = (ap_int<24>)RS(3,21,18,3)*(-1) + (ap_int<24>)RS(9,21,6,3)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 536; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -519 : 1078; }
        { ap_int<24> raw = (ap_int<24>)RS(5,6,13,6)*(-1) + (ap_int<24>)RS(5,8,13,2)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -120; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 372 : -1379; }
        { ap_int<24> raw = (ap_int<24>)RS(18,1,6,15)*(-1) + (ap_int<24>)RS(18,1,3,15)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 826; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -312 : 1485; }
        { ap_int<24> raw = (ap_int<24>)RS(1,1,6,15)*(-1) + (ap_int<24>)RS(4,1,3,15)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 768; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -413 : 1244; }
        { ap_int<24> raw = (ap_int<24>)RS(0,8,24,15)*(-1) + (ap_int<24>)RS(8,8,8,15)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -1136; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1079 : -468; }
        { ap_int<24> raw = (ap_int<24>)RS(5,6,14,12)*(-1) + (ap_int<24>)RS(5,6,7,6)*(2) + (ap_int<24>)RS(12,12,7,6)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -803; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1664 : 157; }
        { ap_int<24> raw = (ap_int<24>)RS(2,12,21,12)*(-1) + (ap_int<24>)RS(2,16,21,4)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 1390; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 291 : -1604; }
        { ap_int<24> raw = (ap_int<24>)RS(8,1,4,10)*(-1) + (ap_int<24>)RS(10,1,2,10)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -28; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1039 : 239; }
        { ap_int<24> raw = (ap_int<24>)RS(2,13,20,10)*(-1) + (ap_int<24>)RS(2,13,10,10)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 1894; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -171 : 1311; }
        { ap_int<24> raw = (ap_int<24>)RS(0,1,6,13)*(-1) + (ap_int<24>)RS(2,1,2,13)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -840; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1553 : -310; }
        { ap_int<24> raw = (ap_int<24>)RS(20,2,4,13)*(-1) + (ap_int<24>)RS(20,2,2,13)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -700; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1806 : -53; }
        { ap_int<24> raw = (ap_int<24>)RS(0,5,22,19)*(-1) + (ap_int<24>)RS(11,5,11,19)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 6091; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -321 : 1573; }
        { ap_int<24> raw = (ap_int<24>)RS(18,4,6,9)*(-1) + (ap_int<24>)RS(20,4,2,9)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 318; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -103 : 959; }
        { ap_int<24> raw = (ap_int<24>)RS(0,3,6,11)*(-1) + (ap_int<24>)RS(2,3,2,11)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 286; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -413 : 986; }
        { ap_int<24> raw = (ap_int<24>)RS(12,1,4,9)*(-1) + (ap_int<24>)RS(12,1,2,9)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 649; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 174 : -3031; }
        { ap_int<24> raw = (ap_int<24>)RS(0,6,19,3)*(-1) + (ap_int<24>)RS(0,7,19,1)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -150; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 909 : -449; }
        { ap_int<24> raw = (ap_int<24>)RS(12,1,4,9)*(-1) + (ap_int<24>)RS(12,1,2,9)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 28; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -381 : 410; }
        { ap_int<24> raw = (ap_int<24>)RS(8,1,4,9)*(-1) + (ap_int<24>)RS(10,1,2,9)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 495; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 68 : -2710; }
        { ap_int<24> raw = (ap_int<24>)RS(5,5,14,14)*(-1) + (ap_int<24>)RS(12,5,7,7)*(2) + (ap_int<24>)RS(5,12,7,7)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -1291; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1791 : 292; }
        { ap_int<24> raw = (ap_int<24>)RS(1,10,18,2)*(-1) + (ap_int<24>)RS(1,11,18,1)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 35; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -962 : 238; }
        { ap_int<24> raw = (ap_int<24>)RS(17,13,4,11)*(-1) + (ap_int<24>)RS(17,13,2,11)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 444; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -54 : 1101; }
        { ap_int<24> raw = (ap_int<24>)RS(0,4,6,9)*(-1) + (ap_int<24>)RS(0,7,6,3)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -737; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1873 : 102; }
        if (acc < -4767) pass = false;
    }
    { // ---- stage 3: 32 stumps, threshold -4560
        ap_int<18> acc = 0;
        { ap_int<24> raw = (ap_int<24>)RS(6,4,12,9)*(-1) + (ap_int<24>)RS(6,7,12,3)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -1076; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1184 : -1097; }
        { ap_int<24> raw = (ap_int<24>)RS(6,5,12,6)*(-1) + (ap_int<24>)RS(10,5,4,6)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 260; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1601 : 787; }
        { ap_int<24> raw = (ap_int<24>)RS(0,1,24,5)*(-1) + (ap_int<24>)RS(8,1,8,5)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 2387; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -588 : 1414; }
        { ap_int<24> raw = (ap_int<24>)RS(4,10,18,6)*(-1) + (ap_int<24>)RS(4,12,18,2)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 101; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1492 : 528; }
        { ap_int<24> raw = (ap_int<24>)RS(2,17,12,6)*(-1) + (ap_int<24>)RS(2,17,6,3)*(2) + (ap_int<24>)RS(8,20,6,3)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 110; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -855 : 599; }
        { ap_int<24> raw = (ap_int<24>)RS(19,3,4,13)*(-1) + (ap_int<24>)RS(19,3,2,13)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 303; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -321 : 1198; }
        { ap_int<24> raw = (ap_int<24>)RS(1,3,4,13)*(-1) + (ap_int<24>)RS(3,3,2,13)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 327; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -445 : 977; }
        { ap_int<24> raw = (ap_int<24>)RS(0,1,24,23)*(-1) + (ap_int<24>)RS(8,1,8,23)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -4547; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1526 : -141; }
        { ap_int<24> raw = (ap_int<24>)RS(1,7,8,12)*(-1) + (ap_int<24>)RS(1,11,8,4)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 151; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -987 : 283; }
        { ap_int<24> raw = (ap_int<24>)RS(14,7,3,14)*(-1) + (ap_int<24>)RS(14,14,3,7)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -618; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -2505 : 242; }
        { ap_int<24> raw = (ap_int<24>)RS(3,12,16,6)*(-1) + (ap_int<24>)RS(3,12,8,3)*(2) + (ap_int<24>)RS(11,15,8,3)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 302; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 180 : -1375; }
        { ap_int<24> raw = (ap_int<24>)RS(6,6,12,6)*(-1) + (ap_int<24>)RS(6,8,12,2)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -182; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 499 : -920; }
        { ap_int<24> raw = (ap_int<24>)RS(8,7,6,12)*(-1) + (ap_int<24>)RS(8,13,6,6)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 556; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 183 : -1673; }
        { ap_int<24> raw = (ap_int<24>)RS(15,15,9,6)*(-1) + (ap_int<24>)RS(15,17,9,2)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -584; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -2008 : 185; }
        { ap_int<24> raw = (ap_int<24>)RS(1,17,18,3)*(-1) + (ap_int<24>)RS(1,18,18,1)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -187; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1014 : -390; }
        { ap_int<24> raw = (ap_int<24>)RS(4,4,16,12)*(-1) + (ap_int<24>)RS(4,10,16,6)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -1069; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -2641 : 253; }
        { ap_int<24> raw = (ap_int<24>)RS(0,1,4,20)*(-1) + (ap_int<24>)RS(2,1,2,20)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -693; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1476 : -302; }
        { ap_int<24> raw = (ap_int<24>)RS(3,0,18,2)*(-1) + (ap_int<24>)RS(3,1,18,1)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 31; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -508 : 647; }
        { ap_int<24> raw = (ap_int<24>)RS(1,5,20,14)*(-1) + (ap_int<24>)RS(1,5,10,7)*(2) + (ap_int<24>)RS(11,12,10,7)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -2117; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -2387 : 56; }
        { ap_int<24> raw = (ap_int<24>)RS(5,8,14,12)*(-1) + (ap_int<24>)RS(5,12,14,4)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 376; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -865 : 397; }
        { ap_int<24> raw = (ap_int<24>)RS(3,14,7,9)*(-1) + (ap_int<24>)RS(3,17,7,3)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -559; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -1478 : 101; }
        { ap_int<24> raw = (ap_int<24>)RS(14,15,9,6)*(-1) + (ap_int<24>)RS(14,17,9,2)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 430; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 187 : -1289; }
        { ap_int<24> raw = (ap_int<24>)RS(1,15,9,6)*(-1) + (ap_int<24>)RS(1,17,9,2)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 364; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 71 : -2439; }
        { ap_int<24> raw = (ap_int<24>)RS(11,6,8,10)*(-1) + (ap_int<24>)RS(15,6,4,5)*(2) + (ap_int<24>)RS(11,11,4,5)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -95; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 403 : -282; }
        { ap_int<24> raw = (ap_int<24>)RS(5,5,14,14)*(-1) + (ap_int<24>)RS(5,5,7,7)*(2) + (ap_int<24>)RS(12,12,7,7)*(2);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 715; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 152 : -1164; }
        { ap_int<24> raw = (ap_int<24>)RS(6,0,12,5)*(-1) + (ap_int<24>)RS(10,0,4,5)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 938; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 252 : -1299; }
        { ap_int<24> raw = (ap_int<24>)RS(9,0,6,9)*(-1) + (ap_int<24>)RS(9,3,6,3)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 38; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -773 : 278; }
        { ap_int<24> raw = (ap_int<24>)RS(9,6,6,9)*(-1) + (ap_int<24>)RS(11,6,2,9)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = 213; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -373 : 727; }
        { ap_int<24> raw = (ap_int<24>)RS(7,0,6,9)*(-1) + (ap_int<24>)RS(9,0,2,9)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -434; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -2378 : 37; }
        { ap_int<24> raw = (ap_int<24>)RS(10,6,6,9)*(-1) + (ap_int<24>)RS(12,6,2,9)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -96; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 432 : -49; }
        { ap_int<24> raw = (ap_int<24>)RS(8,6,6,9)*(-1) + (ap_int<24>)RS(10,6,2,9)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -271; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? 1137 : -357; }
        { ap_int<24> raw = (ap_int<24>)RS(3,8,18,4)*(-1) + (ap_int<24>)RS(9,8,6,4)*(3);
          ap_int<46> lhs = ((ap_int<46>)raw) << 22;
          ap_int<16> tq = -1117; ap_int<28> nq = nfq; ap_int<44> rhs = tq * nq;
          acc += (lhs < rhs) ? -3407 : 218; }
        if (acc < -4560) pass = false;
    }
    return pass;
}
#undef G
#undef RS
