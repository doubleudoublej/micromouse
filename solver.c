#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <limits.h>
#include <math.h>
#include "API.h"

#define INF 0
#define N 16
char buffer[20];
int dx[] = {0, 1, 0, -1}, dy[] = {1, 0, -1, 0};

// A structure to represent a queue
struct Queue {
    int front, rear, size;
    unsigned capacity;
    unsigned char* array;
};

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (unsigned char*)malloc(
        queue->capacity * sizeof(unsigned char));
    return queue;
}

// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, unsigned char item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
        % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

// Function to remove an item from queue.
// It changes front and size
unsigned char dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return UCHAR_MAX;
    unsigned char item = queue->array[queue->front];
    queue->front = (queue->front + 1)
        % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
unsigned char front(struct Queue* queue)
{
    if (isEmpty(queue))
        return UCHAR_MAX;
    return queue->array[queue->front];
}

// Function to get rear of queue
unsigned char rear(struct Queue* queue)
{
    if (isEmpty(queue))
        return UCHAR_MAX;
    return queue->array[queue->rear];
}

//logi print
void logit(char* text) {
    fprintf(stderr, "%s\n", text);
    fflush(stderr);
}

void logWall(int a) {
    fprintf(stderr, "%d. exploration run...\n", a);
    fflush(stderr);
}

//count directions of cell
int countWays(){
    return !API_wallFront() + !API_wallRight() + !API_wallLeft();
}

//move and change position as x, y
void move(int* x, int* y, int turnR){
    API_moveForward();
    *x += dx[turnR];
    *y += dy[turnR];
}

//left-handed turn
void left_handed(int* turnR){
    if (!API_wallLeft()) {
        API_turnLeft();
        *turnR = (*turnR + 3) % 4;
    }
    while (API_wallFront()){
        API_turnRight();
        *turnR = (*turnR + 1) % 4;
    }
}

//mark green if visited
void markCell(char color, int x, int y){
    API_setColor(x, y, color);
}

/*
//compare cells by distance to 7, 7
int getCost(int turn, int turnR){
    return abs((x + dx[(turnR + turn) % 4]) - 7) + abs((y + dy[(turnR + turn) % 4]) - 7);
}
*/

//return 1 if entered in finish grid
int isFinish(int x, int y){
    if((x==7 && y==7) || (x==7 && y==8) || (x==8 && y==7) || (x==8 && y==8)) return 1;
    return 0;
}

//return 1 if if the location is starting cell
int isStart(int x, int y){
    if(x==0 && y==0) return 1;
    return 0;
}

//flood the grid to finish considering wall data
void floodToFinish(unsigned char flood[N][N], unsigned char walls[N][N]){
    int x, y, nx, ny;
    struct Queue* q = createQueue(100);
    for(int i = 0; i < 16; i++)
        for(int j = 0; j < 16; j++)
            flood[i][j] = INF;

    flood[7][7] = 1; flood[7][8] = 1; flood[8][7] = 1; flood[8][8] = 1;
    enqueue(q, 7); enqueue(q, 7); enqueue(q, 7); enqueue(q, 8); enqueue(q, 8); enqueue(q, 7); enqueue(q, 8); enqueue(q, 8);
    
    while(!isEmpty(q)){
        x = dequeue(q); y = dequeue(q);
		sprintf(buffer, "%d", flood[x][y]);
		API_setText(x, y, buffer);
        for(int i = 0; i < 4; i++){
			nx = x + dx[i]; ny = y + dy[i];
			if(0 <= nx && nx < 16 && 0 <= ny && ny < 16 && flood[nx][ny] == INF && !(walls[x][y] & (1 << i))){
				flood[nx][ny] = flood[x][y] + 1;
				enqueue(q, nx); enqueue(q, ny);
			}
		}
    }
    free(q->array);
    free(q);
}

//flood the grid to start considering wall data
void floodToStart(unsigned char flood[N][N], unsigned char walls[N][N]){
    int x, y, nx, ny;
    struct Queue* q = createQueue(100);
    for(int i = 0; i < 16; i++)
        for(int j = 0; j < 16; j++)
            flood[i][j] = INF;

    flood[0][0] = 1; enqueue(q, 0); enqueue(q, 0);
    
    while(!isEmpty(q)){
        x = dequeue(q); y = dequeue(q);
		sprintf(buffer, "%d", flood[x][y]);
		API_setText(x, y, buffer);
        for(int i = 0; i < 4; i++){
			nx = x + dx[i]; ny = y + dy[i];
			if(0 <= nx && nx < 16 && 0 <= ny && ny < 16 && flood[nx][ny] == INF && !(walls[x][y] & (1 << i))){
				flood[nx][ny] = flood[x][y] + 1;
				enqueue(q, nx); enqueue(q, ny);
			}
		}
    }
    free(q->array);
    free(q);
}

