// Bacon Number
// g++ -O3 -Wall -std=c++14 ./dist.c++ 
// ./a.out playedin.csv
// time echo -e "1 1\n1 2\n2 3\n3 4\n4 5" |./bacon playedin.csv
// time echo -e "1 1\n1 2" |./bacon playedin.csv


/*
Some numbers:
Max ActorID: 1971696
NRows: 17316773-1
Max MovieID: 1151758
*/


#include <iostream>
#include <fstream>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <list>
#include <thread>
//#include <mutex>

using namespace std;

//static std::mutex barrier;

// BAD CODING!!! <3
int *actor_keys = new int[1971696];
int *a1 = new int[1971696];
int *m1 = new int[17316773-1];
int *movie_to = new int[1151758];
int *to_actors = new int[17316773-1]();

// Breadth-First Search
int BFS(
        int *actor_keys,
        int *a1,
        int *m1,
        int *movie_to,
        int *to_actors,
        size_t actorid2, 
        list<size_t> current_nodes,
        bool *visited
    ) {

    // If BFS is called on an empty list of nodes return -1
    if(current_nodes.empty()) {
        return -1;
    }

    // Now we want to find all neighbours of each of the current nodes
    list<size_t> neighbours;
    
    // For all current actors
    for(size_t i : current_nodes) {
        // Get all movies
        for(size_t j = a1[actor_keys[i]-1]; j < a1[actor_keys[i]]; j++) {
            int movie = m1[j];
            // For each movie find all actors
            for(size_t k=movie_to[movie-1]; k<movie_to[movie]; k++){
                size_t new_actor = to_actors[k];
                // If he has not been inspected yet add him to neighbors
                if(!visited[new_actor]) {
                    // If it is the actor2 we are looking for return 1 as distance
                    if(new_actor==actorid2){
                        return 1;
                    }
                    visited[new_actor] = 1;
                    neighbours.push_back(new_actor);
                }
            }
        }
    }

    // Now perform BFS on the neighbours we just found
    int count = BFS(
        actor_keys, a1, m1,
        movie_to, to_actors,
        actorid2, neighbours, visited);
    
    // If BFS returns -1 we pass that forward
    if(count == -1) {
        return -1;
    }
    // If BFS returns a distance we have to increase that distance by 1
    return ++count;
}

void BFSThread(size_t thread_a1, size_t thread_a2, int *dist_thread, size_t i){
    bool *visited = new bool[1971696]();
    // Boolean to save if actor i has been visited or not
    // Nodes are the ones we are visiting right now - We'll want to find their neighbours with each iteration of BFS
    // unordered_set<size_t> current_nodes;
    list<size_t> current_nodes;
    // We start with only actorid1
    current_nodes.push_back(thread_a1);
    int dist;
    // Start Breadth-First-Search
    dist = BFS(
        actor_keys, a1, m1, 
        movie_to, to_actors,
        thread_a2, current_nodes, visited);
    // Write on global dist variable 
    // std::lock_guard<std::mutex> block_threads_until_finish_this_job(barrier);
    cout << "Process: " << i << " Distance: " << dist << endl;
    dist_thread[i] = dist;
}

int main(int argc, char** argv) {

    // Movie to actor map
    vector<vector<size_t>> M(1151758+1);

    // Open file and figre out length
    int handle=open(argv[1],O_RDONLY);
    if (handle<0) return 1;
    lseek(handle,0,SEEK_END);
    long length=lseek(handle,0,SEEK_CUR);
    
    // Map file into address space
    auto data=static_cast<const char*>(mmap(nullptr,length,PROT_READ,MAP_SHARED,handle,0));
    auto dataLimit=data+length;
    
    // Read file and create our datatypes
    const char* line=data;
    int actor_read_nr = 1;
    int m1_current = 0;
    int last_actor = 0;
    for (const char* current=data;current!=dataLimit;) {
        const char* last=line;
        unsigned column=0;
        size_t actor=0;
        size_t movie=0;
        for (;current!=dataLimit;++current) {
            char c=*current;
            if (c==',') {
                    last=current+1;
                    ++column;
            }else if (c=='\n') {
                M[movie].push_back(actor);

                // If the actor is different to the last one
                // Increase actor_read_nr to write into a new index
                // Add new entry to id_to_index map
                if(actor != last_actor){
                    ++actor_read_nr;
                    actor_keys[actor] = actor_read_nr;
                }

                a1[actor_read_nr] = m1_current+1;
                // Insert movie to list
                m1[m1_current] = movie;
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
    // return 0;

    // Movie to actor:
    int iterator = 0;
    for(size_t movie_id=1; movie_id<=1151758; movie_id++){
        for(size_t actor : M.at(movie_id)){
            to_actors[iterator] = actor;
            movie_to[movie_id] = ++iterator;
        }
    }
    cout << "Created Movie to Actor!" << endl;
    // return 0;

    // While there is an input: read, store, compute
    size_t actorid1;
    size_t actorid2;
    vector<size_t> actor1;
    vector<size_t> actor2;
    while((cin >> actorid1) && (cin >> actorid2)) {
        if(actorid1 == actorid2){
            cout << 0 << endl;
            continue;
        }
        actor1.push_back(actorid1);
        actor2.push_back(actorid2);
    }

    size_t inputlen = actor1.size();
    int *distance = new int[inputlen];
    thread *thread_arr = new thread[inputlen];

    for(size_t i=0; i < inputlen; i++){
        thread_arr[i] = thread(BFSThread, actor1[i], actor2[i], distance, i);
    }
    cout << "Threading started" << endl;
    for(size_t i=0; i < inputlen; i++){
        thread_arr[i].join();
    }

    for(size_t j=0; j<inputlen; j++){
        cout << distance[j] << endl;
    }
    return 0;
}