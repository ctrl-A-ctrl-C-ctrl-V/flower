#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define MAX_BLOCK 25
#define MAX_CON 50
#define XDIM 80
#define YDIM 80
#define TRN 5

/*--------------------------------------ALGO---------------------------------------
        mod1 : reads in the blocks
        mod2 : derives the connections
        mod3 : builds up the connectivity matrix
        mod4 : finds out the placement position
        mod5 : forms the grid for routing space
        mod6 : raster scanning starts
        -- NOT USED mod7 : copying the temporary pins to the layout
        mod7 : compacts the layout
        mod8 : display the layout
---------------------------------------------------------------------------------*/
struct  block{
        int length;
        int width;
        int noconn;             /* No. of connections  */
        int conn[MAX_CON];      /* Which are the connections */
        float cab;              /* Abscissa of the centre */
        float cor;              /* Ordinate of the centre */
        float alen;             /* Alloted length */
        float awid;             /* Alloted width */
};

struct block    blockarray[MAX_BLOCK]; /* Array of blocks */

struct  con{
        int nblocks; /* No of blocks */
        int bl[MAX_BLOCK];      /* Which are the blocks */
};

struct con conarray[MAX_CON];

int kanekted[MAX_CON];
int subass[MAX_CON];
float dcm[MAX_CON][MAX_CON]; /* The DisConnectivity Matrix */
int inarb[MAX_BLOCK]; /* The initial array contains all block numbers */
int inarc[MAX_CON];   /* The initial array contains all connection numbers */
int grid[XDIM][YDIM][2];    /* For 1st layer of metallisation */
int dst[4];

int  area();
void readinput();
void partition();
void initpl();
void formgrid();
void rscan();
void backtrack();
void un_gendu();
void prop();
void order();
void tail();
void washout();
void compress();
void display();
void squeezex();
void squeezey();

int cut = 1;

/* The cut direction 1 -> vertical, -1 -> horizontal */
int main()
{
        int i,j,k,m,ii;
        int nc=1;       /* The actual no. of connection    */
        int nb;         /* The actual no. of blocks        */
        int totlar=0;   /* The minimum total area required */
        float l0,w;     /* sqrt(totlar)= l0                */
        int xmaxm = XDIM, ymaxm = YDIM;
        /*------------------------------mod1---------------------------------------*/
        readinput(&nb,&nc);
        /*------------------------------mod1---------------------------------------*/
        /*------------------------------mod2---------------------------------------*/
        printf(" You have a total of %d connections\n",
        nc);
        for (i=0;i<nc;i++)
        {
                subass[i] = 0;
                kanekted[i] = 0;
                conarray[i].nblocks=0;
                for (j=0;j<nb;j++)
                        for (k=0; k < blockarray[j].noconn;
                        k++)
                                if (blockarray[j].conn[k]==(i+1))
                                        conarray[i].bl[conarray[i].nblocks++]=
                                        j;
        }
        /*------------------------------mod2---------------------------------------*/
        /*------------------------------mod3---------------------------------------*/
        for (i=0; i<nb;i++)
                totlar += (blockarray[i].length)*(blockarray[i].width);

        if (totlar >= (XDIM*YDIM)/2)     /* 100% routing area */
        {
                printf("You should segmentize the chip--area insufficient\n");
                exit(2);
        }

        l0=(float)(sqrt((double)(totlar)));

        for ( i= 0; i<nb; i++)
                for ( j=0; j<nb; j++)  dcm[i][j] = -2;

        for ( i= 0; i<nb; i++)
                for ( j=0; j<nb; j++)
                {
                        w=0;
                        for (k=0;k<blockarray[i].noconn;k++)
                                for (m=0; m<blockarray[j].noconn;m++)
                                        if ((blockarray[i].conn[k]==blockarray[j].conn[m]) && (i!=j) )
                                        {
        /* code folded from here */
                                                ii=blockarray[i].conn[k]-1;
                                                w += 1.0/((float)( conarray[ii].nblocks -1));
        /* unfolding */
                                        }
                        if (w!=0) dcm[i][j] = w;
                }
        /*------------------------------mod3---------------------------------------*/
        /*------------------------------mod4---------------------------------------*/
        for (i=0; i<nb;i++)
                inarb[i]=i;
        for (i=0;i<nc;i++)
                inarc[i]=i;
        initpl(nb,inarb,cut,l0,l0,l0/2,l0/2);
        /*------------------------------mod4---------------------------------------*/
        /*------------------------------mod5---------------------------------------*/
        formgrid(nb,l0);
        /*------------------------------mod5---------------------------------------*/
        /*------------------------------mod6---------------------------------------*/
        rscan(nc,inarc);
        /*------------------------------mod6---------------------------------------*/
        /*------------------------------mod7---------------------------------------*/
        /*compress(&xmaxm,&ymaxm);*/
        /*------------------------------mod7---------------------------------------*/
        /*------------------------------mod8---------------------------------------*/
        display(xmaxm,ymaxm);
        /*------------------------------mod8---------------------------------------*/
}

