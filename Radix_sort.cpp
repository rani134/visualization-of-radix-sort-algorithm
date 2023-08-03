#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<GL/glut.h>
#include<string.h>
#define MAX 10

int k=0;
typedef struct circle
{
	GLfloat x;	//x axis of center
	GLfloat y;	//y axis of center
	GLfloat r;	// radius this is corresponding value inside circle
}circle;

typedef struct bucket
{
    GLfloat nextCircleX;
    GLfloat nextCircleY;
    int circleCount;
    GLfloat bucketLeftTopX;
    GLfloat bucketLeftTopY;
    int bucketIndex;
    int circleArrayPointer;
    int ballNumberStored[MAX];
    circle balls[MAX];//when a circle is pushed into bucket we will store that circle object inside the bucket WE CAN REMOVE THIS IT IS NOT USED AND IS REDUNDANT
}bucket;

circle c[MAX];	//array of circles to store the center and radius of each circle
circle cTemp[MAX];
bucket b[MAX];//array of buckets b0-b9 for each integer base 10
bucket upOpenBucket[MAX];
int a[MAX];		// int array for sorting algorithm
int initial[MAX] = {48,36,22,13,3,1,24,35,79,97};	//int array to restore random values after sorting
int initialTEMP[MAX] = {48,36,22,13,3,1,24,35,79,97};	//int array to restore random values after sorting
int sorting = 0;	//start sorting only after user input

//radix sort variables:
int currentIndex = 0;
int MAXINDEX=2;
int moving=0;
int movingBalls=0;
int cTempPointer=0;
int currentBall_i=0;
int processCount=0;
int bucketMoveCount=0;
int moveBucketUpBy400=800;
int bringBallsDownDestBall_i=0;
int bringBallsDownBucket_i=0;
int bringBallsDownNumberOfBallsInBucket;
int bringBallsDownBallCount_i=0;
float moveBallDownDestY;
float moveBallDownDestX;
int isMovingBallDown=0;
int radix;
float destX, destY;
int DONESORTINGPROCESS=0;
int showUpBuckets=1;

int getMaxIndex()
{
    int maxElement=0;
    for(int i=0;i<MAX;i++)
    {
        if(initial[i]>maxElement)
            maxElement=a[i];
    }
    int maxRadix=0;
    maxElement/=10;
    while(maxElement!=0)
    {
        maxRadix++;
        maxElement/=10;
    }
    return maxRadix;
}

void initialise()
{
	//initialize all circles
	for(int i=0;i<MAX;i++)
	{
		a[i] = initial[i];	//if a[] is sorted restore from initial[]
		c[i].r = a[i];	//4 because to fit 10 circles in screen
		c[i].y = 400.0;		//vertical baseline for the circles of window 0->700
		if (i == 0)
			c[i].x = 90.0;	// first circle starts from 90 offset
		else
			c[i].x = c[i-1].x + 90.0;//(c[i-1].r+c[i].r+10.0); //distance between circles = sum of 2 max radii

	}
    //initialize all buckets
    GLfloat bucketX=48,bucketY=250;
    /*For the Bottom Buckets
    top left (x,y)
    top right (x+84)
    bottom left(x+10,y-160)
    bottom right (x+74,y-160)
    center X of bucket: x+42
    initial Y of bucket: y-128
    */
    /*For the Top Buckets
    top left (x+10,y)
    top right (x+74)
    bottom left(x,y-160)
    bottom right (x+84,y-160)
    center X of bucket: x+42
    initial Y of bucket: y-128
    */

    for(int i=0;i<MAX;i++)//initialize bottom buckets
    {
        b[i].bucketLeftTopX=bucketX;
        b[i].bucketLeftTopY=bucketY;
        b[i].bucketIndex=i;
        b[i].nextCircleY=bucketY-128;//next circle at a distance of 60+2
        b[i].nextCircleX=bucketX+42;//X coordinate stays as is once initialized
        b[i].circleCount=0;
        b[i].circleArrayPointer=0;
        bucketX=bucketX+89;
    }
    GLfloat upBucketX=48,upBucketY=650;
    for(int i=0;i<MAX;i++)//initialize top buckets
    {
        upOpenBucket[i].bucketLeftTopX=upBucketX;
        upOpenBucket[i].bucketLeftTopY=upBucketY;
        upOpenBucket[i].bucketIndex=i;
        upBucketX=upBucketX+89;
    }

}

//function to display text on screen char by char
void bitmap_output(int x, int y, char *str, void *font)
{
  int len, i;
  glRasterPos2f(x, y);
  len = (int) strlen(str);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, str[i]);
  }
}

