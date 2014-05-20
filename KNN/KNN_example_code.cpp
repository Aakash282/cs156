/*
 * Copyright (C) 2008, Kadence, http://www.trahald.com
 * You may use this source code as you wish as long as this copyright notice is retained, unmodified
*/

#include <iostream>
#include <probe.h>
#include <user.h>
#include <vector>
#include <valarray>
#include <math.h>
#include <map>
#include <qfile.h>
#include <fstream>
#include <ctime>

#ifdef Q_OS_WIN
#include <winmmap.h>
#else
#include <sys/mman.h>
#endif

using namespace std;

bool MMAP_PEARSON = true;
bool MMAP_COUNT = true;
int MAX_NEIGHBORS = 30;
int MIN_COMMON_VIEWERS = 24;
int MIN_SEEN_NEIGHBORS = 2;
bool POSITIVE_ONLY = false;
const char* pearsonFile = "pearson.data";
const char* countFile = "count.data";

float *pearsonMap;
int *countMap;
int totpreds = 0;
int backups = 0;
int errors = 0;

struct simStats{
    vector<uint> simVotesI;
    vector<uint> simVotesJ;
};

struct nStruct{
    int movie;
    int count;
    float rating;
    float pearson;
    float similarity;
    //    Sort descending order by similarity
    bool operator<(const nStruct& rhs) const {
        float fixed = similarity;
        float rhsfixed = rhs.similarity;
        if(isnan(fixed) | isinf(fixed)) fixed = 0;
        if(isnan(rhsfixed) | isinf(rhsfixed)) rhsfixed = 0;
        return fixed > rhsfixed;
    }
};

float pearson(vector<uint> vi, vector<uint> vj){
    //    input vectors must be the same size
    if(vi.size()!=vj.size()) return false;
    int size = vi.size();
    //    Convert to valarrays for calculation speed
    valarray<float> vI(size);
    valarray<float> vJ(size);
    copy(vi.begin(), vi.end(), &vI[0]);
    copy(vj.begin(), vj.end(), &vJ[0]);
    float meanI = vI.sum() / size;
    float meanJ = vJ.sum() / size;
    float stdevI = sqrt( pow((vI-meanI), 2).sum() / (size-1));
    float stdevJ = sqrt( pow((vJ-meanJ), 2).sum() / (size-1));
    float pearson = ( ((vI-meanI)/stdevI) * ((vJ-meanJ)/stdevJ) ).sum() / (size-1);
    if(isnan(pearson) | isinf(pearson)) pearson = 0;
    if(pearson>1) pearson = 1.0;
    if(pearson<-1) pearson = -1.0;
    return pearson;
}

float fisher(float r, int n, float zscore, bool isLower=true){
    float z = .5 * log( (1+r) / (1-r));
    float diff = zscore / sqrt(n-3);
    if(isLower) diff = diff * -1;
    return z + diff;
}

float fisher_inv(float z){
    return (exp(2*z) - 1) / (exp(2*z) + 1);
}

float calcSim(float pearson, int count){
    //    A pearson 1 1 will give an error in fisher()
    if(pearson>=.999) pearson=.999;
    float fisher_lower;
    if(pearson>0) fisher_lower = fisher_inv(fisher(pearson, count, 1.96));
    else fisher_lower = fisher_inv(fisher(pearson, count, -1.96));
    //    Check to see if the sign of pearson was changed; if so, set to 0
    if(pearson>=0 && fisher_lower<0) fisher_lower = 0;
    if(pearson<=0 && fisher_lower>0) fisher_lower = 0;
    return fisher_lower * fisher_lower / (1 - fisher_lower * fisher_lower) * log(count);
}

float get_float(ifstream &in, int num){
    &in.seekg(num*sizeof(int), ios_base::beg);
    float value;
    &in.read((char*)&value, sizeof(float));
    return value;
}

int get_int(ifstream &in, int num){
    &in.seekg(num*sizeof(int), ios_base::beg);
    int value;
    &in.read((char*)&value, sizeof(int));
    return value;
}

