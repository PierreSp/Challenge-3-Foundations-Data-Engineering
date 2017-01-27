// Bacon Number
// g++ -O3 -Wall -std=c++14 ./dist.c++ 
// ./a.out playedin.csv
// time echo -e "1 1\n1 2\n2 3\n3 4\n4 5" |./bacon playedin.csv
// time echo -e "1 1\n1 2" |./bacon playedin.csv

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

using namespace std;

//rekursive Funktion Breadth-First-Search
int BFS(
        int *actor_keys,
        int *a1,
        int *m1,
        // const unordered_map<size_t, unordered_set<size_t>>& A, 
        const unordered_map<size_t, unordered_set<size_t>>& M, 
        size_t actorid2, 
        unordered_set<size_t> current_nodes,
        bool *visited
    ) {
    // If BFS has no starting nodes return -1
    if(current_nodes.empty()) {
        return -1;
    }
    // If actorid2 is in the current nodes return distance 0
    if(current_nodes.count(actorid2)>0) {
        return 0;
    }

    // Now we want to find all neighbours of each of the current nodes
    unordered_set<size_t> neighbours;
    // bool *neighbours = new bool[1971696];
    
    // For all current actors
    for(size_t i : current_nodes) {
        // Get all movies
        for(size_t j = a1[actor_keys[i]-1]; j < a1[actor_keys[i]]; j++) {
            int movie = m1[j];
            // For each movie find all actors
            for(size_t k : M.at(movie)) {
                // If he has not been inspected yet do so
                if(!visited[k]) {
                    visited[k] = 1;
                    neighbours.insert(k);
                }  
            }
        }
    }
    // Now perform BFS on the neighbours we just found
    int count = BFS(actor_keys, a1, m1, M, actorid2, neighbours, visited);
    
    // If BFS returns -1 we pass that forward
    if(count == -1) {
        return -1;
    }
    // If BFS returns a distance we have to increase that distance by 1
    return ++count;
}

int main(int argc, char** argv) {
    int *actor_keys = new int[1971696];
    int *a1 = new int[1971696];
    int *m1 = new int[17316773-1];
    // 1731673-1
    // a[1] = 2;

    /*
    I want to look at the movies actor i played in
    best case: a[i-1] to a[i] gives me the indexes to look them up in m
    but really: i = actor_keys[i]
    */

    // Actor to movie map
    unordered_map<size_t, unordered_set<size_t>> A;
    // Movie to actor map
    unordered_map<size_t, unordered_set<size_t>> M;

    // Open file and figre out length
    int handle=open(argv[1],O_RDONLY);
    if (handle<0) return 1;
    lseek(handle,0,SEEK_END);
    long length=lseek(handle,0,SEEK_CUR);
    
    // Map file into address space
    auto data=static_cast<const char*>(mmap(nullptr,length,PROT_READ,MAP_SHARED,handle,0));
    auto dataLimit=data+length;
    
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
                // Insert the read actor and movie in both hashmaps
                // A[actor].insert(movie);
                M[movie].insert(actor);

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

    // Movie Map to lists here

    // While there is an input: read, store, compute
    size_t actorid1;
    size_t actorid2;
    while((cin >> actorid1) && (cin >> actorid2)) {
        //cout << "Berechne " << actorid1 << " " << actorid2 << endl;

        // Boolean to save if actor i has been visited or not
        bool *visited = new bool[1971696];
        // Nodes are the ones we are visiting right now - We'll want to find their neighbours with each iteration of BFS
        unordered_set<size_t> current_nodes;
        // We start with only actorid1
        current_nodes.insert(actorid1);

        // Start Breadth-First-Search
        int dist = BFS(
            actor_keys, a1, m1, 
            M, actorid2, current_nodes, visited);

        cout << dist << endl;
    }
    return 0;
}