void readinput(nbl,ncl)
int *nbl, *ncl;
{
        int i,j;

        printf(" How many blocks do you have ?");
        scanf_s("%d",nbl);
        if (*nbl < 2)
        {
                printf(" Insufficient number of blocks -- Sorry !\n");
                exit(2);
        }
        for (i=0;i<*nbl;i++)
        {
                printf(" The length of the %dth block :",
                i+1);
                scanf_s("%d", &(blockarray[i].length));
                printf(" The width of the %dth block :",
                i+1);
                scanf_s("%d", &blockarray[i].width);
                printf(" How many connections are there with this block ?");
                scanf_s("%d", &blockarray[i].noconn);
                printf(" Input the connections :\n");
                for (j=0; j < blockarray[i].noconn; j++)
                {
                        printf("\t%dth connection :",j+
                        1);
                        scanf_s("%d",&blockarray[i].conn[j]);
                        if (blockarray[i].conn[j]>*ncl) *
                                ncl = blockarray[i].conn[j];
                }
        }
}

void initpl(n,s,mark,ln,wd,cox,coy)

int n;      /* no. of elements */
int mark;   /* cut direction */
int *s ;    /* The list of blocks */
float ln,wd,cox,coy;

{
        int s1[MAX_BLOCK], s3[MAX_BLOCK], n1, n3;
        float r;

        if (n==1)
        {
                blockarray[s[0]].cab=cox;
                blockarray[s[0]].cor=coy;
                if (ln > wd)
                        blockarray[s[0]].alen=ln, blockarray[s[0]].awid=
                        wd;
                else
                        blockarray[s[0]].alen=wd, blockarray[s[0]].awid=
                        ln;
        }
        else
        {
                partition(n,s,s1,s3,&n1,&n3);
                r = ( ((float)(area(n1,s1))) / ((float)(area(n1,
                s1) + area(n3,s3))));
                if (mark == 1)
                {
                        initpl(n1,s1,(mark*(-1)),(r*ln),
                        wd,(cox-ln*(1-r)/2),coy);
                        initpl(n3,s3,(mark*(-1)),((1-r)*
                        ln),wd,(cox+(r*ln)/2),coy);
                }
                else
                {
                        initpl(n1,s1,(mark*(-1)),ln,(r*
                        wd),cox,(coy+wd*(1-r)/2));
                        initpl(n3,s3,(mark*(-1)),ln,((1-
                        r)*wd),cox,(coy-(r*wd)/2));
                }
        }
}

int area(p,q)
int p, *q;
{
        int co,a=0;

        if (p == 0) return 0;
        else
        {
                for(co=0;co<p;co++)
                        a += (blockarray[q[co]].length)*
                        (blockarray[q[co]].width);
                return(a);
        }
}

