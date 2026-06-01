#include <stdio.h>
#include <stdlib.h>
#include "API.h"
#include <stdbool.h>
#include <string.h>

#define MAZE_SIZE 16

typedef struct xy_coordinates coor; 

struct xy_coordinates {
    int x;
    int y;
    int dir;
    char dir_values[4];
};

typedef struct where_walls walls;
struct where_walls {
    bool north;
    bool east; 
    bool south;
    bool west;
    int value;
    int value_back;
};

typedef struct queue_node Qnode;
struct queue_node {
    int x;
    int y; 
    struct queue_node* next;
};

typedef struct Queue_system Queue; 
struct Queue_system {
    Qnode* start;
    Qnode* end;
};

Queue* createQueue(){
    Queue* Q = malloc(sizeof(Queue));
    Q->start = NULL;
    Q->end = NULL;
    return Q;
}

void enqueue(Queue* Q, int x, int y){
    Qnode* n = malloc(sizeof(Qnode));
    n->x = x;
    n->y = y;
    n->next = NULL;

    if(Q->start == NULL){
        Q->start = n;
        Q->end = n;
    }
    else{
        Q->end->next = n;
        Q->end = n;
    }
}

Qnode* dequeue(Queue* Q){   
    if(Q->start == NULL){
        return NULL;
    }
    Qnode* n = Q->start;
    Q->start = Q->start->next;

    if(Q->start == NULL){
        Q->end = NULL;
    }
    return n;
}

bool queue_empty(Queue* Q){
    return Q->start == NULL;
}

typedef struct stack_node Snode;
struct stack_node{
    int x;
    int y;
    struct stack_node* next;
};

typedef struct Stack_system Stack;
struct Stack_system{
    Snode* top;
};

Stack* createStack(){
    Stack* S = malloc(sizeof(Stack));
    S->top = NULL;
    return S;
}

void push(Stack* S, int x, int y){
    Snode* n = malloc(sizeof(Snode));
    n->x = x;
    n->y = y;
    n->next = S->top;
    S->top = n;
}

//FINISH STACK

//Prints message out
void log_message(char* text) {
    fprintf(stderr, "%s\n", text);
    fflush(stderr);
}

//modified left and right so they update dir
//up is 0, right is 1, down is 2, left is 3
//when mod by 4. Need to make sure to add pos multiples of 4 to make it 
//pos before mod
int turnLeft(coor *C){
    API_turnLeft();
    return C->dir--;
}

int turnRight(coor *C){
    API_turnRight();
    return C->dir++;
}

//updates coordinates
void update_coor(coor *C){
    if(C->dir <0){
        C->dir = ((abs(C->dir)/4) + 1) * 4 + C->dir;
    }
    C->dir = C->dir % 4;
    
    switch(C->dir) {
        case 0:
            C->y += 1;
            break;
        case 1:
            C->x += 1;
            break;
        case 2:
            C->y -= 1;
            break;
        case 3:
            C->x -= 1;
            break;
    }
}

//modified set wall so that I can keep track of the walls
void custom_setWall(int x, int y, char dir, walls *maze[MAZE_SIZE][MAZE_SIZE]){
    API_setWall(x, y, dir);
    
    switch(dir){
        case 'n':
            maze[x][y]->north = true;
            if(y < 15) maze[x][y+1]->south = true;
            break;
        case 'e':
            maze[x][y]->east = true;
            if(x < 15) maze[x+1][y]->west = true;
            break;
        case 's':
            maze[x][y]->south = true;
            if(y > 0) maze[x][y-1]->north = true;
            break;
        case 'w':
            maze[x][y]->west = true;
            if(x > 0) maze[x-1][y]->east = true;
            break;
    }
}

//when see wall add wall;
void set_wall(coor *C, walls *maze[MAZE_SIZE][MAZE_SIZE]){
    if(API_wallLeft()){
        custom_setWall(C->x, C->y, C->dir_values[(C->dir + 3) % 4], maze);
    }
    if(API_wallFront()){
        custom_setWall(C->x, C->y, C->dir_values[C->dir], maze);
    }
    if(API_wallRight()){
        custom_setWall(C->x, C->y, C->dir_values[(C->dir + 1) % 4], maze);
    }
}

//helper function to make sure the walls are updated correctly
void update_walltext(walls *maze[MAZE_SIZE][MAZE_SIZE]){
    for (int i = 0; i < MAZE_SIZE; i++){
        for (int j = 0; j < MAZE_SIZE; j++){
            char wall_text[10] = ""; 

            if (maze[i][j]->north) {
                strcat(wall_text, "n");
            }
            if (maze[i][j]->east) {
                strcat(wall_text, "e");
            }
            if (maze[i][j]->south) {
                strcat(wall_text, "s");
            }
            if (maze[i][j]->west) {
                strcat(wall_text, "w");
            }

            API_setText(i, j, wall_text);
        }
    }
}

