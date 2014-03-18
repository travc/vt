/* The MIT License

   Copyright (c) 2014 Adrian Tan <atks@umich.edu>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include "chmm.h"
#define MAXLEN 250
#define S  0
#define X  1
#define Y  2
#define LM 3
#define LI 4
#define LD 5
#define M  6
#define I  7
#define D  8
#define RM 9
#define RI 10
#define RD 11
#define W  12
#define Z  13
#define E  14

/**
 * Constructor.
 */
CHMM::CHMM()
{
    lt = new LogTool();
};

/**
 * Constructor.
 */
CHMM::CHMM(LogTool *lt)
{
    this->lt = lt;
};

/**
 * Destructor.
 */
CHMM::~CHMM()
{
    delete scoreX;
    delete scoreY;
    delete scoreLM;
    delete scoreLI;
    delete scoreLD;
    delete scoreM;
    delete scoreI;
    delete scoreD;
    delete scoreRM;
    delete scoreRI;
    delete scoreRD;
    delete scoreW;
    delete scoreZ;

    delete positionLM;
    delete positionLI;
    delete positionLD;
    delete positionM;
    delete positionI;
    delete positionD;
    delete positionRM;
    delete positionRI;
    delete positionRD;
    
    delete pathX;
    delete pathY;
    delete pathLM;
    delete pathLI;
    delete pathLD;
    delete pathM;
    delete pathI;
    delete pathD;
    delete pathRM;
    delete pathRI;
    delete pathRD;
    delete pathW;
    delete pathZ;
};

/**
 * Initializes object, helper function for constructor.
 */