class KNN : public Algorithm
{
public:
    KNN(DataBase *db, int n) : Algorithm(), currentMovie(db), currentUser(db), doPearson(false), doCount(false), totalMovies(n), totalUsers(db->totalUsers())
    {}

    void setMovie(int id){
        currentMovie.setId(id);
    }

    //    Note: movieAverages and userAverages must be set before running this
    double determine(int user){
        totpreds++;
        int movieId = currentMovie.id();
        User currentUser(currentMovie.dataBase());
        currentUser.setId(user);
        int totalUserVotes = currentUser.votes();
//        fprintf(stderr, "MovieId: %d\tUserId: %d\n", movieId, user);
        ifstream pearsonIn(pearsonFile, ios::binary);
        ifstream countIn(countFile, ios::binary);
        vector<nStruct> v;
        //    For each movie seen, add the neighbor
        for(int j=0; j<totalUserVotes; j++){
            int neighborid = currentUser.movie(j);
            if(movieId!=neighborid){
                int score = currentUser.score(j);
                int itemNum;
                //    If movieId>neighborid, use the mirror matrix position. Adjust itemNum according to Gaussian sequence.
                if(movieId<=neighborid) itemNum = (movieId-1)*totalMovies + (neighborid - 1) - (movieId-1)*movieId/2;
                else itemNum = (neighborid-1)*totalMovies + (movieId - 1) - (neighborid-1)*neighborid/2;
                nStruct n;
                n.movie = neighborid;
                if(MMAP_COUNT) n.count = countMap[itemNum];
                else n.count = get_int(countIn, itemNum);
                //    Don't proceed with this neighbor if MIN_COMMON_VIEWERS not met
                if(n.count>MIN_COMMON_VIEWERS){
                    if(MMAP_PEARSON) n.pearson = pearsonMap[itemNum];
                    else n.pearson = get_float(pearsonIn, itemNum);
                    //    Don't proceed if POSITIVE_ONLY==true and the pearson is negative
                    if(n.pearson>0 | POSITIVE_ONLY!=true){
                        n.rating = score;
                        n.similarity = calcSim(n.pearson, n.count);
                        v.push_back(n);
                    }
                }
            }
        }
        pearsonIn.close();
        countIn.close();
        int size = (int) v.size();
        //    Backup algorithm triggered by MIN_SEEN_NEIGHBORS
        if(size<MIN_SEEN_NEIGHBORS){
            backups++;
            float prediction = .5*movieAverages[movieId] + .5*userAverages[user];
            return prediction;
        }
        //    Sort the neighbors in descending order
        partial_sort(v.begin(), v.begin()+min(size, MAX_NEIGHBORS), v.end());
        float weightSum = 0;
        float combinationSum = 0;
        //    Iterate over MAX_NEIGHBORS
        for(int i=0; i<min(size, MAX_NEIGHBORS); i++){
            nStruct n = v.at(i);
            int neighborid = n.movie;
            float pearson = n.pearson;
            int count = n.count;
            float similarity = n.similarity;
            weightSum += similarity;
            float delta = n.rating - movieAverages[neighborid];
            float score;
            //    Whether to add or subtract the delta is based on the sign of the correlation
            if(pearson>0) score = movieAverages[movieId] + delta;
            else score = movieAverages[movieId] - delta;
            combinationSum += similarity*score;
//            fprintf(stderr, "Pred: %d\tMovieID: %d\tUserID: %d\tNeighborID: %d.\n", totpreds, movieId, user, neighborid);
//            fprintf(stderr, "i: %d NeighborID: %d Pearson: %f Count: %d Similarity: %f Delta: %f Score: %f CombinationSum: %f WeightSum: %f\n", i, neighborid, pearson, count, similarity, delta, score, combinationSum, weightSum);
        }
        float prediction = combinationSum / weightSum;
        //    Backup algorithm triggered by nan or inf values
        //    This can happen if all movies have similarity 0, which can happen if the similar count is low thus the Pearson and inverse(Fisher Lower) signs don't match
        if(isnan(prediction) | isinf(prediction)){
            backups++;
            return .5*movieAverages[movieId] + .5*userAverages[user];
        }
        //    Bounds checking
        if(prediction<1) prediction = 1;
        if(prediction>5) prediction = 5;
        return prediction;
    }
    
