/*
Some numbers:
Max ActorID: 1971696
NRows: 17316773-1
Max MovieID: 1151758
*/

#include <iostream>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <thread>

using namespace std;


// BAD CODING!!! <3
int *actor_keys = new int[1971696];
int *act2mov_actors = new int[1971696];
int *act2mov_movies = new int[17316773-1];
int *mov2act_movies = new int[1151758];
int *mov2act_actors = new int[17316773-1]();

// Breadth-First Search
int BFS(
        int *act2mov_actors,
        int *act2mov_movies,
        int *mov2act_movies,
        int *mov2act_actors,
        int actorid2, 
        vector<int> current_nodes,
        bool *actor_visited,
        bool *movie_visited
    ) {
    /*
    Iterate over the current nodes.
    For each actor find all movies.
    If we visited the movie already skip it, otherwise find all actors in that movie.
    Add the actors we did not look at already to the neighbourhood.
    Swap neighbours and current_nodes and start again.
    */
    
    vector<int> neighbours;
    int depth = 0;

    while(!current_nodes.empty()){
        depth++;
        for(int i : current_nodes) {
            for(int j = act2mov_actors[i-1]; j < act2mov_actors[i]; j++) {
                int movie = act2mov_movies[j];
                if(!movie_visited[movie]){
                    movie_visited[movie] = 1;
                    for(int k=mov2act_movies[movie-1]; k<mov2act_movies[movie]; k++){
                        int new_actor = mov2act_actors[k];
                        if(!actor_visited[new_actor]) {
                            if(new_actor==actorid2){
                                return depth;
                            }
                            actor_visited[new_actor] = 1;
                            neighbours.push_back(new_actor);
                        }
                    }
                }
            }
        }

    current_nodes.swap(neighbours);
    neighbours.clear();
    }

    return -1;
}

void BFSThread(int thread_a1, int thread_a2, int *dist_thread, int i){
    
    if(thread_a1 == thread_a2){
            dist_thread[i] = 0;
            return;
    }

    // Boolean arrays to save which has been visited
    bool *actor_visited = new bool[1971696+1]();
    bool *movie_visited = new bool[1151758+1]();
    
    // We start with only actor 1 on the current node
    vector<int> current_nodes;
    current_nodes.push_back(actor_keys[thread_a1]);
    // Start Breadth-First-Search
    int dist;
    dist = BFS(
        act2mov_actors, act2mov_movies, 
        mov2act_movies, mov2act_actors,
        actor_keys[thread_a2], current_nodes, actor_visited, movie_visited);
    // cout << "Process: " << i << " Distance: " << dist << endl;
    dist_thread[i] = dist;

    // delete unsused variable
    delete[] actor_visited;
    delete[] movie_visited;
}

int main(int argc, char** argv) {

    // Movie to actor map - Will be replaced later
    vector<vector<int>> M(1151758+1);

    // Open file and figre out length
    int handle = open(argv[1],O_RDONLY);
    if (handle<0) return 1;
    lseek(handle,0,SEEK_END);
    long length = lseek(handle,0,SEEK_CUR);
    
    // Map file into address space
    auto data = static_cast<const char*>(mmap(nullptr,length,PROT_READ,MAP_SHARED,handle,0));
    auto dataLimit = data + length;
    
    /* Read file and create our datatypes
    We store the actor to movie relation in a CSR and movie to actor in a map
    */
    const char* line = data;
    int actor_index = -1;
    int current_line = 0;
    int last_actor = 0;
    // Dummy loop to delete first line
    const char* current;
    for (current=data;current!=dataLimit;++current) {
        char c=*current;
        if (c=='\n') {
            ++current;
            break;
        }
    }
    for (;current!=dataLimit;) {
        unsigned column=0;
        int actor=0;
        int movie=0;
        for (;current!=dataLimit;++current) {
            char c=*current;
            if (c==',') {
                    ++column;
            }else if (c=='\n') {
                /* Check if the actor is different to the last one
                If yes increase actor_index and add entry to actor_keys */
                if(actor != last_actor){
                    last_actor = actor;
                    act2mov_actors[actor_index] = current_line;
                    ++actor_index;
                    actor_keys[actor] = actor_index;
                }

                // Insert entry into Movie->Actor Map
                M[movie].push_back(actor_index);
                
                // Insert movie to list
                act2mov_movies[current_line] = movie;
                
                // Update index
                ++current_line;
                
                ++current;
                break;
            }else if (column==0) {
                    actor=10*actor+c-'0';
            }else if (column==1) {
                    movie=10*movie+c-'0';
            }
        }
    }
    act2mov_actors[actor_index] = current_line;

    // cout << "File eingelesen" << endl;

    // Create CSR for movie to actor relation
    int iterator = 0;
    for(int movie_id=1; movie_id<=1151758; movie_id++){
        for(int actor : M.at(movie_id)){
            mov2act_actors[iterator] = actor;
	       ++iterator;
        }
        mov2act_movies[movie_id] = iterator;
    }

    // While there is an input: read, store, compute
    int actorid1;
    int actorid2;
    vector<int> actor1;
    vector<int> actor2;
    
    while((cin >> actorid1) && (cin >> actorid2)) {
        actor1.push_back(actorid1);
        actor2.push_back(actorid2);
    }

    
    int inputlen = actor1.size();
    int *distance = new int[inputlen];
    thread *thread_arr = new thread[inputlen];
    for(int time_counter = 0; time_counter<1; ++time_counter){
        for(int i=0; i < inputlen; i++){
            thread_arr[i] = thread(BFSThread, actor1[i], actor2[i], distance, i);
        }
        // cout << "Threading started" << endl;
        for(int i=0; i < inputlen; i++){
            thread_arr[i].join();
        }
    }
    
    for(int j=0; j<inputlen; j++){
        cout << distance[j] << endl;
    }

    return 0;
}