void CHMM::initialize(const char* lflank, const char* ru, const char* rflank)
{
    delta = 0.001;
    epsilon = 0.5;
    tau = 0.1;
    eta = 0.001;

    logOneSixteenth = log10(1.0/16.0);

    transition[S][X] = 0;
    transition[X][X] = 0;

    transition[S][Y] = 0;
    transition[X][Y] = 0;
    transition[Y][Y] = 0;

    transition[S][LM] = log10((1-tau)/(eta*(1-eta)*(1-eta)));
    transition[X][LM] = transition[S][LM];
    transition[Y][LM] = transition[S][LM];
    transition[LM][LM] = log10(((1-2*delta-tau))/((1-eta)*(1-eta)));
    transition[LD][LM] = log10(((1-epsilon))/((1-eta)*(1-eta)));
    transition[LI][LM] = transition[LD][LM];

    transition[LM][LD] = log10((delta)/((1-eta)));
    transition[LD][LD] = log10((epsilon)/((1-eta)));
    
    transition[LM][LI] = transition[LM][LD];
    transition[LI][LI] = transition[LD][LD];

    transition[S][M] = log10((tau*(1-2*delta-tau))/(eta*eta*eta*(1-eta)*(1-eta)));
    transition[X][M] = transition[S][M];
    transition[Y][M] = transition[S][M];
    transition[LM][M] = log10((tau*(1-2*delta-tau))/(eta*eta*eta*(1-eta)*(1-eta)));
    transition[M][M] = log10((tau*(1-2*delta-tau))/(eta*eta*(1-eta)*(1-eta)));
    transition[D][M] = log10(((1-epsilon-tau))/((1-eta)*(1-eta)));
    transition[I][M] = transition[I][M];

    transition[S][D] = log10((tau*delta)/(eta*eta*(1-eta)));
    transition[X][D] = transition[S][D];
    transition[Y][D] = transition[S][D];
    transition[LM][D] = log10((tau*delta)/((1-eta)));
    transition[M][D] = log10(delta/(1-eta));

    transition[S][I] = log10((tau*delta)/(eta*eta*eta*(1-eta)));
    transition[X][I] = transition[S][I];
    transition[Y][I] = transition[S][I];
    transition[LM][I] = log10((tau*delta)/(eta*(1-eta)));
    transition[M][I] = log10(delta/(1-eta));

    transition[S][RM] = log10((tau*tau*(1-tau))/(eta*eta*eta*eta*eta*(1-eta)*(1-eta)));
    transition[X][RM] = transition[S][RM];
    transition[Y][RM] = transition[S][RM];
    transition[LM][RM] = log10((tau*tau*(1-tau))/(eta*eta*eta*eta*(1-eta)*(1-eta)));
    transition[M][RM] = log10((tau*(1-tau))/(eta*eta*eta*(1-eta)*(1-eta)));
    transition[D][RM] = transition[M][RM];
    transition[I][RM] = log10((tau*(1-tau))/(eta*eta*(1-eta)*(1-eta)));
    transition[RM][RM] = log10(((1-2*delta-tau))/((1-eta)*(1-eta)));
    transition[RD][RM] = log10(((1-epsilon))/((1-eta)*(1-eta)));
    transition[RI][RM] = transition[RD][RM];
    
    transition[RM][RD] = log10(delta/(1-eta));
    transition[RD][RD] = log10(epsilon/(1-eta));

    transition[RM][RI] = transition[RM][RD];
    transition[RI][RI] = transition[RM][RI];
    
    transition[S][W] = log10((tau*tau*tau)/(eta*eta*eta*eta*eta*eta)); 
    transition[X][W] = transition[S][W]; 
    transition[Y][W] = transition[S][W]; 
    transition[LM][W] = log10((tau*tau*tau)/(eta*eta*eta*eta*eta)); 
    transition[M][W] = log10((tau*tau)/(eta*eta*eta*eta)); 
    transition[D][W] = transition[M][W]; 
    transition[I][W] = log10((tau*tau)/(eta*eta*eta));  
    transition[RM][W] = log10(tau/eta);  
    transition[W][W] = 0;  

    transition[S][Z] = log10((tau*tau*tau)/(eta*eta*eta*eta*eta*eta)); 
    transition[X][Z] = transition[S][Z]; 
    transition[Y][Z] = transition[S][Z]; 
    transition[LM][Z] = log10((tau*tau*tau)/(eta*eta*eta*eta*eta)); 
    transition[M][Z] = log10((tau*tau)/(eta*eta*eta*eta)); 
    transition[D][Z] = transition[M][W]; 
    transition[I][Z] = log10((tau*tau)/(eta*eta*eta));  
    transition[RM][Z] = log10(tau/eta);  
    transition[W][Z] = 0;  
    transition[Z][Z] = 0;  

    //the best alignment score for subsequence (i,j)
    scoreX = new double[MAXLEN*MAXLEN];
    scoreY = new double[MAXLEN*MAXLEN];
    scoreLM = new double[MAXLEN*MAXLEN];
    scoreLI = new double[MAXLEN*MAXLEN];
    scoreLD = new double[MAXLEN*MAXLEN];
    scoreM = new double[MAXLEN*MAXLEN];
    scoreI = new double[MAXLEN*MAXLEN];
    scoreD = new double[MAXLEN*MAXLEN];
    scoreRM = new double[MAXLEN*MAXLEN];
    scoreRI = new double[MAXLEN*MAXLEN];
    scoreRD = new double[MAXLEN*MAXLEN];
    scoreW = new double[MAXLEN*MAXLEN];
    scoreZ = new double[MAXLEN*MAXLEN];

    //the matching position of the model sequence for the best alignment for subsequence (i,j)
    positionLM = new int32_t[MAXLEN*MAXLEN];
    positionLI = new int32_t[MAXLEN*MAXLEN];
    positionLD = new int32_t[MAXLEN*MAXLEN];
    positionM = new int32_t[MAXLEN*MAXLEN];
    positionI = new int32_t[MAXLEN*MAXLEN];
    positionD = new int32_t[MAXLEN*MAXLEN];
    positionRM = new int32_t[MAXLEN*MAXLEN];
    positionRI = new int32_t[MAXLEN*MAXLEN];
    positionRD = new int32_t[MAXLEN*MAXLEN];
    
    pathX = new char[MAXLEN*MAXLEN];
    pathY = new char[MAXLEN*MAXLEN];
    pathLM = new char[MAXLEN*MAXLEN];
    pathLI = new char[MAXLEN*MAXLEN];
    pathLD = new char[MAXLEN*MAXLEN];
    pathM = new char[MAXLEN*MAXLEN];
    pathI = new char[MAXLEN*MAXLEN];
    pathD = new char[MAXLEN*MAXLEN];
    pathRM = new char[MAXLEN*MAXLEN];
    pathRI = new char[MAXLEN*MAXLEN];
    pathRD = new char[MAXLEN*MAXLEN];
    pathW = new char[MAXLEN*MAXLEN];
    pathZ = new char[MAXLEN*MAXLEN];

    //assume alignments can't possibly be maxLength bases or more
    for (int32_t i=0; i<MAXLEN; ++i)
    {
        for (int32_t j=0; j<MAXLEN; ++j)
        {
            scoreX[i*MAXLEN+j] = -DBL_MAX;
            scoreY[i*MAXLEN+j] = -DBL_MAX;
            scoreM[i*MAXLEN+j] = -DBL_MAX;
            scoreI[i*MAXLEN+j] = -DBL_MAX;
            scoreD[i*MAXLEN+j] = -DBL_MAX;
            scoreW[i*MAXLEN+j] = -DBL_MAX;
            scoreZ[i*MAXLEN+j] = -DBL_MAX;

            if (j)
            {
                pathX[i*MAXLEN+j] = 'Y';
                pathY[i*MAXLEN+j] = 'Y';
                pathM[i*MAXLEN+j] = 'Y';
                pathI[i*MAXLEN+j] = 'Y';
                pathD[i*MAXLEN+j] = 'Y';
                pathW[i*MAXLEN+j] = 'Y';
                pathZ[i*MAXLEN+j] = 'Y';
            }
            else
            {
                pathX[i*MAXLEN+j] = 'X';
                pathY[i*MAXLEN+j] = 'X';
                pathM[i*MAXLEN+j] = 'X';
                pathI[i*MAXLEN+j] = 'X';
                pathD[i*MAXLEN+j] = 'X';
                pathW[i*MAXLEN+j] = 'X';
                pathZ[i*MAXLEN+j] = 'X';
            }
        }
    }

    logEta = log10(eta);
    logTau = log10(tau);

    scoreX[0*MAXLEN+0] = 0;
    scoreY[0*MAXLEN+0] = 0;
    scoreM[0*MAXLEN+0] = 0;
    scoreW[0*MAXLEN+0] = 0;
    scoreZ[0*MAXLEN+0] = 0;
    pathX[0*MAXLEN+0] = 'N';
    pathX[1*MAXLEN+0] = 'S';
    pathY[0*MAXLEN+0] = 'N';
    pathY[0*MAXLEN+1] = 'S';
    pathM[0*MAXLEN+0] = 'N';
    pathM[1*MAXLEN+1] = 'S';

    for (uint32_t k=1; k<MAXLEN; ++k)
    {
        scoreX[k*MAXLEN+0] = scoreX[(k-1)*MAXLEN+0] + transition[X][X];
        scoreX[0*MAXLEN+k] = -DBL_MAX;
        scoreY[k*MAXLEN+0] = -DBL_MAX;
        scoreY[0*MAXLEN+k] = scoreY[0*MAXLEN+(k-1)] + transition[Y][Y];
        scoreW[k*MAXLEN+0] = scoreW[(k-1)*MAXLEN+0] + transition[W][W];
        scoreW[0*MAXLEN+k] = -DBL_MAX;
        scoreZ[k*MAXLEN+0] = -DBL_MAX;
        scoreZ[0*MAXLEN+k] = scoreZ[0*MAXLEN+(k-1)] + transition[Z][Z];
    }

    scoreX[0*MAXLEN+0] = -DBL_MAX;
    scoreY[0*MAXLEN+0] = -DBL_MAX;
    scoreW[0*MAXLEN+0] = -DBL_MAX;
    scoreZ[0*MAXLEN+0] = -DBL_MAX;
};