    void setMovieAverages(){
        for(int i=1; i<=totalMovies; i++){
            double average = 0;
            currentMovie.setId(i);
            uint numVotes = currentMovie.votes();
            for(uint j=0; j<numVotes; j++){
                average += currentMovie.score(j);
            }
            average = average / (double) numVotes;
            movieAverages[i] = average;
        }
    }
    
    void setUserAverages(){
        int totalUsers = currentMovie.dataBase()->totalUsers();
        currentUser.setId(6);
        for (int i = 0; i < totalUsers; ++i){
            double average = 0;
            uint numVotes = currentUser.votes();
            for(uint j=0; j<numVotes; j++){
                average += currentUser.score(j);
            }
            average = average / (double) numVotes;
            userAverages[currentUser.id()] = average;
            currentUser.next();
        }
    }
    
    void generatePearson(ofstream &pearsonOut, ofstream &countOut){
        User user(currentMovie.dataBase());
        int genstart = time(NULL);
        //    movie ID's are from 1 to number of movies
        for(int i=1; i<=totalMovies; ++i){
            currentMovie.setId(i);
            //    store stats in array, indexes 1 to totalMovies
            simStats statArray[totalMovies+1];
            //    initialize
            for(int sim=0; sim<=totalMovies; ++sim){
                statArray[sim].simVotesI.clear();
                statArray[sim].simVotesJ.clear();
            }
            int numVotes = currentMovie.votes();
            //    votes are from 0 to numVotes-1
            for(int v=0; v<numVotes; ++v){
                int ratingI = currentMovie.score(v);
                int userid = currentMovie.user(v);
                user.setId(userid);
//                printf("i: %d\tMovieID: %d\tv: %d\tUser: %d\n", i, currentMovie.id(), v, userid);
                int numUserVotes = user.votes();
                for(int j=0; j<numUserVotes; ++j){
                    int movieId = user.movie(j);
                    int ratingJ = user.score(j);
                    statArray[movieId].simVotesI.push_back(ratingI);
                    statArray[movieId].simVotesJ.push_back(ratingJ);
                }
            }
            //    Only calculate for half the matrix, if i<=j
            for(int j=1; j<=totalMovies; ++j) if(i<=j){
                int count = statArray[j].simVotesI.size();
                //    calculate correlation if 2 or more similar votes
                float correlation = 0;
                if(count>=2) correlation = pearson(statArray[j].simVotesI, statArray[j].simVotesJ);
                &pearsonOut.write((char*)&correlation, sizeof(float));
                &countOut.write((char*)&count, sizeof(int));
            }
            float percent = (float) 100 * i / (totalMovies);
            if(i%25==0) fprintf(stderr, "Pearson/Count: Approximately %f%% done generating after %d seconds.\n", percent, time(NULL)-genstart);
        }
    }
    
    void showNeighbors(int movieId){
        vector<nStruct> v;
        ifstream pearsonIn(pearsonFile, ios::binary);
        ifstream countIn(countFile, ios::binary);
        for(int neighborid=1; neighborid<=totalMovies; neighborid++) if(neighborid!=movieId){
            int itemNum;
            //    If movieId>neighborid, use the mirror matrix position. Adjust itemNum according to Gaussian sequence.
            if(movieId<=neighborid) itemNum = (movieId-1)*totalMovies + (neighborid - 1) - (movieId-1)*movieId/2;
            else itemNum = (neighborid-1)*totalMovies + (movieId - 1) - (neighborid-1)*neighborid/2;
            nStruct n;
            n.movie = neighborid;
            if(MMAP_PEARSON) n.pearson = pearsonMap[itemNum];
            else n.pearson = get_float(pearsonIn, itemNum);
            if(MMAP_COUNT) n.count = countMap[itemNum];
            else n.count = get_int(countIn, itemNum);
            n.similarity = calcSim(n.pearson, n.count);
            v.push_back(n);
        }
        pearsonIn.close();
        countIn.close();
        int size = (int) v.size();
        partial_sort(v.begin(), v.begin()+min(size, MAX_NEIGHBORS), v.end());
        int i = 1;
        for(vector<nStruct>::iterator iter=v.begin(); iter!=v.begin()+min(size, MAX_NEIGHBORS); iter++){
            int neighborid = (*iter).movie;
            float pearson = (*iter).pearson;
            float similarity = (*iter).similarity;
            int count = (*iter).count;
            printf("Movie: %d\tCount: %d\tPearson: %f Similarity: %f\n", neighborid, count, pearson, similarity);
            i++;
        }
    }
    