void partition(p,q,z1,z3,c1,c3)
int p, *q, *z1, *z3, *c1, *c3;
{
        int flag1,flag2,klag1,klag2,u,v,x,y,c2,c4,cow,i,
            z2[MAX_BLOCK],z4[MAX_BLOCK];
        float max = 0.0, min1, min2, area1, area2, area3,
              area4;

        if (p==2) u=q[0],v=q[1];
        else
        {
                for (x=0;x<p;x++)
                        for (y=(x+1);y<p;y++)
                                if ((max < dcm[q[x]][q[y]]) &&
                                    (dcm[q[x]][q[y]] > 0))
                                {
                                        u=q[x],v=q[y];
                                        max = dcm[u][v];
                                }
        }
        *c1 = 0, *c3 = 0, c2 = 0, c4 = 0;
        z1[(*c1)++]=u, z3[(*c3)++]=v;

        for (x=0;x<p;x++)
        {
                flag1=1,flag2=1,klag1=1,klag2=1;
                for (y=0;(y<(*c1)) && (flag1==1);y++)
                        if ( z1[y] == q[x]) flag1 = 0;

                for (y=0;(y<c2) && (flag2==1);y++)
                        if ( z2[y] == q[x]) flag2 = 0;

                for (y=0;(y<(*c3)) && (klag1==1);y++)
                        if ( z3[y] == q[x]) klag1 = 0;

                for (y=0;(y<c4) && (klag2==1);y++)
                        if ( z4[y] == q[x]) klag2 = 0;


                if (flag1*flag2*klag1*klag2 == 1)
                {
                        min1 = 20 , min2 = 20;
                        for (y=0;y<(*c1);y++)
                                if (min1 > dcm[(z1[y])][(q[x])]) min1
                                = dcm[(z1[y])][(q[x])];
                        for (y=0;y<(*c3);y++)
                                if (min2 > dcm[(z3[y])][(q[x])]) min2
                                = dcm[(z3[y])][(q[x])];

                        if (min1 < min2)
                        {
                                if (min2 == -2) z1[(*c1)++]
                                = q[x];
                                else z2[c2++] = q[x];
                        }
                        else
                        {
                                if (min1 == -2) z3[(*c3)++]
                                = q[x];
                                else z4[c4++] = q[x];
                        }

                }
        }

        area1=(float)area(*c1,z1);
        area2=(float)area(c2,z2);
        area3=(float)area(*c3,z3);
        area4=(float)area(c4,z4);

        cow=0;
        while(((( (area1+area2) / (area3+area4) )>1.3)
        ||( ( (area1+area2) / (area3+area4) )< 0.7)) && (cow<
        5))
        {
                if(area1 + area2 - area3 - area4 >0)
                {
                        if (c2>0)
                        {
                                z4[c4] = z2[c2-1];
                                area2 -=(float)((blockarray[z2[c2-1]].length)*
                                (blockarray[z2[c2-1]].width));
                                area4 +=(float)((blockarray[z2[c2-1]].length)*
                                (blockarray[z2[c2-1]].width));
                                c2--,c4++;
                        }
                }
                else
                {
                        if (c4>0)
                        {
                                z2[c2] = z4[c4-1];
                                area4 -=(float)((blockarray[z4[c4-1]].length)*
                                (blockarray[z4[c4-1]].width));
                                area2 +=(float)((blockarray[z4[c4-1]].length)*
                                (blockarray[z4[c4-1]].width));
                                c4--,c2++;
                        }
                }
                cow++;
        }

        for (i=0;i<c2;i++)
                z1[(*c1)+i] = z2[i];
        for (i=0;i<c4;i++)
                z3[(*c3)+i] = z4[i];
        (*c1) += c2;
        (*c3) += c4;
}