/**
 * Align y against x.
 */
void CHMM::align(const char* y, const char* qual, bool debug)
{
    this->y = y;
    this->qual = qual;

    //adds a starting character at the fron of each string that must be matched
    xlen = strlen(x);
    ylen = strlen(y);

    if (xlen>MAXLEN||ylen>MAXLEN)
    {
        fprintf(stderr, "[%s:%d %s] Sequence to be aligned is greater than %d currently supported: %d\n", __FILE__, __LINE__, __FUNCTION__, MAXLEN, xlen>ylen?xlen:ylen);
        exit(1);
    }

    double max = 0;
    char maxPath = 'X';

    //alignment needs to take care of repeats.

    //construct possible solutions
    for (size_t i=1; i<=xlen; ++i)
    {
        for (size_t j=1; j<=ylen; ++j)
        {
            //X
            double xx = scoreX[(i-1)*MAXLEN+j] + transition[X][X];

            max = xx;
            maxPath = 'X';

            scoreX[i*MAXLEN+j] = max;
            pathX[i*MAXLEN+j] = maxPath;

            //Y
            double xy = scoreX[i*MAXLEN+(j-1)] + transition[X][Y];
            double yy = scoreY[i*MAXLEN+(j-1)] + transition[Y][Y];

            max = xy;
            maxPath = 'X';

            if (yy>max)
            {
                max = yy;
                maxPath = 'Y';
            }

            scoreY[i*MAXLEN+j] = max;
            pathY[i*MAXLEN+j] = maxPath;

            //M
            double xm = scoreX[(i-1)*MAXLEN+(j-1)] + transition[X][M];
            double ym = scoreY[(i-1)*MAXLEN+(j-1)] + transition[Y][M];
            double mm = scoreM[(i-1)*MAXLEN+(j-1)] + ((i==1&&j==1) ? transition[S][M] : transition[M][M]);
            double im = scoreI[(i-1)*MAXLEN+(j-1)] + transition[I][M];
            double dm = scoreD[(i-1)*MAXLEN+(j-1)] + transition[D][M];

            max = xm;
            maxPath = 'X';

            if (ym>max) //special case
            {
                max = ym;
                maxPath = 'Y';
            }
            if (mm>max)
            {
                max = mm;
                maxPath = (i==1&&j==1) ? 'S' : 'M';
            }
            if (im>max)
            {
                max = im;
                maxPath = 'I';
            }
            if (dm>max)
            {
                max = dm;
                maxPath = 'D';
            }

            scoreM[i*MAXLEN+j] = max + log10_emission_odds(x[i-1], y[j-1], lt->pl2prob((uint32_t) qual[j-1]-33));
            pathM[i*MAXLEN+j] = maxPath;

            //D
            double md = scoreM[(i-1)*MAXLEN+j] + transition[M][D];
            double dd = scoreD[(i-1)*MAXLEN+j] + transition[D][D];

            max = md;
            maxPath = 'M';

            if (dd>max)
            {
                max = dd;
                maxPath = 'D';
            }

            scoreD[i*MAXLEN+j] = max;
            pathD[i*MAXLEN+j] = maxPath;

            //I
            double mi = scoreM[i*MAXLEN+(j-1)] + transition[M][I];
            double ii = scoreI[i*MAXLEN+(j-1)] + transition[I][I];

            max = mi;
            maxPath = 'M';

            if (ii>max)
            {
                max = ii;
                maxPath = 'I';
            }

            scoreI[i*MAXLEN+j] = max;
            pathI[i*MAXLEN+j] = maxPath;

            //W
            double mw = scoreM[(i-1)*MAXLEN+j] + transition[M][W];
            double ww = scoreW[(i-1)*MAXLEN+j] + transition[W][W];

            max = mw;
            maxPath = 'M';

            if (ww>max)
            {
                max = ww;
                maxPath = 'W';
            }

            scoreW[i*MAXLEN+j] = max;
            pathW[i*MAXLEN+j] = maxPath;

            //Z
            double mz = scoreM[i*MAXLEN+(j-1)] + transition[M][Z];
            double wz = scoreW[i*MAXLEN+(j-1)] + transition[W][Z];
            double zz = scoreZ[i*MAXLEN+(j-1)] + transition[Z][Z];

            max = mz;
            maxPath = 'M';

            if (wz>max)
            {
                max = wz;
                maxPath = 'W';
            }
            if (zz>max)
            {
                max = zz;
                maxPath = 'Z';
            }

            scoreZ[i*MAXLEN+j] = max;
            pathZ[i*MAXLEN+j] = maxPath;
        }

        scoreM[xlen*MAXLEN+ylen] += logTau-logEta;
    }

    if (debug)
    {
        std::cerr << "\n=X=\n";
        print(scoreX, xlen+1, ylen+1);
        std::cerr << "\n=Y=\n";
        print(scoreY, xlen+1, ylen+1);
        std::cerr << "\n=M=\n";
        print(scoreM, xlen+1, ylen+1);
        std::cerr << "\n=D=\n";
        print(scoreD, xlen+1, ylen+1);
        std::cerr << "\n=I=\n";
        print(scoreI, xlen+1, ylen+1);
        std::cerr << "\n=W=\n";
        print(scoreW, xlen+1, ylen+1);
        std::cerr << "\n=Z=\n";
        print(scoreZ, xlen+1, ylen+1);
        std::cerr << "\n=Path X=\n";
        print(pathX, xlen+1, ylen+1);
        std::cerr << "\n=Path Y=\n";
        print(pathY, xlen+1, ylen+1);
        std::cerr << "\n=Path M=\n";
        print(pathM, xlen+1, ylen+1);
        std::cerr << "\n=Path D=\n";
        print(pathD, xlen+1, ylen+1);
        std::cerr << "\n=Path I=\n";
        print(pathI, xlen+1, ylen+1);
        std::cerr << "\n=Path W=\n";
        print(pathW, xlen+1, ylen+1);
        std::cerr << "\n=Path Z=\n";
        print(pathZ, xlen+1, ylen+1);
    }

    trace_path();
};

