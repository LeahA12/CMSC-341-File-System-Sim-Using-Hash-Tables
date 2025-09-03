#include "filesys.h"
#include <math.h>
#include <algorithm>
#include <random>
#include <vector>
using namespace std;

enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};
class Random {
public:
    Random(){}
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = std::normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
        else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_uniReal = std::uniform_real_distribution<double>((double)min,(double)max);
        }
        else { //the case of SHUFFLE to generate every number only once
            m_generator = std::mt19937(m_device());
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = std::mt19937(seedNum);
    }
    void init(int min, int max){
        m_min = min;
        m_max = max;
        m_type = UNIFORMINT;
        m_generator = std::mt19937(10);// 10 is the fixed seed value
        m_unidist = std::uniform_int_distribution<>(min,max);
    }
    void getShuffle(vector<int> & array){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the user program creates the vector param and passes here
        // here we populate the vector using m_min and m_max
        for (int i = m_min; i<=m_max; i++){
            array.push_back(i);
        }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the param array must be of the size (m_max-m_min+1)
        // the user program creates the array and pass it here
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){
            temp.push_back(i);
        }
        std::shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = std::floor(result*100.0)/100.0;
        return result;
    }

    string getRandString(int size){
        // the parameter size specifies the length of string we ask for
        // to use ASCII char the number range in constructor must be set to 97 - 122
        // and the Random type must be UNIFORMINT (it is default in constructor)
        string output = "";
        for (int i=0;i<size;i++){
            output = output + (char)getRandNum();
        }
        return output;
    }
    
    int getMin(){return m_min;}
    int getMax(){return m_max;}
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//integer uniform distribution
    std::uniform_real_distribution<double> m_uniReal;//real uniform distribution

};