void int_str(int num, char r[])
{
    int i, rem, len = 0, n;
    n = num;
    if(n==0)
    {
        r[0]='0';
        r[1]='\0';
        return;
    }
    while (n != 0)
    {
        len++;
        n=n/10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        r[len - (i + 1)] = rem + '0';
    }
    r[len] = '\0';
}

//draw circle by drawing consecutive triangle fans
void circle_draw(circle c)
{
	float i;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(c.x, c.y);	//center of circle
	for (i=0;i<360;i+=1)
		glVertex2f(c.x + sin(i) * 30, c.y + cos(i) * 30  );
	glEnd();

	//display the value of circle inside
	int x = c.x;
	int y = c.y;
	int rad = c.r;
	char r[3] = "";
	int_str(rad,r);
	glColor3f(0.0,0.0,0.0);
	bitmap_output(x, y, r, GLUT_BITMAP_9_BY_15);

}


void up_bucket_draw(bucket bu)
{
    /*For the Top Buckets
    top left (x+10,y)
    top right (x+74)
    bottom left(x,y-160)
    bottom right (x+84,y-160)
    center X of bucket: x+42
    initial Y of bucket: y-128
    */
    glBegin(GL_LINE_LOOP);
        glVertex2f(bu.bucketLeftTopX+10,bu.bucketLeftTopY);//top left
        glVertex2f(bu.bucketLeftTopX+74,bu.bucketLeftTopY);//top right
        glVertex2f(bu.bucketLeftTopX+84,bu.bucketLeftTopY-160);//bottom right
        glVertex2f(bu.bucketLeftTopX,bu.bucketLeftTopY-160);//bottom left
    glEnd();
     glColor3f(0,0,1);
     glPointSize(10);
        glBegin(GL_POINTS);
            glVertex2f(bu.nextCircleX,bu.nextCircleY);
        glEnd();
    //display the index of bucket under
    int x=bu.bucketLeftTopX+42;
    int y=bu.bucketLeftTopY+25;
	int rad = bu.bucketIndex;
	char r[3] = "";
	int_str(rad,r);
	glColor3f(0.0,0.0,0.0);
	bitmap_output(x, y, r, GLUT_BITMAP_HELVETICA_18);

}
//================================================================================================================================================

//_____________________________________SORTING LOGIC FOR RADIX SORT__________________________________________________________

void move_to_bucket(int ball_i, int destination_bucket)
{
    if(moving==0)
    {
        destX = b[destination_bucket].nextCircleX;
        destY = b[destination_bucket].nextCircleY;
        //initialize temp circle array to hold the coordinates of the next iteration we intialize the position coordinates as well
        cTemp[ball_i]=c[ball_i];//copy the initial value and coordinates so we can reset later
        initialTEMP[ball_i]=initial[ball_i];
        moving = 1;
    }
    if(c[ball_i].y>300)
        c[ball_i].y-=0.5;

    //move ball left or right
    else if(c[ball_i].x<destX)
        c[ball_i].x+=0.5;

    else if(c[ball_i].x>destX)
        c[ball_i].x-=0.5;

    //move ball down
    else if(c[ball_i].y>destY)
        c[ball_i].y-=0.5;

    //move completed
    if (abs(c[ball_i].x-destX) < 0.3 && abs(c[ball_i].y-destY) < 0.3)
	{
		moving = 0;
        b[destination_bucket].circleCount++;
        b[destination_bucket].nextCircleY+=62;
        circle temp=c[ball_i];
        int tempPtr=b[destination_bucket].circleArrayPointer;
        b[destination_bucket].balls[b[destination_bucket].circleArrayPointer]=temp;//each bucket n will have an array containing the obj of balls inside it
        b[destination_bucket].ballNumberStored[tempPtr]=currentBall_i;//each bucket n will have an array containing the obj of balls inside it
        b[destination_bucket].circleArrayPointer++;
        currentBall_i++;
    }
}