/**
 * Trace path after alignment.
 */
void CHMM::trace_path()
{
//    double globalMax = scoreM[xlen*MAXLEN+ylen];
//    char globalMaxPath = 'M';
//    if (scoreW[xlen*MAXLEN+ylen]>globalMax)
//    {
//        globalMax = scoreW[xlen*MAXLEN+ylen];
//        globalMaxPath = 'W';
//    }
//    if (scoreZ[xlen*MAXLEN+ylen]>globalMax)
//    {
//        globalMax = scoreZ[xlen*MAXLEN+ylen];
//        globalMaxPath = 'Z';
//    }
//
//    matchStartX = -1;
//    matchEndX = -1;
//    matchStartY = -1;
//    matchEndY = -1;
//    noBasesAligned = 0;
//    if (globalMaxPath == 'M')
//    {
//        matchEndX = xlen;
//        matchEndY = ylen;
//        ++noBasesAligned;
//    }
//
//    maxLogOdds = globalMax;
//
//    ss.str("");
//    ss << globalMaxPath;
//
//    //recursively trace path
//    trace_path(globalMaxPath, xlen, ylen);
//
//    path = reverse(ss.str());
//
//    bool inI = false;
//    bool inD = false;
//
//    //records appearance of Indels in probe and read
//    indelStartsInX.clear();
//    indelEndsInX.clear();
//    indelStartsInY.clear();
//    indelEndsInY.clear();
//    indelStartsInPath.clear();
//    indelEndsInPath.clear();
//    indelStatusInPath.clear();
//
//    matchedBases = 0;
//    mismatchedBases = 0;
//
//    uint32_t x_index=0, y_index=0;
//    for (uint32_t i=0; i<path.size(); ++i)
//    {
//        if (!inI && path[i]=='I')
//        {
//            inI = true;
//            indelStartsInPath.push_back(i);
//            indelStatusInPath.push_back('I');
//            indelStartsInX.push_back(x_index);
//            indelStartsInY.push_back(y_index);
//        }
//
//        if (inI && path[i]!='I')
//        {
//            inI = false;
//            indelEndsInPath.push_back(i);
//            indelEndsInX.push_back(x_index);
//            indelEndsInY.push_back(y_index);
//        }
//
//        if (!inD && path[i]=='D')
//        {
//            inD = true;
//            indelStartsInPath.push_back(i);
//            indelStatusInPath.push_back('D');
//            indelStartsInX.push_back(x_index);
//            indelStartsInY.push_back(y_index);
//        }
//
//        if (inD && path[i]!='D')
//        {
//            inD = false;
//            indelEndsInPath.push_back(i);
//            indelEndsInX.push_back(x_index);
//            indelEndsInY.push_back(y_index);
//        }
//
//        if (path[i]=='M')
//        {
//            if (x[x_index]== y[y_index])
//            {
//                ++matchedBases;
//            }
//            else
//            {
//                ++mismatchedBases;
//            }
//
//            ++x_index;
//            ++y_index;
//        }
//
//        if (path[i]=='I' || path[i]=='Y' || path[i]=='Z')
//        {
//            ++y_index;
//        }
//
//        if (path[i]=='D' || path[i]=='X' || path[i]=='W')
//        {
//            ++x_index;
//        }
//    }
//
//    left_align();
}