void formgrid(nbl,l1)
int nbl;
float l1;
{
        int i,j,k,l,m,str,orgx,orgy,c0,c1,of1,of2,ox;

        str = (XDIM-15)/((int)l1);
        for(i=0;i< XDIM; i++)
                for(j=0;j< YDIM; j++) grid[i][j][0] = 0,
                grid[i][j][1] = 0;

        for(i=0;i< nbl; i++)
        {
                l = (blockarray[i].length);
                m = (blockarray[i].width);
                orgx = (int)(blockarray[i].cab)*str+TRN-
                (int)(l/2);
                orgy = (int)(blockarray[i].cor)*str+TRN-
                (int)(m/2);

                for(j=0;j< l; j++)
                        for(k=0;k< m; k++)
                                grid[orgx+j][orgy+k][1]
                                = -1;

                l--,m--;
                c0=0;
                while (c0 < blockarray[i].noconn)
                {
                        c1=2*c0;

                        if ((c1 >= 0) && (c1 <= l)) of1
                        = orgx +c1, of2 = orgy;
                        if ((c1 > l) && (c1 <= (l+m))) of1
                        = orgx + l, of2 = orgy +c1 -l;
                        if ((c1 > (l+m)) && (c1 <= (2*l+
                        m))) of1 = orgx + 2*l +m -c1, of2
                        = orgy+m;
                        if ((c1 > (2*l +m)) && (c0 < (l+
                        m))) of1 = orgx, of2 = orgy -c1+
                        2*m+2*l;

                        ox= (blockarray[i].conn[c0])-1;
                        grid[of1][of2][0] = 100*ox+(++subass[ox]);
                        grid[of1][of2][1] = -2;

                        c0++;
                }
        }

        for (i=0; i< XDIM;i++)
        {
                grid[i][0][1] = -1;
                grid[i][YDIM-1][1] = -1;
        }
        for (i=0; i< YDIM; i++)
        {
                grid[0][i][1] = -1;
                grid[XDIM-1][i][1] = -1;
        }
}

void rscan(ncl,s)
int ncl;
int *s;
{
        int cnm,i,j,k,betty = 1,ronnie = 0;

        for (j=0;j< XDIM;j++)
                for (k=0;k< YDIM;k++)
                        if (grid[j][k][1] == (-2)) prop(j,
                        k,&betty);

        for (i=0;i<ncl;i++)
        {
                cnm = 0;
                while (cnm < 1)
                {
                        betty = 1;
                        switch (ronnie) {
                        case 0 :
                                for (j=0;j< YDIM;j++)
                                        for (k=0;k< XDIM;k++)
        /* code folded from here */
        if ( (grid[j][k][1] >0) && (grid[j][k][0]/100 == s[i])) prop(j,k, &betty);
        /* unfolding */
                                break;
                        case 1 :
                                for (j=YDIM-1;j >= 0;j--)
                                        for (k=XDIM-1;k>= 0;k--)
        /* code folded from here */
        if ( (grid[j][k][1] >0) && (grid[j][k][0]/100 == s[i])) prop(j,k, &betty);
        /* unfolding */
                                break;
                        }
                        ronnie++, ronnie %=2;

                        if (betty ==1) un_gendu(s[i],0);
                        if ((kanekted[s[i]] == (conarray[s[i]].nblocks-1)) || (betty ==1)) 
                           cnm =1;
                }
        }
        printf(" Accounting :\n First Layer of Metallization :\n");
        for (i=0;i<ncl;i++)
                if (kanekted[s[i]] != (conarray[s[i]].nblocks-
                1))
                        printf(" %d out of %d connection-segment achieved for conn. %d\n",
                        kanekted[s[i]],(conarray[s[i]].nblocks-
                        1),s[i]);
        tail();
}

void un_gendu(tt,yes)
int tt,yes;
{
        int i,j;

        for( i=0;i< XDIM; i++)
                for (j=0; j< YDIM; j++)
                {
                        switch (yes){
                        case 0 :
                                if ((grid[i][j][1] > 0) && (grid[i][j][0]/100 == tt))
                                   grid[i][j][1] = 0; 
                                break;
                        case 1 :
                                if ((grid[i][j][1] > 0) && (grid[i][j][0] == tt))
                                        grid[i][j][1] = 0;
                                break;
                        }
                        if (grid[i][j][1] == -3) 
                           grid[i][j][1] = -2;
                }
}

