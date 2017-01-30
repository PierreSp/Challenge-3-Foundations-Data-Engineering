// Time echo -e "1 1\n1 2\n2 3\n3 4\n4 5" |./bacon playedin.csv
// time echo -e "1 1\n1 2" |./bacon playedin.csv

/*
Some numbers:
Max ActorID: 1971696
NRows: 17316773-1
Max MovieID: 1151758
*/

#include <iostream>
#include <ios>
#include <fstream>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <list>
#include <thread>
//#include <mutex>

#include <chrono>		// should allow high precision timing

using namespace std;

//static std::mutex barrier;

// BAD CODING!!! <3
//
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


    // Now we want to find all neighbours of each of the current nodes
    vector<int> neighbours;
    
    // For all current actors
    int depth = 0;
    while(!current_nodes.empty()){
        depth++;
        for(int i : current_nodes) {
            // Get all movies
            for(int j = act2mov_actors[i-1]; j < act2mov_actors[i]; j++) {
                int movie = act2mov_movies[j];
                // For each movie find all actors
                if(!movie_visited[movie]){
                    movie_visited[movie] = 1;
                    for(int k=mov2act_movies[movie-1]; k<mov2act_movies[movie]; k++){
                        int new_actor = mov2act_actors[k];
                        // If he has not been inspected yet add him to neighbours
                        if(!actor_visited[new_actor]) {
                            // If it is the actor2 we are looking for return 1 as distance
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

    bool *actor_visited = new bool[1971696+1]();
    bool *movie_visited = new bool[1151758+1]();
    // Boolean to save if actor i has been visited or not
    // Nodes are the ones we are visiting right now - We'll want to find their neighbours with each iteration of BFS
    vector<int> current_nodes;
    // We start with only actorid1
    current_nodes.push_back(actor_keys[thread_a1]);
    int dist;
    // Start Breadth-First-Search
    dist = BFS(
        act2mov_actors, act2mov_movies, 
        mov2act_movies, mov2act_actors,
        actor_keys[thread_a2], current_nodes, actor_visited, movie_visited);
    // Write on global dist variable 
    // std::lock_guard<std::mutex> block_threads_until_finish_this_job(barrier);
    cout << "Process: " << i << " Distance: " << dist << endl;
    dist_thread[i] = dist;

    // delete unsused variable
    delete[] actor_visited;
    delete[] movie_visited;
}

int main(int argc, char** argv) {

  // proper timing of actual code execution
  auto start_time = chrono::high_resolution_clock::now();
  
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
    int m1_current = 0;
    int last_actor = 0;
    for (const char* current=data;current!=dataLimit;) {
        const char* last=line;
        unsigned column=0;
        int actor=0;
        int movie=0;
        for (;current!=dataLimit;++current) {
            char c=*current;
            if (c==',') {
                    last=current+1;
                    ++column;
            }else if (c=='\n') {
                // Insert entry into Movie->Actor Map
                // M[movie].push_back(actor);

                /* Check if the actor is different to the last one
                If yes increase actor_index and add entry to actor_keys */
                if(actor != last_actor){
                    ++actor_index;
                    actor_keys[actor] = actor_index;
                }
                M[movie].push_back(actor_index);

                act2mov_actors[actor_index] = m1_current+1;
                // Insert movie to list
                act2mov_movies[m1_current] = movie;
                // Update index
                ++m1_current;

                last_actor = actor;
                ++current;
                break;
            }else if (column==0) {
                    actor=10*actor+c-'0';
            }else if (column==1) {
                    movie=10*movie+c-'0';
            }
        }
    }

    cout << "File eingelesen" << endl;

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
        cout << "Threading started" << endl;
        for(int i=0; i < inputlen; i++){
            thread_arr[i].join();
        }
    }
    
    // timing
    auto end_time = chrono::high_resolution_clock::now();

    auto passed_usecs = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
    double elapsed_u = (double) passed_usecs.count();
    double elapsed = elapsed_u / (1000.0 * 1000.0);
    // cout << "Passed time: " << passed_usecs.count() << " microseconds" << endl << endl;
    cout << endl << "Passed time: " << elapsed << " seconds" << endl << endl;
    
    for(int j=0; j<inputlen; j++){
        cout << distance[j] << endl;
    }
    return 0;
}
