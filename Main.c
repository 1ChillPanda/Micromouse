#include <stdio.h>
#include <stdlib.h>
#include "API.h"
#include <stdbool.h>
#include <string.h>
#include <float.h>

#define MAZE_SIZE 16
#define NUM_DIRECTIONS 4

typedef struct xy_coordinates coor; 

struct xy_coordinates {
    int x;
    int y;
    int dir;
    char dir_values[NUM_DIRECTIONS];
};

typedef enum direction direction;
enum direction {
    NORTH = 0,
    NORTHEAST = 4,
    EAST = 1,
    SOUTHEAST = 5,
    SOUTH = 2,
    SOUTHWEST = 6,
    WEST = 3,
    NORTHWEST = 7,
    ANY = -1
};

char DirToChar(direction dir){
    switch(dir){
        case NORTH:
            return 'n';
        case SOUTH:
            return 's';
        case EAST:
            return 'e';
        case WEST:
            return 'w';
        default:
            return 'z';
    }
}


typedef struct where_walls walls;
struct where_walls {
    bool north;
    bool northeast;
    bool east; 
    bool southeast;
    bool south;
    bool southwest;
    bool west;
    bool northwest;
    double value;
    double value_back;
    direction dir;
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

//Prints message out
void log_message(char* text) {
    fprintf(stderr, "%s\n", text);
    fflush(stderr);
}

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

//HEAP IMPLEMENTATION

typedef struct Heap_Node Hnode;
struct Heap_Node{
    int x;
    int y;
    double cost;
    direction dir;
    int speed;
};

typedef struct Heap_system Heap;
struct Heap_system{
    Hnode *data[MAZE_SIZE * MAZE_SIZE * 4];
    int size;
};

Heap *CreateHeap(){
    Heap *H = malloc(sizeof(Heap));
    H->size = 0;
    return H;
}

bool Heap_empty(Heap *H){
    return H->size <= 0;
}

void swap(Heap *H, int a, int b){
    Hnode *temp = H->data[a];
    H->data[a] = H->data[b];
    H->data[b] = temp;
}

void move_up(Heap *H, int index){
    if(index > H->size){
        log_message("Index error at move_up");
        return;
    }

    while(index > 1){
        if(H->data[index]->cost < H->data[index/2]->cost){
            swap(H, index, index/2);
            index = index/2;
        }
        else{
            break;
        }
    }
}

void move_down(Heap *H, int index){
    if(index < 1){
        log_message("Index error at move_down");
        return;
    }

    while(index * 2 <= H->size){
        int smaller_child = index * 2;

        if(H->size >= index * 2 + 1){
            if(H->data[index * 2]->cost > H->data[index * 2 + 1]->cost){
                smaller_child = index * 2 + 1;
            }
        }
        
        if(H->data[smaller_child]->cost < H->data[index]->cost){
            swap(H, smaller_child, index);
            index = smaller_child;
        }
        else{
            break;
        }
    }
}

void push_heap(Heap *H, Hnode *node){
    if(H->size >= MAZE_SIZE * MAZE_SIZE * 4){
        log_message("DATA FULL at push_heap");
        return;
    }
    H->size++;
    H->data[H->size] = node;

    move_up(H, H->size);
}

Hnode *Pop_heap(Heap *H){
    if(Heap_empty(H)){
        log_message("Empty Heap at Pop_heap");
        return NULL;
    }

    Hnode *result = H->data[1];
    H->data[1] = H->data[H->size];

    H->size--;
    move_down(H, 1);

    return result;
} 

Hnode *create_Hnode(int x, int y, double cost, direction dir, int speed){
    Hnode *Hn = malloc(sizeof(Hnode));
    Hn->x = x;
    Hn->y = y;
    Hn->cost = cost;
    Hn->dir = dir;
    Hn->speed = speed;
    return Hn;
}


int DirToInt(direction dir){
    return dir;
}


//NOT UPDATED WITH ENUM I DON'T REALLY EVER NEED THIS
char IntToDir(int dir){
    switch(dir){
        case 0:
            return 'n';
        case 1:
            return 'e';
        case 2:
            return 's';
        case 3:
            return 'w';
    }
    return 'z'; //just to return something
}

//modified left and right so they update dir
//up is 0, right is 1, down is 2, left is 3
//when mod by 4. Need to make sure to add pos multiples of 4 to make it 
//pos before mod
int turnLeft(coor *C){
    API_turnLeft();
    C->dir = (C->dir + NUM_DIRECTIONS - 1) % NUM_DIRECTIONS;
    return C->dir;
}

int turnRight(coor *C){
    API_turnRight();
    C->dir = (C->dir + 1) % NUM_DIRECTIONS;
    return C->dir;
}

//updates coordinates
void update_coor(coor *C){
    if(C->dir < 0){
        C->dir = ((abs(C->dir)/NUM_DIRECTIONS) + 1) * NUM_DIRECTIONS + C->dir;
    }
    C->dir = C->dir % NUM_DIRECTIONS;
    
    if(C->dir == NORTH){
        C->y += 1;
    }
    else if(C->dir == EAST){
        C->x += 1;
    }
    else if(C->dir == SOUTH){
        C->y -= 1;
    }
    else if(C->dir == WEST){
        C->x -= 1;
    }
}

//modified set wall so that I can keep track of the walls
void custom_setWall(int x, int y, direction dir, walls *maze[MAZE_SIZE][MAZE_SIZE]){

    API_setWall(x, y, DirToChar(dir));
    
    switch(dir){
        case NORTH:
            maze[x][y]->north = true;
            if(y < 15) maze[x][y+1]->south = true;
            break;
        case EAST:
            maze[x][y]->east = true;
            if(x < 15) maze[x+1][y]->west = true;
            break;
        case SOUTH:
            maze[x][y]->south = true;
            if(y > 0) maze[x][y-1]->north = true;
            break;
        case WEST:
            maze[x][y]->west = true;
            if(x > 0) maze[x-1][y]->east = true;
            break;
    }
}

//when see wall add wall;
void set_wall(coor *C, walls *maze[MAZE_SIZE][MAZE_SIZE]){
    if(API_wallLeft()){
        custom_setWall(C->x, C->y, (C->dir + 3) % NUM_DIRECTIONS, maze);
    }
    if(API_wallFront()){
        custom_setWall(C->x, C->y, C->dir, maze);
    }
    if(API_wallRight()){
        custom_setWall(C->x, C->y, (C->dir + 1) % NUM_DIRECTIONS, maze);
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
bool wavefront_helper(walls *maze[MAZE_SIZE][MAZE_SIZE], Hnode *first, Hnode *second, double acc[5], bool front){

    //If second one is lower or equal value to first stop

    double value;
    //Differentiaes between forward and backward
    if(front){
        value = maze[first->x][first->y] -> value;
    }
    else{
        value = maze[first->x][first->y] -> value_back;
    }

    //checks if going in same direction, if so increases speed
    if(first->dir == second->dir){
        second->speed = first->speed + 1;
    }
    else if(first->dir == ANY){ //inital point makes second node second tier acc
        second->speed = 1;
    }
    else { //different direction, add time for turn
        second->speed = 0;
        value += acc[4];
    }

    if(second->speed >= 3){ //reached max acceleration
        value += acc[3];
    }
    else value += acc[second->speed];

    second->cost = value;

    if(front){
        if(maze[second->x][second->y]->value == -1 ||
            maze[second->x][second->y]->value > value){
            
            maze[second->x][second->y]->value = value;
        }
        else return false;
    }
    else{
        if(maze[second->x][second->y]->value_back == -1 ||
            maze[second->x][second->y]->value_back > value){
            
            maze[second->x][second->y]->value_back = value;
        }
        else return false;
    }

    
    char str[10];

    snprintf(str, sizeof(str), "%.1f", value);
    if(front){
        API_setText(second->x, second->y, str);
    }
    return true;
}

void wavefront_helper_back(walls *maze[MAZE_SIZE][MAZE_SIZE], int x, int y, double value, Queue* Q){
    char str[10];

    snprintf(str, sizeof(str), "%.1f", value);
    //API_setText(x, y, str); // can comment out later
    maze[x][y]->value_back = value;

    enqueue(Q, x, y);
}

//Just runs the wavefront every time
//IN PROGRESS UPDATING
//WORKING ON MAKING IT MORE GENERIC, ENTER INITAL ONES INTO HEAP BEFORE USING 

//MAKE ALL THE VALUES -1 BESDIES GOAL BEFORE COMING IN
void update_wavefront(walls *maze[MAZE_SIZE][MAZE_SIZE], Heap *H, double acc[5], bool front){

    while(!Heap_empty(H)){
        Hnode *Hn = Pop_heap(H);
        int x = Hn->x;
        int y = Hn->y;

        //check all four directions for no wall
        walls* w = maze[x][y];
        if(!w->north && y < 15){
            Hnode *Hn_north = malloc(sizeof(Hnode));
            Hn_north -> x = x;
            Hn_north -> y = y + 1;
            Hn_north ->dir = NORTH;
            bool add = wavefront_helper(maze, Hn, Hn_north, acc, front);

            if(!add){
                free(Hn_north);
            }
            else push_heap(H, Hn_north);
        }

        if(!w->east && x < 15){
            Hnode *Hn_east = malloc(sizeof(Hnode));
            Hn_east ->x = x + 1;
            Hn_east ->y = y;
            Hn_east -> dir = EAST;
            bool add = wavefront_helper(maze, Hn, Hn_east, acc, front);
            
            if(!add){
                free(Hn_east);
            }
            else push_heap(H, Hn_east);
        }
        if(!w->south && y > 0){
            Hnode *Hn_south = malloc(sizeof(Hnode));
            Hn_south ->x = x;
            Hn_south -> y = y-1;
            Hn_south -> dir = SOUTH;
            bool add = wavefront_helper(maze, Hn, Hn_south, acc, front);
            
            if(!add){
                free(Hn_south);
            }
            else push_heap(H, Hn_south);
        }
        if(!w->west && x > 0){
            Hnode *Hn_west = malloc(sizeof(Hnode));
            Hn_west -> x = x - 1;
            Hn_west -> y = y;
            Hn_west ->dir = WEST;
            bool add = wavefront_helper(maze, Hn, Hn_west, acc, front);
            
            if(!add){
                free(Hn_west);
            }
            else push_heap(H, Hn_west);
        }
        free(Hn);
    }

    free(H);
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
        double value = DBL_MAX;
        
        if(forward){
            if(!w->north && y < 15 && maze[x][y+1]->value < w->value){
                //log_message("north is true");
                //y = y+1;
                if(maze[x][y+1]->value < value){
                    north = true;
                    value = maze[x][y+1]->value;
                }
            }
            if(!w->east && x < 15 && maze[x+1][y]->value < w->value){
                //log_message("east is true");
                //x = x+1;
                if(maze[x+1][y]->value < value){
                    east = true;
                    value = maze[x+1][y]->value;
                    north = false;
                }
                else if (maze[x+1][y]->value == value){
                    east = true;
                }
            }
            if(!w->south && y > 0 && maze[x][y-1]->value < w->value){
                //log_message("south is true");
                //y = y-1;
                if(maze[x][y-1]->value < value){
                    south = true;
                    value = maze[x][y-1]->value;
                    north = false;
                    east = false;
                }
                else if (maze[x][y-1]->value == value){
                    south = true;
                }
            }
            if(!w->west && x > 0 && maze[x-1][y]->value < w->value){
                if(maze[x-1][y]->value < value){
                    west = true;
                    value = maze[x-1][y]->value;
                    north = false;
                    east = false;
                    south = false;
                }
                else if(maze[x-1][y]->value == value){
                    west = true;
                }
            }
        }
        else{
            value = DBL_MAX;
            if(!w->north && y < 15 && maze[x][y+1]->value_back < w->value_back){
                //log_message("north is true");
                //y = y+1;
                if(maze[x][y+1]->value_back < value){
                    north = true;
                    value = maze[x][y+1]->value_back;
                }
            }
            if(!w->east && x < 15 && maze[x+1][y]->value_back < w->value_back){
                //log_message("east is true");
                //x = x+1;
                if(maze[x+1][y]->value_back < value){
                    east = true;
                    value = maze[x+1][y]->value_back;
                    north = false;
                }
                else if (maze[x+1][y]->value_back == value){
                    east = true;
                }
            }
            if(!w->south && y > 0 && maze[x][y-1]->value_back < w->value_back){
                //log_message("south is true");
                //y = y-1;
                if(maze[x][y-1]->value_back < value){
                    south = true;
                    value = maze[x][y-1]->value_back;
                    north = false;
                    east = false;
                }
                else if(maze[x][y-1]->value_back == value){
                    south = true;
                }
            }
            if(!w->west && x > 0 && maze[x-1][y]->value_back < w->value_back){
                if(maze[x-1][y]->value_back < value){
                    west = true;
                    value = maze[x-1][y]->value_back;
                    north = false;
                    east = false;
                    south = false;
                }
                else if(maze[x-1][y]->value_back == value){
                    west = true;
                }
            }
        }

        if(C->dir == NORTH){
            if(north) y++;
            else if (east) x++;
            else if(west) x--;
            else if(south) y--;
        }
        else if(C->dir == EAST){
            if(east) x++;
            else if(north) y++;
            else if(south) y--;
            else if(west) x--;
        }
        else if(C->dir == SOUTH){
            if(south) y--;
            else if(east) x++;
            else if(west) x--;
            else if (north) y++;
        }
        else if(C->dir == WEST){ 
            if(west) x--;
            else if(south) y--;
            else if(north) y++;
            else if (east)x++;
        }
        
        if(!east && !north && !west && !south){
            log_message("no possible direction found");
            break;
        }

        /*
        char log_temp[50]; // Make a buffer big enough to hold the whole message
        snprintf(log_temp, sizeof(log_temp), "New action, current coords are x: %d y: %d", x, y);
        log_message(log_temp);
        */
        enqueue(path, x, y);
        API_setColor(x,y,'G');

        if(maze[x][y]->value == 0 && forward){
            //log_message("Found");
            break;
        }
        if(maze[x][y]->value_back == 0 && !forward){
            //log_message("Found");
            break;
        }
    }

    return path;
}

void moveTo(coor *C, Qnode *next){
    int direction;
    if(C->y < next->y){ //needs to go north
        direction = NORTH;
    }
    else if(C->x < next->x){ //needs to go east
        direction = EAST;
    }
    else if(C->y > next->y){ //needs to go south
        direction = SOUTH;
    }
    else{ //needs to go west
        direction = WEST;
    }

    //how many times to turn right
    int turns = (direction - C->dir + NUM_DIRECTIONS) % NUM_DIRECTIONS; 
    switch(turns){
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


    //SETTING INITAL GOAL TEXT
    API_setText(7, 7, "0");
    API_setText(7, 8, "0");
    API_setText(8, 7, "0");
    API_setText(8, 8, "0");

    //set boundary walls
    for(int i = 0; i < MAZE_SIZE; i++){
        custom_setWall(i, 0, SOUTH, maze);
        custom_setWall(i, 15, NORTH, maze);
        custom_setWall(0, i, WEST, maze);
        custom_setWall(15, i, EAST, maze);
    }
    //log_message("Boundary Walls set");
    set_wall(c, maze);

    while (maze[c->x][c->y]->value != 0) {
        char log_temp[50]; // Make a buffer big enough to hold the whole message
        snprintf(log_temp, sizeof(log_temp), "New action, current coords are x: %d y: %d", c->x, c->y);
        //log_message(log_temp);
        set_wall(c, maze);
        

        //SETS The inital value of the maze
        for(int i = 0; i< MAZE_SIZE; i++){
            for(int j = 0; j < MAZE_SIZE; j++){
                maze[i][j]->value = -1;
            }
        }

        //Sets the inital value of the goal
        //change if you want to change goal
        maze[7][7]->value = 0;
        maze[8][7]->value = 0;
        maze[7][8]->value = 0;
        maze[8][8]->value = 0;

        //Creates the heap that the wavefront works off of
        //Change inputs based on differing goals
        Heap *H = CreateHeap();
        //-1 for direction means that its the goal and can start in any direction
        Hnode *Hn1 = create_Hnode(7, 7, 0, ANY, 1);
        Hnode *Hn2 = create_Hnode(7, 8, 0, ANY, 1);
        Hnode *Hn3 = create_Hnode(8, 7, 0, ANY, 1);
        Hnode *Hn4 = create_Hnode(8, 8, 0, ANY, 1);
        push_heap(H, Hn1);
        push_heap(H, Hn2);
        push_heap(H, Hn3);
        push_heap(H, Hn4);

        //Last one in acc is for turn cost
        double acceleration[5] = {1, 1, 1, 1, 0.5};
        update_wavefront(maze, H, acceleration, true);
     

        
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

        for(int i = 0; i< MAZE_SIZE; i++){
            for(int j = 0; j < MAZE_SIZE; j++){
                maze[i][j]->value = -1;
                maze[i][j]->value_back = -1;
            }
        }

        maze[7][7]->value = 0;
        maze[8][7]->value = 0;
        maze[7][8]->value = 0;
        maze[8][8]->value = 0;

        maze[0][0]->value_back = 0;

        Heap *H = CreateHeap();
        //-1 for direction means that its the goal and can start in any direction
        Hnode *Hn1 = create_Hnode(7, 7, 0, ANY, 1);
        Hnode *Hn2 = create_Hnode(7, 8, 0, ANY, 1);
        Hnode *Hn3 = create_Hnode(8, 7, 0, ANY, 1);
        Hnode *Hn4 = create_Hnode(8, 8, 0, ANY, 1);
        push_heap(H, Hn1);
        push_heap(H, Hn2);
        push_heap(H, Hn3);
        push_heap(H, Hn4);

        //Last one in acc is for turn cost
        double acceleration[5] = {1, 1, 1, 1, 0.5};
        update_wavefront(maze, H, acceleration, true);
      

        Heap *H_back = CreateHeap();
        
        Hnode *Hn1_back = create_Hnode(0, 0, 0, NORTH, 1);
        push_heap(H_back, Hn1_back);

        update_wavefront(maze, H_back, acceleration, false);
        

        Queue* path = find_path(c, maze, false);

        Qnode* next = dequeue(path);
        moveTo(c, next);
        update_coor(c);
        free(next);
        clear_path(path);
        clear_color();
    }


    //WORK ON DEBUG THIS ERROR
    //update_walltext(maze);
    Queue* path = find_path(c, maze, true);
    while(!queue_empty(path)){
        Qnode* n = dequeue(path);
        free(n);
    }
    free(path);

    
    for(int i = 0; i<MAZE_SIZE; i++){
        for(int j= 0; j<MAZE_SIZE; j++){
            free(maze[i][j]);
        }
    }
}

//NOTE
/* JUST FINISHED UPDATE_WAVEFRONT SHOULD BE USED FOR FRONT AND BACK
*/