//Updates text, value, and adds to queue
void wavefront_helper(walls *maze[MAZE_SIZE][MAZE_SIZE], int x, int y, int value, Queue* Q){
    char str[5];

    snprintf(str, sizeof(str), "%d", value);
    API_setText(x, y, str);
    maze[x][y]->value = value;

    enqueue(Q, x, y);
}

void wavefront_helper_back(walls *maze[MAZE_SIZE][MAZE_SIZE], int x, int y, int value, Queue* Q){
    char str[5];

    snprintf(str, sizeof(str), "%d", value);
    //API_setText(x, y, str); // can comment out later
    maze[x][y]->value_back = value;

    enqueue(Q, x, y);
}

//Just runs the wavefront every time
void update_wavefront(walls *maze[MAZE_SIZE][MAZE_SIZE]){

    Queue* Q = createQueue();

    for(int i = 0; i< MAZE_SIZE; i++){
        for(int j = 0; j < MAZE_SIZE; j++){
            maze[i][j]->value = -1;
        }
    }

    wavefront_helper(maze, 7,7,0, Q);
    wavefront_helper(maze, 8,7,0, Q);
    wavefront_helper(maze, 7,8,0, Q);
    wavefront_helper(maze, 8,8,0, Q);

    while(!queue_empty(Q)){
        Qnode* n = dequeue(Q);
        //check all four directions for no wall
        walls* w = maze[n->x][n->y];
        if(!w->north && n->y < 15){
            walls* w1 = maze[n->x][n->y+1];
            if(w1->value == -1){
                wavefront_helper(maze, n->x, n->y+1, w->value+1, Q);
            }
        }
        if(!w->east && n->x < 15){
            walls* w1 = maze[n->x+1][n->y];
            if(w1->value == -1){
                wavefront_helper(maze, n->x+1, n->y, w->value+1, Q);
            }
        }
        if(!w->south && n->y > 0){
            walls* w1 = maze[n->x][n->y-1];
            if(w1->value == -1){
                wavefront_helper(maze, n->x, n->y-1, w->value+1, Q);
            }
        }
        if(!w->west && n->x > 0){
            walls* w1 = maze[n->x-1][n->y];
            if(w1->value == -1){
                wavefront_helper(maze, n->x-1, n->y, w->value+1, Q);
            }
        }
        free(n);
    }

    free(Q);
    //log_message("Finished wavefront");
}

void update_wavefront_back(walls *maze[MAZE_SIZE][MAZE_SIZE], int x, int y){

    Queue* Q = createQueue();

    for(int i = 0; i< MAZE_SIZE; i++){
        for(int j = 0; j < MAZE_SIZE; j++){
            maze[i][j]->value_back = -1;
        }
    }

    wavefront_helper_back(maze, 0,0,0, Q);

    while(!queue_empty(Q)){
        Qnode* n = dequeue(Q);
        //check all four directions for no wall
        walls* w = maze[n->x][n->y];
        if(!w->north && n->y < 15){
            walls* w1 = maze[n->x][n->y+1];
            if(w1->value_back == -1){
                wavefront_helper_back(maze, n->x, n->y+1, w->value_back+1, Q);
            }
        }
        if(!w->east && n->x < 15){
            walls* w1 = maze[n->x+1][n->y];
            if(w1->value_back == -1){
                wavefront_helper_back(maze, n->x+1, n->y, w->value_back+1, Q);
            }
        }
        if(!w->south && n->y > 0){
            walls* w1 = maze[n->x][n->y-1];
            if(w1->value_back == -1){
                wavefront_helper_back(maze, n->x, n->y-1, w->value_back+1, Q);
            }
        }
        if(!w->west && n->x > 0){
            walls* w1 = maze[n->x-1][n->y];
            if(w1->value_back == -1){
                wavefront_helper_back(maze, n->x-1, n->y, w->value_back+1, Q);
            }
        }
        free(n);
    }

    free(Q);
    //log_message("Finished wavefront");
}