void bringBallsBackDown()
{
     //select bucket
     if(bringBallsDownBucket_i==MAX)//maybe behavior in this case is different
        return;
     if(b[bringBallsDownBucket_i].circleCount==0&&isMovingBallDown==0)
     {
        bringBallsDownBucket_i++;
     }

     else//choose ball in bucket remeber copy into circletemp then at end after motion copy circletemp to circle
     {
        if(isMovingBallDown==0)
        {
            moveBallDownDestX=cTemp[bringBallsDownDestBall_i].x;
            moveBallDownDestY=cTemp[bringBallsDownDestBall_i].y;
            isMovingBallDown=1;
        }
        if(c[b[bringBallsDownBucket_i].ballNumberStored[bringBallsDownBallCount_i]].y>450)
            c[b[bringBallsDownBucket_i].ballNumberStored[bringBallsDownBallCount_i]].y-=0.5;
        else if(c[b[bringBallsDownBucket_i].ballNumberStored[bringBallsDownBallCount_i]].x<cTemp[bringBallsDownDestBall_i].x)
            c[b[bringBallsDownBucket_i].ballNumberStored[bringBallsDownBallCount_i]].x+=0.5;
        else if(c[b[bringBallsDownBucket_i].ballNumberStored[bringBallsDownBallCount_i]].x>cTemp[bringBallsDownDestBall_i].x)
            c[b[bringBallsDownBucket_i].ballNumberStored[bringBallsDownBallCount_i]].x-=0.5;
        else if(c[b[bringBallsDownBucket_i].ballNumberStored[bringBallsDownBallCount_i]].y>cTemp[bringBallsDownDestBall_i].y)
            c[b[bringBallsDownBucket_i].ballNumberStored[bringBallsDownBallCount_i]].y-=0.5;
        if(abs(c[b[bringBallsDownBucket_i].ballNumberStored[bringBallsDownBallCount_i]].x-cTemp[bringBallsDownDestBall_i].x) < 0.3 && abs(c[b[bringBallsDownBucket_i].ballNumberStored[bringBallsDownBallCount_i]].y-cTemp[bringBallsDownDestBall_i].y) < 0.3)//move completed
        {
            isMovingBallDown=0;
            b[bringBallsDownBucket_i].circleCount--;
            b[bringBallsDownBucket_i].nextCircleY-=62;
            b[bringBallsDownBucket_i].circleArrayPointer--;
            cTemp[bringBallsDownDestBall_i].r=c[b[bringBallsDownBucket_i].ballNumberStored[bringBallsDownBallCount_i]].r;
            if(b[bringBallsDownBucket_i].circleCount==0)//No more balls in bucket
            {
                bringBallsDownBucket_i++;
                bringBallsDownDestBall_i++;
                bringBallsDownBallCount_i=0;//reset stucture array access pointer back to 0
            }
            else
            {
                bringBallsDownDestBall_i++;
                bringBallsDownBallCount_i++;
            }
        }

     }
    if(bringBallsDownDestBall_i==MAX)
    {
        for(int i=0;i<MAX;i++)
        {
            c[i]=cTemp[i];
        }
       DONESORTINGPROCESS=1;
    }
}

void processBuckets()//this function should draw top buckets, move balls up, create the new array, move balls down sort will resume when you return
{
    //draw up buckets
    if(showUpBuckets==1)
    {
        for(int i=0;i<MAX;i++)
        {
            up_bucket_draw(upOpenBucket[i]);
        }
    }
    //move all balls up
    if(movingBalls==0)
    {
        movingBalls = 1;
    }
    if(moveBucketUpBy400>=0)
    {
        for(int i=0;i<MAX;i++)//set the next interval for ALL BALLS to move up
        {
            c[i].y+=0.5;
        }
        moveBucketUpBy400--;
    }
    if(moveBucketUpBy400<0)//move balls back to center position for resort
    {
        movingBalls=0;
        bringBallsBackDown();

    }
}

void sort()
{
	//int radix;
//	MAXINDEX=2;
	if(moving == 0&&currentIndex<=getMaxIndex()&&currentBall_i<MAX) //if not in process of moving a ball, process the next ball
	{
		while (currentBall_i < MAX)
		{
			//get the integer at the index:
            radix = c[currentBall_i].r;
            for(int div=0;div<currentIndex;div++)
            {
                radix=radix/10;
            }
            radix = radix % 10;
            goto MOVE;
		}
		if(currentBall_i<MAX)
            currentBall_i++;
    }
    MOVE: move_to_bucket(currentBall_i,radix);
    if(currentBall_i==MAX)
    {
        processBuckets();
        upOpenBucket[0].bucketLeftTopY=650;//DONE BECAUSE UP BUCKET 0 WANDERS TO BOTTOM OF SCREEN WITHOUT REASON AFTER SORT()
    }
}
//================================================================================================================================================