/**
 * Compute log10 emission odds based on equal error probability distribution.
 */
double CHMM::log10_emission_odds(char readBase, char probeBase, double e)
{
    //4 encodes for N
    if (readBase=='N' || probeBase=='N')
    {
        //silent match
        return 0;
    }

    if (readBase!=probeBase)
    {
        return log10(e/3)-logOneSixteenth;
    }
    else
    {
        return log10(1-e)-logOneSixteenth;
    }
};

/**
 * Prints an alignment.
 */
void CHMM::print_alignment()
{
    std::string pad = "\t";
    print_alignment(pad);
};

/**
 * Prints an alignment with padding.
 */
void CHMM::print_alignment(std::string& pad)
{
//    std::stringstream xAligned;
//    std::stringstream yAligned;
//    std::stringstream qualAligned;
//    uint32_t xIndex=0, yIndex=0;
//    for (uint32_t i=0; i<path.size(); ++i)
//    {
//        char state = path.at(i);
//        if (state=='S' || state=='E')
//        {
//            xAligned << state;
//            yAligned << state;
//            qualAligned << state;
//        }
//        else if (state=='X'||state=='W')
//        {
//            xAligned << x[xIndex++];
//            yAligned << '.';
//            qualAligned << '.';
//        }
//        else if (state=='M')
//        {
//            xAligned << x[xIndex++];
//            yAligned << y[yIndex];
//            qualAligned << qual[yIndex++];
//        }
//        else if (state=='D')
//        {
//            xAligned << x[xIndex++];
//            yAligned << '-';
//            qualAligned << '-';
//        }
//        else if (state=='I')
//        {
//            xAligned << '-';
//            yAligned << y[yIndex];
//            qualAligned << qual[yIndex++];
//        }
//        else if (state=='Y'||state=='Z')
//        {
//            xAligned << '.';
//            yAligned << y[yIndex];
//            qualAligned << qual[yIndex++];
//        }
//    }
//
//    std::cerr << pad << "X    " << xAligned.str() << "E\n";
//    std::cerr << pad << "Path " << path << "E\n";
//    std::cerr << pad << "Y    " << yAligned.str() << "E\n";
//    std::cerr << pad << "Qual " << qualAligned.str() << "E\n\n";
//
//    std::cerr << pad << "Alignment in Probe : [" << matchStartX << "," << matchEndX<< "]\n";
//    std::cerr << pad << "Alignment in Read  : [" << matchStartY << "," << matchEndY<< "]\n";
//    std::cerr << pad << "# Aligned Bases : " << noBasesAligned << "\n";
//    std::cerr << pad << "# Matched Bases : " << matchedBases << "\n";
//    std::cerr << pad << "# Mismatched Bases : " << mismatchedBases << "\n";
//    std::cerr << pad << "Max Log odds    : " << maxLogOdds << "\n";
};

/**
 * Prints a double matrix.
 */
void CHMM::print(double *v, uint32_t xlen, uint32_t ylen)
{
    for (uint32_t i=0; i<xlen; ++i)
    {
        for (uint32_t j=0; j<ylen; ++j)
        {
            std::cerr << (v[i*MAXLEN+j]==-DBL_MAX?-1000:v[i*MAXLEN+j]) << "\t";
        }

        std::cerr << "\n";
    }
};

/**
 * Prints a char matrix.
 */
void CHMM::print(char *v, uint32_t xlen, uint32_t ylen)
{
    for (uint32_t i=0; i<xlen; ++i)
    {
        for (uint32_t j=0; j<ylen; ++j)
        {
          std::cerr << v[i*MAXLEN+j] << "\t";
        }

        std::cerr << "\n";
    }
};

#undef MAXLEN
#undef S
#undef X
#undef Y
#undef LM
#undef LI
#undef LD
#undef M
#undef I
#undef D
#undef RM
#undef RI
#undef RD
#undef W
#undef Z
#undef E