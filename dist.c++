// Bacon Number
// g++ -O3 -Wall -std=c++14 ./dist.c++ 
// ./a.out playedin.csv


#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <set>


using namespace std;



//rekursive Funktion Breadth-First-Search
int BFS(
    const unordered_map<size_t, unordered_set<size_t>>& A, 
    const unordered_map<size_t, unordered_set<size_t>>& M, 
    size_t actorid2, 
    unordered_set<size_t> N,
    unordered_set<size_t>& B
    ) {
    if(N.empty()) { //Ist die Nachbarschaft leer, kann keine Verbindung zwischen zwei Actors hergestellt werden, es wird -1 zurück gegeben.
        return -1;
    }
    if(N.count(actorid2)>0) {
        return 0; //Wir haben ActorID2 gefunden!
    }
    unordered_set<size_t> N2;
    // for(size_t l : N) {
    //     B.insert(l); //aktualisieren der bereits besuchten Knoten
    // }
    for(size_t i : N) { //für jeden Knoten (Actor) in der Nachbarschaft,
        for(size_t j : A.at(i)) { //berechnen wir mit der Hashmap die zugehörigen Movies und wählen einen festen Movie aus,
            for(size_t k : M.at(j)) { //für diesen Movie suchen wir mit der Hashmap die dort mitwirkenden Actoren und legen wieder einen Actor konkret fest, um
                if(B.count(k)==0) { //zu überprüfen ob er bereits besucht wurde, ist dies nicht der Fall, 
                    B.insert(k);
                    N2.insert(k); //wird er zur Nachbarschaft hinzugefügt und im nächsten Schritt der BFS berücksichtigt.
                }  
            }
        }
    }
    int count = BFS(A, M, actorid2, N2, B); //rekurisver Aufruf der BFS mit angepassster Nachbarschaft und angepassten besuchten Knoten.
    if(count == -1) {
        return -1; //sollte -1 zurück gegeben werden, muss das gesondert ausgegeben werden.
    }
    return 1 + count; //ansonsten geben wir die Anzahl der BFS-Durchläufe zurück.
}

int main(int argc, char** argv) {
    unordered_map<size_t, unordered_set<size_t>> A; 
    unordered_map<size_t, unordered_set<size_t>> AA;
    unordered_map<size_t, unordered_set<size_t>> M;
    std::vector<size_t> set_actors;

    //A.reserve(2000000);
    set_actors.reserve(2000000);
    M.reserve(1400000);
    // Open file and figre out length
    int handle=open(argv[1],O_RDONLY);
    if (handle<0) return 1;
    lseek(handle,0,SEEK_END);
    long length=lseek(handle,0,SEEK_CUR);
    
    // Map file into address space
    auto data=static_cast<const char*>(mmap(nullptr,length,PROT_READ,MAP_SHARED,handle,0));
    auto dataLimit=data+length;
    
    const char* line=data;
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
            }else if (column==0) {
                    actor=10*actor+c-'0';
            }else if (c=='\n') {
                //A[actor].insert(movie); //Actor und Movie werden in unsere Hashmap eingefügt.
            //M[movie].insert(actor);
                set_actors.push_back(actor);
                set_actors.push_back(movie);

                ++current;
                break;
            }else if (column==1) {
                    movie=10*movie+c-'0';
            }
        }
    }
exit(0);
    for(size_t j : set_actors){ 
        //cout << j << endl;
        for(size_t k : M.at(j)) {
            AA[j].insert(k);
        }
    }

    cout << "File eingelesen2 " <<  A.count(1)<< endl;

    while(true) { //Input von User wird gelesen und als ActorID1 und ActorID2 verwendet.
        size_t actorid1;
        size_t actorid2;
        if (!(cin >> actorid1 )|| !(cin >> actorid2)){
            break;
        }
      
        //cout << "Berechne " << actorid1 << " " << actorid2 << endl;
        
        unordered_set<size_t> B; //in B wollen wir alle bereits besuchten Actor speichern, B ist zu Beginn leer.
        unordered_set<size_t> N; //N ist die Nachbarschaft, also die Knoten, von denen aus als nächstes gesucht wird.
        N.insert(actorid1); //Zu Beginn ist nur ActorID1 in der Nachbarschaft.

        int dist = BFS(A, M, actorid2, N, B); //Mit diesen Parametern wird die BFS aufgerufen.

        cout << dist << endl;
    }
    return 0;
}