    ~KNN(){}

    Movie currentMovie;
    User currentUser;
    bool doPearson, doCount;
    int totalMovies, totalUsers;
    map<uint, double> movieAverages;
    map<uint, double> userAverages;
};

int main(){
    time_t start_time = time(NULL);
    DataBase db;
    db.load();
    Movie currentMovie(&db);
    User user(currentMovie.dataBase());
    int totalMovies = db.totalMovies();
    KNN *knn = new KNN(&db, totalMovies);

    ifstream pearsonIn(pearsonFile, ios::binary);
    ifstream countIn(countFile, ios::binary);
    if(pearsonIn.fail()){
        fprintf(stderr, "Generating pearson file...\n");
        knn->doPearson = true;
    }
    if(countIn.fail()){
        fprintf(stderr, "Generating count file...\n");
        knn->doCount = true;
    }
    if(knn->doPearson || knn->doCount){
        ofstream pearsonOut(pearsonFile, ios::binary);
        ofstream countOut(countFile, ios::binary);
        knn->generatePearson(pearsonOut, countOut);
        pearsonOut.close();
        countOut.close();
        fprintf(stderr, "\n...Done Writing pearson file.\n");
        pearsonIn.open(pearsonFile, ios::binary);
        countIn.open(countFile, ios::binary);
        int dur = time(NULL) - start_time;
        fprintf(stderr, "Creation of .data files took: %d seconds\n", dur);
    }
    pearsonIn.close();
    countIn.close();
    if(MMAP_PEARSON){
        fprintf(stderr, "Attempting to mmap %s...", pearsonFile);
        QFile *pearsonIn = new QFile(pearsonFile);
        if(pearsonIn->open(QFile::ReadOnly | QFile::Unbuffered)){
            pearsonMap = (float*) mmap(0, pearsonIn->size(), PROT_READ, MAP_SHARED, pearsonIn->handle(), (off_t)0);
            if(pearsonMap==(float*) - 1){
                MMAP_PEARSON = false;
            }
        }
        //    Else there was an error
        else MMAP_PEARSON = false;
        if(MMAP_PEARSON) fprintf(stderr, "Success.\n");
        else fprintf(stderr, "Failure.\n");
    }
    if(MMAP_COUNT){
        fprintf(stderr, "Attempting to mmap %s...", countFile);
        QFile *countIn = new QFile(countFile);
        if(countIn->open(QFile::ReadOnly | QFile::Unbuffered)){
            countMap = (int*) mmap(0, countIn->size(), PROT_READ, MAP_SHARED, countIn->handle(), (off_t)0);
            if(countMap==(int*) - 1){
                MMAP_COUNT = false;
            }
        }
        //    Else there was an error
        else MMAP_COUNT = false;
        if(MMAP_COUNT) fprintf(stderr, "Success.\n");
        else fprintf(stderr, "Failure.\n");
    }
    knn->setMovieAverages();
    knn->setUserAverages();
    //knn->showNeighbors(14240);

    Probe probe(&db);
//    probe.setOutput(Probe::SubmitionFile);
    probe.runProbe(knn);
//    probe.runProbe(knn, "qualifying");

    delete knn;
    fprintf(stderr, "\nTotal predictions: %d\nBackups: %d\nErrors: %d\n\n", totpreds, backups, errors);

    int dur = time(NULL) - start_time;
    fprintf(stderr, "kNN took: %d seconds\n", dur);
}