Queue* find_path(coor *C, walls *maze[MAZE_SIZE][MAZE_SIZE], bool forward){
    Queue* path = createQueue();
    
    int x = C->x;
    int y = C->y;

    while(true){
        walls* w = maze[x][y];
        bool north = false;
        bool east = false;
        bool south = false;
        bool west = false;
        
        if(forward){
            if(!w->north && y < 15 && maze[x][y+1]->value < w->value){
            north = true;
            //log_message("north is true");
            //y = y+1;
            }
            if(!w->east && x < 15 && maze[x+1][y]->value < w->value){
                east = true;
                //log_message("east is true");
                //x = x+1;
            }
            if(!w->south && y > 0 && maze[x][y-1]->value < w->value){
                south = true;
                //log_message("south is true");
                //y = y-1;
            }
            if(!w->west && x > 0 && maze[x-1][y]->value < w->value){
                west = true;
                //log_message("west is true");
                //x = x-1;
            }
        }
        else{
            if(!w->north && y < 15 && maze[x][y+1]->value_back < w->value_back){
            north = true;
            //log_message("north is true");
            //y = y+1;
            }
            if(!w->east && x < 15 && maze[x+1][y]->value_back < w->value_back){
                east = true;
                //log_message("east is true");
                //x = x+1;
            }
            if(!w->south && y > 0 && maze[x][y-1]->value_back < w->value_back){
                south = true;
                //log_message("south is true");
                //y = y-1;
            }
            if(!w->west && x > 0 && maze[x-1][y]->value_back < w->value_back){
                west = true;
                //log_message("west is true");
                //x = x-1;
            }
        }

        if(C->dir == 0){
            if(north) y++;
            else if (east) x++;
            else if(west) x--;
            else y--;
        }
        else if(C->dir == 1){
            if(east) x++;
            else if(north) y++;
            else if(south) y--;
            else x--;
        }
        else if(C->dir == 2){
            if(south) y--;
            else if(east) x++;
            else if(west) x--;
            else y++;
        }
        else{
            if(west) x--;
            else if(south) y--;
            else if(north) y++;
            else x++;
        }

        enqueue(path, x, y);
        API_setColor(x,y,'G');

        if(maze[x][y]->value == 0 && forward){
            break;
        }
        if(maze[x][y]->value_back == 0 && !forward){
            break;
        }
    }

    return path;
}

void moveTo(coor *C, Qnode *next){
    int direction;
    if(C->y < next->y){ //needs to go north
        direction = 0;
    }
    else if(C->x < next->x){ //needs to go east
        direction = 1;
    }
    else if(C->y > next->y){ //needs to go south
        direction = 2;
    }
    else{ //needs to go west
        direction = 3;
    }

    direction = (direction - C->dir + 4) % 4; //how many times to turn right
    switch(direction){
        case 1:
            turnRight(C);
            break;
        case 2:
            turnRight(C);
            turnRight(C);
            break;
        case 3:
            turnLeft(C);
            break;
    }
    API_moveForward();
}

void clear_path(Queue* path){
    while(!queue_empty(path)){
        Qnode* n = dequeue(path);
        API_setColor(n->x, n->y, 'k');
        free(n);
    }
    free(path);
}

void clear_color(){
    for (int i = 0; i < MAZE_SIZE; i++){
        for (int j = 0; j < MAZE_SIZE; j++){
            API_setColor(i, j, 'k');
        }
    }
}

int main(int argc, char* argv[]) {
    log_message("Running...");
    API_setColor(0, 0, 'G');
    API_setText(0, 0, "abc");
    coor *c = calloc(1, sizeof(coor));
    c->dir_values[0] = 'n';
    c->dir_values[1] = 'e';
    c->dir_values[2] = 's';
    c->dir_values[3] = 'w';

    //initalize wall 2d array 
    //all set to false
    walls *maze[MAZE_SIZE][MAZE_SIZE];
    for (int i = 0; i< MAZE_SIZE; i++){
        for (int j = 0; j < MAZE_SIZE; j++){
            maze[i][j] = calloc(1, sizeof(walls));
            maze[i][j]->value = -1;
            maze[i][j]->value_back = -1;
            API_setText(i, j, "-1");
        }
    }

    //set boundary walls
    for(int i = 0; i < MAZE_SIZE; i++){
        custom_setWall(i, 0, 's', maze);
        custom_setWall(i, 15, 'n', maze);
        custom_setWall(0, i, 'w', maze);
        custom_setWall(15, i, 'e', maze);
    }
    //log_message("Boundary Walls set");
    set_wall(c, maze);

    while (maze[c->x][c->y]->value != 0) {
        char log_temp[50]; // Make a buffer big enough to hold the whole message
        snprintf(log_temp, sizeof(log_temp), "New action, current coords are x: %d y: %d", c->x, c->y);
        //log_message(log_temp);
        set_wall(c, maze);
        update_wavefront(maze);
        Queue* path = find_path(c, maze, true);
        
        Qnode* next = dequeue(path);
        moveTo(c, next);
        update_coor(c);
        //set_wall(c, maze);
        //update_walltext(maze);
        free(next);
        clear_path(path);
        clear_color();
    }

    
    while(maze[c->x][c->y]->value_back != 0){
        set_wall(c, maze);
        update_wavefront_back(maze, c->x, c->y);
        update_wavefront(maze);
        Queue* path = find_path(c, maze, false);

        Qnode* next = dequeue(path);
        moveTo(c, next);
        update_coor(c);
        free(next);
        clear_path(path);
        clear_color();
    }

    Queue* path = find_path(c, maze, true);
    free(path);
}

//NOTES
/*Make it so that once it finishes it takes what it thinks is the fastest way 
back
Use the code I wrote just in reverse
*/