void prop(p,q,arch)
int p, q, *arch;
{
        int k, u, v, tempo;

        for (k=0;k<4;k++)
        {
                u = (int)cos(M_PI_2*k);
                v = (int)sin(M_PI_2*k);
                if (grid[p+u][q+v][1] == 0)
                {
                        if (grid[p][q][1] == -2) grid[p+u][q+v][1]
                        = 1;
                        else if (grid[p][q][1] > 0) grid[p+u][q+v][1]
                        = grid[p][q][1] + 1;
                        grid[p+u][q+v][0] = grid[p][q][0];
                        (*arch) = (-1);
                }
                else
                        if ((grid[p+u][q+v][1] > 0) &&
                        (grid[p+u][q+v][0]/100 == grid[p][q][0]/
                        100) && (grid[p+u][q+v][0] != grid[p][q][0]))
                        {
                                k = 4;
                                (*arch) = (-1);
                                (kanekted[((grid[p][q][0])/100)])++; 
                                dst[0] = 0;
                                dst[1] = 1;
                                dst[2] = 2;
                                dst[3] = 3;
                                tempo = grid[p+u][q+v][0];
                                backtrack(p,q,grid[p][q][0]);
                                backtrack(p+u,q+v,grid[p][q][0]);
                                un_gendu(grid[p][q][0],
                                1);
                                un_gendu(tempo,1);
                        }
        }
}

void backtrack(a,b,s)
int a,b,s;
{
        int flag = 1,klag,m,n,p;

        while( flag==1)
        {
                klag = 0;
                for( m=0; (m<4) && (klag <1); m++)
                {
                        n = (int)cos(M_PI_2*dst[m]);
                        p = (int)sin(M_PI_2*dst[m]);
                        if ( grid[a+n][b+p][0] == grid[a][b][0])
                        {
                                if (grid[a+n][b+p][1] == -2)
                                {
                                        flag = 0;
                                        klag = 1;
                                        grid[a][b][1] =
                                        -3;
                                        grid[a][b][0] =
                                        s;
                                }
                                else if ( grid[a+n][b+p][1]
                                == grid[a][b][1]-1)
                                {
                                        grid[a][b][1] =
                                        -3;
                                        grid[a][b][0] =
                                        s;
                                        klag = 1;
                                        a += n;
                                        b += p;
                                        order(m,dst);
                                }
                        }
                }
        }
}

void order(f,ar)
int f, *ar;
{
        int i,j,temp;

        for (i=0;i<(4-f);i++)
        {
                temp = *(ar+3);
                for ( j=2;j>=0;j--)
                        *(ar+j+1) = *(ar+j);
                *ar = temp;
        }
}

void tail()
{
        int flag,i,j,k,px,py,a;

        for (i=0;i<XDIM;i++)
        {
                flag = 0;
                for (j=2;j<YDIM;j++)
                {
                        if ((grid[i][j][1] == 0) && (grid[i][j-1][1]
                        == -2) && (grid[i][j-2][1] == -
                        2) && (grid[i][j-1][0]/100 == grid[i][j-2][0]/
                        100))
                        {
                                flag = 1;
                                px = i;
                                py = j-1;
                                a = grid[i][j-1][0];
                        }
                        else if (( flag == 1) && (grid[i][j][1] == -2) && (grid[i][j][0] == a))
                                washout(px,py,j);
                        else if ((grid[i][j][1] == -1) || ((grid[i][j][1] == -2) && (grid[i][j][0] !=a))) 
                                flag = 0;
                }

                flag = 0;
                for (j=YDIM-3; j>=0;j--)
                {
                        if ((grid[i][j][1] == 0) && 
                            (grid[i][j+1][1] == -2) && 
                            (grid[i][j+2][1] == -2) && 
                            (grid[i][j+1][0]/100 == grid[i][j+2][0]/100)) {
                                flag = 1;
                                px = i;
                                py = j+1;
                                a = grid[i][j+1][0];
                        }
                        else if (( flag ==1) && (grid[i][j][1] == -2) && (grid[i][j][0] == a))
                                washout(px,py,j);
                        else if ((grid[i][j][1] == -1) || ((grid[i][j][1] == -2) && (grid[i][j][0]!=a))) flag = 0;
                }
        }
}

