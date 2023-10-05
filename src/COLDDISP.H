#include <math.h>
#include <stdlib.h>
#define max( x , y ) ( (x > y) ? x : y )
#define min( x , y ) ( (x < y) ? x : y )
#define sgn(x) ( (x < 0) ? -1 : 1)

class ColdWave
{
private:
	float wce, thdeg, kmin, kmax, wmin, wmax;
	float far *kpos1, *kpos2;
	int aion, zion, reso, nptswu,wll,kll,plot_w, plot_h;
	int nptsk; 
public:
	ColdWave(int width, int height);
	~ColdWave();   		
	void cwcalc(HWND);
	void getcoord(int x, int y, double& plotx, double& ploty);
	void Plot(HDC hdc);
	void SetSize(int w, int h);
    void GetBuffer(TTransferStruct Buffer);
};

void ColdWave::SetSize(int w, int h)
{
	plot_w = w; 	plot_h = h;    nptsk = w;
}

void ColdWave::GetBuffer(TTransferStruct Buffer)
{
	char *endptr;
	if (Buffer.wLIN) wll=LIN; else wll=LOG;
	if (Buffer.kLIN) kll=LIN; else kll=LOG;
	wce = strtod(Buffer.wce, &endptr);
	zion = strtod(Buffer.zion, &endptr);
	aion = strtod(Buffer.aion, &endptr);
	thdeg = strtod(Buffer.theta, &endptr);
	kmin = strtod(Buffer.kmin, &endptr);
	kmax = strtod(Buffer.kmax, &endptr);
	wmin = strtod(Buffer.wmin, &endptr);
	wmax = strtod(Buffer.wmax, &endptr);
	reso = min(20,strtod(Buffer.reso, &endptr));
	nptswu = nptsw*(float)((float)reso/(1.0*RESO_RANGE));
}    

ColdWave::ColdWave(int width, int height)
{
	wce = 1.614213562;
	aion = 1;
	zion = 1;
	thdeg = 7.0;
	reso = 1;
	nptswu = (nptsw*reso)/(1.0*RESO_RANGE);
	
	wmin = 0.0;
	wmax = 5.0;
	wll=LIN;
	kmin = 0.0;
	kmax = 5.0;
	kll=LIN;
	plot_w = width;
	plot_h = height;
	nptsk = width;
	kpos1 = new float[nptsw+1];
	kpos2 = new float[nptsw+1];
}

ColdWave::~ColdWave()
{
	delete kpos1;
    delete kpos2;
};

void ColdWave::cwcalc(HWND hwin)
{

    char s[50];

	float rwv, lwv, rlwv, swv, dwv, pwv, wci,wpi2;
	float th, cth2, sth2;
	float w, w2, delwci, delwce,delw,delk;
	float aqd, bqd, cqd;
	float mu21, mu22, arg;
	int i;

	wci = wce*zion/(1837.0*aion);
	wpi2 = wci/wce;
        
	if (wll==LIN) delw = wmax-wmin; 
	else { wmin=max(1.0e-6,wmin); delw = log(wmax) - log(wmin); }
        
	if (kll==LIN) delk = kmax-kmin;
	else { kmin=max(1.0e-6,kmin); delk = log(kmax) - log(kmin); }

    th = thdeg*0.01745329252;   
	cth2 = cos(th)*cos(th);
	sth2 = sin(th)*sin(th);

   
	for (i=1;i<=nptswu;i++)
	{   
		if (wll==LIN) w = wmin + (i-0.5)*delw/nptswu;
		else w = wmin * exp((i-0.5)*delw/nptswu);
		w2 = w*w;

        if (fabs(w-wci)<(0.001)*wci) delwci = (1.0e-3*wci)*sgn(w-wci);
        else  delwci = w-wci;
      
		if (fabs(w-wce)<(0.001)*wce) delwce = (1.0e-3*wce)*sgn(w-wce);
        else delwce = w-wce;

		rwv = 1.0 - wpi2/(w*(w+wci)) - 1.0/(w*delwce);
        lwv = 1.0 - wpi2/(w*delwci) - 1.0/(w*(w+wce));
        rlwv = rwv*lwv;
        swv = 0.5*(rwv+lwv);
        dwv = 0.5*(rwv-lwv);
        pwv = 1.0 - wpi2/w2 - 1.0/w2;
		aqd = swv*sth2 + pwv*cth2;
        bqd = -rlwv*sth2 - swv*pwv*(1.0 + cth2);
        cqd = rlwv*pwv;    
		arg = max(0.0, bqd*bqd - 4.0*aqd*cqd);

		mu21 = (- bqd + sqrt(arg))/(2.0*aqd);


        if (mu21>=0.0) {
			if (kll==LIN) kpos1[i] = 0.5 + (w*sqrt(mu21)-kmin)*nptsk/delk;
			else kpos1[i] = 0.5 + (log(w*sqrt(mu21))-log(kmin))*nptsk/delk;
			 }
		else kpos1[i]=0;

		mu22 = (- bqd - sqrt(arg))/(2.0*aqd);
		if (mu22 >= 0.0) {
			if (kll==LIN) kpos2[i] = 0.5 +(w*sqrt(mu22)-kmin)*nptsk/delk;
			else kpos2[i] = 0.5 + (log(w*sqrt(mu22))-log(kmin))*nptsk/delk;
			}
		else kpos2[i]=0;
	 }
	return;
}