unsigned int hashCode(const string str) {
   unsigned int val = 0 ;
   const unsigned int thirtyThree = 33 ;  // magic number from textbook
   for (unsigned int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val ;
}


class Tester {
    
public:
    
    // Test the insertion operation in the hash table. The following presents a sample algorithm to test the normal insertion operation:
    bool testInsertCollidingNorm();
    // Test there are some non-colliding data points in the hash table.
    bool testInsertNonCollidingNorm();
    // Test inserting multiple non-colliding keys.
    bool testFindNonCollidingNorm();
    // Test whether they are inserted in the correct bucket (correct index).
    //bool testInsertColliding();
    // Check whether the data size changes correctly.
    // Test the find operation (getFile(...) function) for an error case, the File object does not exist in the database.
    bool testFindNonExistingErr();
    // Test the find operation (getFile(...) function) with several non-colliding keys.
    //bool testFindNonColliding();
    // Test the find operation (getFile(...) function) with several colliding keys without triggering a rehash. This also tests whether the insertion works correctly with colliding data.
    bool testFindCollidingNorm();
    // Test the remove operation with a few non-colliding keys.
    bool testRemoveNonCollidingNorm();
    // Test the remove operation with a number of colliding keys without triggering a rehash.
    bool testRemoveCollidingNorm();
    // Test the rehashing is triggered after a descent number of data insertion.
    bool testTriggerRehashInsertEdge();
    // Test the rehash completion after triggering rehash due to load factor, i.e. all live data is transferred to the new table and the old table is removed.
    bool testRehashCompletionInsertEdge();
    // Test the rehashing is triggered after a descent number of data removal.
    bool testTriggerRehashRemoveEdge();
    // Test the rehash completion after triggering rehash due to delete ratio, i.e. all live data is transferred to the new table and the old table is removed.
    bool testRehashCompletionRemoveEdge();

};

int main() {
    Tester t;

    cout << "Testing the insertion operation in the hash table for a normal case:";
    if (t.testInsertCollidingNorm()) {
        cout << "\n\tpassed!" << endl;
    }else {
        cout << "\n\tfailed." << endl;
    }

    cout << "Testing there are some non-colliding data points in the hash table for a normal case:";
    if (t.testInsertNonCollidingNorm()) {
        cout << "\n\tpassed!" << endl;
    }else {
        cout << "\n\tfailed." << endl;
    }
    
    cout << "Test inserting multiple non-colliding keys:";
    if (t.testFindNonCollidingNorm()) {
        cout << "\n\tpassed!" << endl;
    }else {
        cout << "\n\tfailed." << endl;
    }

    cout << "Testing the find operation (getFile(...) function) for an error case, the File object does not exist in the database:";
    if (t.testFindNonExistingErr()) {
        cout << "\n\tpassed!" << endl;
    }else {
        cout << "\n\tfailed." << endl;
    }

    cout << "Testing the find operation (getFile(...) function) with several colliding keys without triggering a rehash for a normal case:";
    if (t.testFindCollidingNorm()) {
        cout << "\n\tpassed!" << endl;
    }else {
        cout << "\n\tfailed." << endl;
    }

    cout << "Testing the remove operation with a few non-colliding keys for a normal case:";
    if (t.testRemoveNonCollidingNorm()) {
        cout << "\n\tpassed!" << endl;
    }else {
        cout << "\n\tfailed." << endl;
    }

    cout << "Testing the remove operation with a number of colliding keys without triggering a rehash for a nornal case:";
    if (t.testRemoveCollidingNorm()) {
        cout << "\n\tpassed!" << endl;
    }else {
        cout << "\n\tfailed." << endl;
    }

    cout << "Testing the rehashing is triggered after a descent number of data insertion for an edge case:";
    if (t.testTriggerRehashInsertEdge()) {
        cout << "\n\tpassed!" << endl;
    }else {
        cout << "\n\tfailed." << endl;
    }

    cout << "Testing the rehash completion after triggering rehash due to load factor for an edge case:";
    if (t.testRehashCompletionInsertEdge()) {
        cout << "\n\tpassed!" << endl;
    }else {
        cout << "\n\tfailed." << endl;
    }

    cout << "Testing the rehashing is triggered after a descent number of data removal for an edge case:";
    if (t.testTriggerRehashRemoveEdge()) {
        cout << "\n\tpassed!" << endl;
    }else {
        cout << "\n\tfailed." << endl;
    }

    cout << "Testing the rehash completion after triggering rehash due to delete ratio for an edge case:";
    if (t.testRehashCompletionRemoveEdge()) {
        cout << "\n\tpassed!" << endl;
    }else {
        cout << "\n\tfailed." << endl;
    }

}


/* Tester Functions */

bool Tester::testInsertCollidingNorm() {
    FileSys fs(MINPRIME, hashCode, LINEAR);

    // Insert multiple colliding keys.
    File f1("file1", DISKMIN);
    File f2("file1", DISKMIN + 1); 

    if (fs.insert(f1) 
    and fs.insert(f2)
    and fs.getFile("file1", DISKMIN).getName() == "file1"  //Check whether they are inserted in the correct bucket (correct index).
    and fs.getFile("file1", DISKMIN + 1).getName() == "file1" //Check whether they are inserted in the correct bucket (correct index).
    and fs.lambda() > 0 //Check whether the data size changes correctly.
    ) {
        return true;
    }
    return false;
}

bool Tester::testInsertNonCollidingNorm() {
    FileSys fs(MINPRIME, hashCode, LINEAR);

    // Insert multiple non-colliding keys
    File f1("file1", DISKMIN);
    File f2("file2", DISKMIN + 1);
    File f3("file3", DISKMIN + 2);

    if (fs.insert(f1) and fs.insert(f2) and fs.insert(f3)
    and fs.getFile("file1", DISKMIN).getName() == "file1"  //Check whether they are inserted in the correct bucket (correct index).
    and fs.getFile("file2", DISKMIN + 1).getName() == "file2" //Check whether they are inserted in the correct bucket (correct index).
    and fs.getFile("file3", DISKMIN + 2).getName() == "file3" //Check whether they are inserted in the correct bucket (correct index).
    and fs.lambda() > 0 //Check whether the data size changes correctly.
    ){
        return true;
    }
    return false;
}

bool Tester::testFindNonExistingErr() {
    FileSys fs(MINPRIME, hashCode, LINEAR);

    // Find non-existing file
    File result = fs.getFile("nonexistent", DISKMIN);
    if(result.getName().empty()) {
        return true;
    }
    return false;
}

bool Tester::testFindNonCollidingNorm() {
    FileSys fs(MINPRIME, hashCode, LINEAR);

    // Insert non-colliding keys and find them
    File f1("file1", DISKMIN);
    File f2("file2", DISKMIN + 1);

    fs.insert(f1);
    fs.insert(f2);

    if (fs.getFile("file1", DISKMIN).getName() == "file1"
    and fs.getFile("file2", DISKMIN + 1).getName() == "file2") {
        return true;
    }
    return false;
}

bool Tester::testFindCollidingNorm() {
    FileSys fs(MINPRIME, hashCode, LINEAR);

    // Insert colliding keys and find them
    File f1("file1", DISKMIN);
    File f2("file1", DISKMIN + 1);

    fs.insert(f1);
    fs.insert(f2);

    if (fs.getFile("file1", DISKMIN).getName() == "file1"
    and fs.getFile("file1", DISKMIN + 1).getName() == "file1") {
        return true;
    }
    return false;
}

bool Tester::testRemoveNonCollidingNorm() {
    FileSys fs(MINPRIME, hashCode, LINEAR);

    // Insert and remove non-colliding keys
    File f1("file1", DISKMIN);
    File f2("file2", DISKMIN + 1);

    fs.insert(f1);
    fs.insert(f2);

    if (fs.remove(f1) and fs.remove(f2) and fs.lambda() < 0.5){
        return true;
    }
    return false;
}

bool Tester::testRemoveCollidingNorm() {
    FileSys fs(MINPRIME, hashCode, LINEAR);

    // Insert and remove colliding keys
    File f1("file1", DISKMIN);
    File f2("file1", DISKMIN + 1);

    fs.insert(f1);
    fs.insert(f2);

    if (fs.remove(f1) and fs.remove(f2) and fs.lambda() < 0.5) {
        return true;
    }
    return false;
}

bool Tester::testTriggerRehashInsertEdge() {
    FileSys fs(MINPRIME, hashCode, LINEAR);

    // Inserting more than 50% capacity amt of keys to trigger rehash
    int keysAmt = (0.5 * MINPRIME + 1);
    for (int i = 0; i < keysAmt; i++) {
        fs.insert(File("file" + to_string(i), DISKMIN + i));
    }

    // New load factor should be less than 0.5, showing that rehash was triggered by load factor
    if (fs.lambda() < 0.5) {
        return true;
    } 
    return false;
}

bool Tester::testRehashCompletionInsertEdge() {
    FileSys fs(MINPRIME, hashCode, LINEAR);

    // Inserting more than 50% capacity amt of keys to trigger rehash
    int keysAmt = (0.5 * MINPRIME + 1);
    for (int i = 0; i < keysAmt; i++) {
        fs.insert(File("file" + to_string(i), DISKMIN + i));
    }

    // After rehash, Load Factor should be less than 0.5,
    if (fs.lambda() < 0.5 and
    // Old Table should be empty,
    fs.m_oldTable == nullptr and
    // and Current Table's amount of keys should equal amount of keys inserted.
    fs.m_currentSize == keysAmt){
        return true;
    } 
    return false;
}

bool Tester::testTriggerRehashRemoveEdge() {
    FileSys fs(MINPRIME, hashCode, LINEAR);

    // Inserting random amt of keys, not triggering rehash yet
    int keysAmt = (0.5 * MINPRIME);
    for (int i = 0; i < keysAmt; i++) {
        fs.insert(File("file" + to_string(i), DISKMIN + i));
    }

    // Removing more than 80% of the amt of inserted keys to trigger rehash
    int removeKeysAmt = (0.8 * keysAmt);
    for (int i = 0; i < removeKeysAmt + 1; i++) {
        fs.remove(File("file" + to_string(i), DISKMIN + i));
    }

    // New deleted ratio should be less than 0.8, showing that rehash was triggered by deleted ratio specfically
    if (fs.deletedRatio() < 0.8) {
        return true;
    }
    return false;
}

bool Tester::testRehashCompletionRemoveEdge() {
    FileSys fs(MINPRIME, hashCode, LINEAR);

    // Inserting random amt of keys, not triggering rehash yet
    int keysAmt = (0.5 * MINPRIME);
    for (int i = 0; i < keysAmt; i++) {
        fs.insert(File("file" + to_string(i), DISKMIN + i));
    }

    // Removing more than 80% of the amt of inserted keys to trigger rehash
    int removeKeysAmt = (0.8 * keysAmt);
    for (int i = 0; i < removeKeysAmt + 1; i++) {
        fs.remove(File("file" + to_string(i), DISKMIN + i));
    }

    // After rehash, Deleted Ratio should be less than 0.8,
    if (fs.deletedRatio() < 0.8 and 
    // Old Table should be empty,
    fs.m_oldTable == nullptr and
    // and Current Table's amount of keys should equal amount of keys inserted minus keys deleted.
    fs.m_currentSize == keysAmt - removeKeysAmt){
        return true;
    } 
    return false;
}