void display_instructions()
{
	//about radix sort
	bitmap_output(360, 525, "About Radix Sort",GLUT_BITMAP_HELVETICA_18);
	bitmap_output(10, 495, "Radix sort is a non-comparative integer sorting algorithm that sorts elements based on iterations over each ",GLUT_BITMAP_9_BY_15);
	bitmap_output(10, 470, "radix of a set of numbers. Complexity of this algorithm depends on the number of digits of the elements. This ",GLUT_BITMAP_9_BY_15);
	bitmap_output(10, 445, "determines the number of iterations. The complexity of radix sort is o(nk) where k is the number of digits.",GLUT_BITMAP_9_BY_15);

	bitmap_output(380, 400, "Radix Sort Steps",GLUT_BITMAP_HELVETICA_18);
	bitmap_output(200, 370, "1) Prepare buckets 0-9 corresponding to each index base 10",GLUT_BITMAP_9_BY_15);
	bitmap_output(200, 345, "2) Take the least significant digit of each key and place the number in the right bucket",GLUT_BITMAP_9_BY_15);
	bitmap_output(200, 320, "3) Starting from bucket 0 to 9 remove keys in order (FIFO Queue)",GLUT_BITMAP_9_BY_15);
	bitmap_output(200, 295, "4) Repeat for every digit in the number with maximum number of digits",GLUT_BITMAP_9_BY_15);
	bitmap_output(200, 270, "5) The final order of elements is the sorted array. ",GLUT_BITMAP_9_BY_15);

	bitmap_output(380, 230, "Instructions",GLUT_BITMAP_HELVETICA_18);
    bitmap_output(300, 190, "Press 's' to start SORT",GLUT_BITMAP_9_BY_15);
    bitmap_output(300, 165, "Press 'ESC' to QUIT",GLUT_BITMAP_9_BY_15);
}

void front()//HOMEPAGE
{
	glColor3f(0.0,0.0,1.0);
	bitmap_output(340, 665, "RADIX SORT DEMO",GLUT_BITMAP_TIMES_ROMAN_24);
	glBegin(GL_LINE_LOOP);
		glVertex2f(330, 660);
		glVertex2f(570, 660);
	glEnd();
	bitmap_output(390, 630, "Computer Graphics Mini Project",GLUT_BITMAP_TIMES_ROMAN_24);
	bitmap_output(380, 595, "4GW20CS090,4GW20CS097",GLUT_BITMAP_HELVETICA_18);
	glColor3f(0.0,1.0,0.0);
	display_instructions();
	glColor3f(1.0,0.0,0.0);
	glBegin(GL_QUADS);
	  	glVertex2f(300,85);glVertex2f(350,110);glVertex2f(542,110);glVertex2f(592,85);
    glEnd();

	glColor3f(1,1,0);
	bitmap_output(354, 90, "Press Enter to Start!",GLUT_BITMAP_HELVETICA_18);

}