void ColdWave::getcoord(int x, int y, double& plotx, double& ploty)
{
	if (wll==LIN) ploty = wmax - 1.0*(wmax-wmin)*y/plot_h;
    else ploty = wmax * pow(1.0*wmin/wmax,1.0*y/plot_h);

    if (kll==LIN) plotx = kmin + 1.0*(kmax-kmin)*x/nptsk;
	else plotx = kmin * pow(1.0*kmax/kmin,1.0*x/nptsk);
}

void ColdWave::Plot(HDC hdc)
{
	int wplot, kplot, ixt1, ixt2, idif1, idif2, i;

	wplot=0;
    kplot=0;

    MoveTo(hdc,0,0);
    for (i=1;i<=nptswu;i++)
    {
    	wplot = (plot_h*1.0/nptswu)*(nptswu - i);
		kplot = max(0,min(nptsk,kpos1[i]));
		if ( (kplot > 0) && (i!=1) )
		{
			if (i==2)
        	{
            	ixt1 = kpos1[1];
				ixt2 = kpos2[1];
        	}
			else
        	{
        		ixt1 = 2*kpos1[i-1]-kpos1[i-2];
				ixt2 = 2*kpos2[i-1]-kpos2[i-2];
        	}
        	idif1 = fabs(kpos1[i]-ixt1);
        	idif2 = fabs(kpos1[i]-ixt2);
			if (idif1<=idif2) LineTo(hdc,kplot,wplot);
			else MoveTo(hdc,kplot,wplot);
    	}
    	else MoveTo(hdc,kplot,wplot);
	}

	MoveTo(hdc,0,0);
    for (i=1;i<=nptswu;i++)
    {
    	wplot = (plot_h*1.0/nptswu)*(nptswu - i);
		kplot = max(0,min(nptsk,kpos2[i]));
		if ( (kplot > 0) && !(i==1) )
		{
			if (i==2)
        	{
            	ixt1 = kpos1[1];
				ixt2 = kpos2[1];
        	}
			else
        	{
        		ixt1 = 2*kpos1[i-1]-kpos1[i-2];
				ixt2 = 2*kpos2[i-1]-kpos2[i-2];
        	}
        	idif1 = fabs(kpos2[i]-ixt1);
        	idif2 = fabs(kpos2[i]-ixt2);
			if (idif2<=idif1) LineTo(hdc,kplot,wplot);
			else MoveTo(hdc,kplot,wplot);
    	}
		else MoveTo(hdc,kplot,wplot);        
	}
	LineTo(hdc,nptsk,wplot);

	return;
}                