//store explored wall data
void storeWalls(int x, int y, int turnR, unsigned char walls[N][N]){
    char wx, wy;
    if(API_wallFront()){
        wx = x + dx[turnR]; wy = y + dy[turnR];
        walls[x][y] |= (1 << turnR);
        if(!(wx & 0xf0 || wy & 0xf0))
            walls[wx][wy] |= (1 << ((turnR+2)%4));
    }
    if(API_wallRight()){
        wx = x + dx[(turnR+1)%4]; wy = y + dy[(turnR+1)%4];
        walls[x][y] |= (1 << ((turnR+1)%4));
        if(!(wx & 0xf0 || wy & 0xf0))
            walls[wx][wy] |= (1 << ((turnR+3)%4));
    }
    if(API_wallLeft()){
        wx = x + dx[(turnR+3)%4]; wy = y + dy[(turnR+3)%4];
        walls[x][y] |= (1 << ((turnR+3)%4));
        if(!(wx & 0xf0 || wy & 0xf0))
            walls[wx][wy] |= (1 << ((turnR+1)%4));
    }
}

void setWalls(int x, int y, unsigned char walls[N][N]) {
    if (walls[x][y] & (1 << 0)) // Check if there is a wall to the north
        API_setWall(x, y, 'n');
    if (walls[x][y] & (1 << 1)) // Check if there is a wall to the east
        API_setWall(x, y, 'e');
    if (walls[x][y] & (1 << 2)) // Check if there is a wall to the south
        API_setWall(x, y, 's');
    if (walls[x][y] & (1 << 3)) // Check if there is a wall to the west
        API_setWall(x, y, 'w');
}

int main(int argc, char* argv[]){

    int x=0, y=0, turnR=0;
    unsigned char flood[16][16] = {0};
    unsigned char walls[16][16] = {0};

    for (int i=0; i<3; i++){
        logWall(i+1);
        while (!isFinish(x, y)) {
            storeWalls(x, y, turnR, walls);
            setWalls(x,y,walls);
            if(countWays() < 2){
                left_handed(&turnR);
            }
            else{
                floodToFinish(flood, walls);
                if(flood[x+dx[turnR]][y+dy[turnR]] < flood[x][y] && !API_wallFront());
                else if(flood[x+dx[(turnR+1)%4]][y+dy[(turnR+1)%4]] < flood[x][y] && !API_wallRight()){
                    API_turnRight();
                    turnR = (turnR+1) % 4;
                }
                else if(flood[x+dx[(turnR+3)%4]][y+dy[(turnR+3)%4]] < flood[x][y] && !API_wallLeft()){
                    API_turnLeft();
                    turnR = (turnR+3) % 4;
                }
                else left_handed(&turnR);
            }
            markCell('G', x, y);
            move(&x, &y, turnR);
        }
        storeWalls(x, y, turnR, walls);
        setWalls(x,y,walls);

        while(!isStart(x, y)){
            storeWalls(x, y, turnR, walls);
            setWalls(x,y,walls);
            if(countWays() < 2){
                left_handed(&turnR);
            }
            else{
                floodToStart(flood, walls);
                if(flood[x+dx[turnR]][y+dy[turnR]] < flood[x][y] && !API_wallFront());
                else if(flood[x+dx[(turnR+1)%4]][y+dy[(turnR+1)%4]] < flood[x][y] && !API_wallRight()){
                    API_turnRight();
                    turnR = (turnR+1) % 4;
                }
                else if(flood[x+dx[(turnR+3)%4]][y+dy[(turnR+3)%4]] < flood[x][y] && !API_wallLeft()){
                    API_turnLeft();
                    turnR = (turnR+3) % 4;
                }
                else left_handed(&turnR);
            }
            markCell('G', x, y);
            move(&x, &y, turnR);
        }
        storeWalls(x, y, turnR, walls);
        setWalls(x,y,walls);
    }

    Sleep(5000);
    logit("Run on optimised path...");
    floodToFinish(flood, walls);
    while(!isFinish(x, y)){
        if(countWays() < 2){
            left_handed(&turnR);
        }
        else{
            if(flood[x+dx[turnR]][y+dy[turnR]] < flood[x][y] && !API_wallFront());
            else if(flood[x+dx[(turnR+1)%4]][y+dy[(turnR+1)%4]] < flood[x][y] && !API_wallRight()){
                API_turnRight();
                turnR = (turnR+1) % 4;
            }
            else if(flood[x+dx[(turnR+3)%4]][y+dy[(turnR+3)%4]] < flood[x][y] && !API_wallLeft()){
                API_turnLeft();
                turnR = (turnR+3) % 4;
            }
            else left_handed(&turnR);
        }
        markCell('R', x, y);
        move(&x, &y, turnR);
    }

    return 0;
}