void buttons()//Buttons
{
	glColor3f(0.0,0.0,1.0);
	bitmap_output(330, 665, "RADIX SORT COMPLETE!!",GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(1,0,0);
    bitmap_output(320, 600, "Click on a menu option to continue!",GLUT_BITMAP_9_BY_15);

	glColor3f(0.2,0.0,0.5);
	glBegin(GL_QUADS);
	  	glVertex2f(330,555);
	  	glVertex2f(380,580);
	  	glVertex2f(572,580);
	  	glVertex2f(622,555);
    glEnd();
	glColor3f(1,1,1);
	bitmap_output(434, 560, "Main Menu!",GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.0,0.7,0.2);
	glBegin(GL_QUADS);
	  	glVertex2f(380,529);
	  	glVertex2f(330,554);
	  	glVertex2f(622,554);
	  	glVertex2f(572,529);
    glEnd();
	glColor3f(1,1,1);
	bitmap_output(464, 534, "Exit!",GLUT_BITMAP_HELVETICA_18);

}


void bucket_draw(bucket bu)
{
     /*
    top left (x,y)
    top right (x+84)
    bottom left(x+10,y-160)
    bottom right (x+74,y-160)
    center X of bucket: x+42
    initial Y of bucket: y-128
    */
    glBegin(GL_LINE_LOOP);
        glVertex2f(bu.bucketLeftTopX,bu.bucketLeftTopY);//top left
        glVertex2f(bu.bucketLeftTopX+84,bu.bucketLeftTopY);//top right
        glVertex2f(bu.bucketLeftTopX+74,bu.bucketLeftTopY-160);//bottom right
        glVertex2f(bu.bucketLeftTopX+10,bu.bucketLeftTopY-160);//bottom left
    glEnd();
     glColor3f(0,0,1);
     glPointSize(10);
        glBegin(GL_POINTS);
            glVertex2f(bu.nextCircleX,bu.nextCircleY);
        glEnd();
    //display the index of bucket under
    int x=bu.bucketLeftTopX+42;
    int y=bu.bucketLeftTopY-180;
	int rad = bu.bucketIndex;
	char r[3] = "";
	int_str(rad,r);
	glColor3f(0.0,0.0,0.0);
	bitmap_output(x, y, r, GLUT_BITMAP_HELVETICA_18);

}


void drawCirclesAndBuckets(circle *circles, bucket *buckets)
{
    for (int i=0;i<MAX;i++)
	{
		glColor3f(1.0,1.0,0.0);
		circle_draw(circles[i]);
		glColor3f(1,0,0);
		bucket_draw(buckets[i]);
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0,0.0,0.0);
	if (k==0)
        front();
	else if(k==1)
	{
        glPointSize(2.0);
        drawCirclesAndBuckets(c,b);
        if (sorting == 1)
        {
            // call sort only on key press
       //     MAXINDEX=2;
             if(DONESORTINGPROCESS==1&&currentIndex<getMaxIndex())//reset the control variables
            {
                DONESORTINGPROCESS=0;
                currentIndex++;
                moving=0;
                currentBall_i=0;
                moveBucketUpBy400=800;
                bringBallsDownDestBall_i=0;
                bringBallsDownBucket_i=0;
                sort();
            }
            else if(DONESORTINGPROCESS==0&& currentIndex<=getMaxIndex())//sorting is still going on just continue sorting
            {
                sort();
            }
            else
            {
                buttons();
            }
         }
        else
        {
            sorting=0;
        }

	}
	glFlush();
	glutSwapBuffers();
}

void keyboard (unsigned char key, int x, int y)
{
	if(key==13)//13 is ascii for ENTER key
		k=1;
	if (k==1)
	{
        switch (key)
        {
            case 27 : exit (0); //27 is the ascii code for the ESC key
            case 's' : sorting = 1; break;
        }
	}
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(w<=h)
		glOrtho(-2.0, 2.0, -2.0 * (GLfloat) h/ (GLfloat) w, 2.0* (GLfloat) h / (GLfloat) w, -10.0, 10.0);
	else
		glOrtho(-2.0 * (GLfloat) w/ (GLfloat) h, 2.0* (GLfloat) w / (GLfloat) h, -2.0, 2.0, -10.0, 10.0);

	glMatrixMode(GL_MODELVIEW);
}

void mouse(int button,int state, int x,int y)//change this to mimic a button function to change the screen (main menu/restart sorting/etc...)
{
    y=700-y;
    if((button==GLUT_LEFT_BUTTON)&&(state==GLUT_DOWN)&&(x<=600)&&(x>=350)&&(y<=580)&&(y>=555))
    {
        printf("\nmain menu selected");//reset all variables and display front screen
        initialise();
        sorting=0;
        k=0;
        currentIndex = 0;
        MAXINDEX=2;
        moving=0;
        movingBalls=0;
        cTempPointer=0;
        currentBall_i=0;
        processCount=0;
        bucketMoveCount=0;
        moveBucketUpBy400=800;
        bringBallsDownDestBall_i=0;
        bringBallsDownBucket_i=0;
        bringBallsDownNumberOfBallsInBucket;
        bringBallsDownBallCount_i=0;
        moveBallDownDestY;
        moveBallDownDestX;
        isMovingBallDown=0;
        radix;
        DONESORTINGPROCESS=0;
        showUpBuckets=1;
        front();
    }
    if((button==GLUT_LEFT_BUTTON)&&(state==GLUT_DOWN)&&(x<=600)&&(x>=350)&&(y<=554)&&(y>=529))
    {
        printf("\nexit selected");
        exit(0);
    }
}
void init(void)
{
		glClearColor(1.0,1.0,1.0,0.0);
		glMatrixMode(GL_PROJECTION);
		gluOrtho2D(0.0,1000.0,0.0,700.0);
}

int main(int argc, char ** argv)
{
		glutInit(&argc,argv);
		glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
		glutInitWindowPosition(50,50);
		glutInitWindowSize(1000,700);
		printf("RADIX SORT\n\nThis program will sort any %d numbers. Enter %d numbers:\n\n",MAX,MAX);
		for(int i=0;i<MAX;i++)
        {
            printf("Element %d: ",i+1);
            scanf("%d",&initial[i]);
            a[i]=initialTEMP[i]=initial[i];
        }
		glutCreateWindow("RADIX SORT VISUALIZER");
		init();
		initialise();
		glutDisplayFunc(display);
		glutIdleFunc (display);
		glutKeyboardFunc (keyboard);
		glutMouseFunc(mouse);
		glutMainLoop();
		return 0;
}