void washout(x,y1,y2)
int x,y1,y2;
{
        int a,b,dir,xtemp,i,j,k,ytemp,u,v,klag;

        if ((grid[x-1][y1][1] == -2) && 
            (grid[x][y1][0] == grid[x-1][y1][0]))
        {
                dir = -1;
                grid[x-1][y1][1] = 0;
                grid[x-1][y2][1] = 0;
                grid[x-1][y1][0] = 0;
                grid[x-1][y2][0] = 0;
        }
        else if ((grid[x+1][y1][1] == -2) && (grid[x][y1][0] == grid[x+1][y1][0]))
        {
                dir= 1;
                grid[x+1][y1][1] = 0;
                grid[x+1][y2][1] = 0;
                grid[x+1][y1][0] = 0;
                grid[x+1][y2][0] = 0;
        }

        xtemp = x + 2*dir;
        ytemp = y1;

        while ((xtemp != (x+2*dir)) || (ytemp != y2))
        {
                klag = 0;
                for ( i=0; (i<4) && (klag<1); i++)
                {
                        v = (int)sin(M_PI_2*i);
                        u = (int)cos(M_PI_2*i);
                        if ((grid[xtemp+u][ytemp+v][1] == -2) && 
                            (grid[xtemp+u][ytemp+v][0] == grid[xtemp][ytemp][0]))
                        {
                                grid[xtemp][ytemp][1] = 0;
                                grid[xtemp][ytemp][0] = 0;
                                xtemp += u;
                                ytemp += v;
                                klag = 1;
                        }
                }
        }

        grid[xtemp][ytemp][1] = 0;
        grid[xtemp][ytemp][0] = 0;

        if ( y1 < y2)
        {
                a = y1+1;
                b=y2-1;
        }
        else
        {
                a=y2+1;
                b=y1-1;
        }
        for (j = a;j<=b;j++)
        {
                grid[x][j][1] = -2;
                grid[x][j][0] = grid[x][y1][0];
        }
}

void compress(xmax,ymax)
int *xmax, *ymax;
{
        int i,j,klag;

        for(i=1;i< (*xmax)-1;i++)
        {
                klag = 0;
                for(j=1;j<YDIM-1;j++)
                        if (((grid[i][j][1] == -2) && (grid[i][j+1][1] == -2) && (grid[i][j][0] == grid[i][j+1][0])) || (grid[i][j][1] == -1))
                                klag = 1;
                if (klag == 0)
                {
                        squeezex(i,xmax);
                        (*xmax)--;
                        i--;
                }
        }

        for(i=1;i< (*ymax)-1;i++)
        {
                klag = 0;
                for(j=1;j<(*xmax)-1;j++)
                        if (((grid[j][i][1] == -2) && 
                             (grid[j+1][i][1] == -2) && 
                             (grid[j+1][i][0] == grid[j][i][0])) 
                         || (grid[j][i][1] == -1))
                                klag = 1;
                if (klag == 0)
                {
                        squeezey(i,ymax,0,xmax);
                        (*ymax)--;
                        i--;
                }
        }
}

void display(xmax, ymax)
int xmax, ymax;
{

        int j,k;

        for (j=0;j<xmax;j++)
        {
                printf("\t");
                for (k=0;k<ymax;k++)
                {
                        switch (grid[j][k][1]) {
                        case 0 :
                                printf(".");
                                break;
                        case (-1):
                                printf("#");
                                break;
                        case (-2):
                                printf("%c", (char) (65+grid[j][k][0]/100));
                                break;
                        default :
                                printf("^");
                                break;
                        }
                }
                printf("\n");
        }
        printf("\n\t Total die area is %d X %d sq. unit\n",
        xmax,ymax);
}

void squeezex(x1,xmax1)
int x1, *xmax1;
{
        int i, j;

        for (i=x1+1;i< *xmax1;i++)
                for (j=0;j<YDIM; j++)
                {
                        grid[i-1][j][1] = grid[i][j][1];
                        grid[i-1][j][0] = grid[i][j][0];
                }
}

void squeezey(y1,ymax1,xmin1,xmax1)
int y1, *ymax1, xmin1, *xmax1;
{
        int i, j;

        for (i = y1+1;i< *ymax1;i++)
                for (j=xmin1;j<*xmax1;j++)
                {
                        grid[j][i-1][1] = grid[j][i][1];
                        grid[j][i-1][0] = grid[j][i][0];
                }
}

