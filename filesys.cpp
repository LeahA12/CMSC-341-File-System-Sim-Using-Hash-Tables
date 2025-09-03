// CMSC 341 - Fall 2024 - Project 4
#include "filesys.h"

FileSys::FileSys(int size, hash_fn hash, prob_t probing = DEFPOLCY):
m_hash(hash),            // Initialized to hash function provided
m_newPolicy(probing),   // Initialized to default (QUADRATIC) as there's no change yet
m_currentTable(nullptr), // Placeholder - will be set after adjusting size
m_currentCap(size),         // Placeholder - will be set after adjusting size
m_currentSize(0),        // Initialized to zero number of entries
m_currNumDeleted(0),     // Initialized to zero number of deleted entries
m_currProbing(probing),  // Initialized to collision handling policy provided
m_oldTable(nullptr),     // Initialized to nullptr as there's no old hash table initially
m_oldCap(0),             // Initialized to zero as there's no old hash table initially
m_oldSize(0),            // Initialized to zero as there's no old hash table initially
m_oldNumDeleted(0),      // Initialized to zero as there's no old hash table initially
m_oldProbing(probing),  // Initialized to default (QUADRATIC) as there's no change yet
m_transferIndex(-1)     // Initialized to -1 as there's no incremental transfer yet
{
    // "If the user passes a size less than MINPRIME, the capacity must be set to MINPRIME."
    if (size < MINPRIME){
        size = MINPRIME;
    }
    // "If the user passes a size larger than MAXPRIME, the capacity must be set to MAXPRIME."
    if (size > MAXPRIME){
        size = MAXPRIME;
    }
    // "If the user passes a non-prime number, the capacity must be set to the smallest prime number greater than user's value."
    if (!isPrime(size)){
        size = findNextPrime(size);
    }
    // Initializing capacity to "a prime number between MINPRIME and MAXPRIME."
    m_currentCap = size;

    // Allocating memory for the current table
    m_currentTable = new File*[m_currentCap](); // Allocating the array of pointer
    for (int i = 0; i < m_currentCap; i++) {
        m_currentTable[i] = nullptr; // Allocating a file object for each pointer
    }
}

FileSys::~FileSys(){
    // Deallocating memory for current table and old table
    if (m_currentTable != nullptr){
        for (int i = 0; i < m_currentCap; i++) {
            delete m_currentTable[i];  // delete each File object
            m_currentTable[i] = nullptr;
        }
        delete[] m_currentTable; // delete the array of pointers
        m_currentTable = nullptr;
    }
    if (m_oldTable != nullptr){
        for (int i = 0; i < m_oldCap; i++) {
            delete m_oldTable[i];  // delete each File object
            m_oldTable[i] = nullptr;
        }
        delete[] m_oldTable; // delete the array of pointers
        m_oldTable = nullptr;
    }
}

void FileSys::changeProbPolicy(prob_t policy){
    m_newPolicy = policy;
}

bool FileSys::insert(File file) {
    // Checking First Constraint = file's block number value should be within valid range
    if (file.getDiskBlock() < DISKMIN or file.getDiskBlock() > DISKMAX){
        return false;
    }
    // Checking Second Constraint = table isn't full
    if (m_currentSize >= m_currentCap){
        return false;
    }
    // Checking Third Constraint = file object isn't a duplicate object
    const File* foundFile = searchForFile(file, m_currentTable, m_currentCap, m_currProbing);
    if (foundFile == nullptr) {
        if (insertFile(file, m_currentTable, m_currentCap, m_currProbing)) {
            m_currentSize++;
            // Checking If Rehashing Is Needed:
            if (lambda() > 0.5) {
                rehash(m_currentTable);
            }
        }
        return true;
    }else{
        return false;
    }
}

/*
Preconditions = load factor > 50% or deleted ratio > 80%.
Function performs the following tasks:
1. Store Current Data in Old Table: 
    Saves the current table's details (size, capacity, deleted entries) to be used for 
    the rehashing process. The current table's entries are deep copied over to Old Table.
2. Empty Current Table Entries: 
    Current hash table is emptied so it can be rehashed. 
3. Update Current Table: 
    New Capacity is the smallest prime greater than four times the current number of 
    occupied buckets (rehash excludes deleted entries). If a policy has changed, then 
    this function will rehash with the new policy.
4. Transfer Live Data By 25% Portions & Reset Transfer Index: 
    Copies live data (non-deleted entries) from the old table to the current table in 
    25% portions, rehashing the entries to fit the current table. Once all data is 
    transferred, reset m_transferIndex.
5. Delete & Deallocate Old Table: 
    "Once all data is transferred to the new table, the old table will be removed,
    and its memory will be deallocated."
*/
void FileSys::rehash(File ** &table) { 
    if (lambda() > 0.5 or deletedRatio() > 0.8){
        // After all data is transferred, Delete & Deallocate Old Table:
        for (int i = 0; i < m_oldCap; i++) {
            delete m_oldTable[i];  // delete each File object
            m_oldTable[i] = nullptr;
        }
        delete[] m_oldTable; // delete the array of pointers
        m_oldTable = nullptr;

        // Store Current Table Data in Old Table
        m_oldCap = m_currentCap;
        m_oldSize = m_currentSize;
        m_oldNumDeleted = m_currNumDeleted;
        m_oldProbing = m_currProbing;
        
        m_oldTable = new File*[m_oldCap];
        for (int i = 0; i < m_oldCap; i++) {
            if (m_currentTable[i] != nullptr) {
                // Perform Deep Copy of Current File Entry
                m_oldTable[i] = new File(*(m_currentTable[i])); // Copy File object
            }else{
                m_oldTable[i] = nullptr;
            }
        }
        
        // Empty Current Table Entries
        if (m_currentTable != nullptr){
            for (int i = 0; i < m_currentCap; i++) {
                delete m_currentTable[i];  // delete each File object
                m_currentTable[i] = nullptr;
            }
            delete[] m_currentTable; // delete the array of pointers
            m_currentTable = nullptr;
        }
        // Empty Current Table Entries & Update Current Table
        m_currentCap = findNextPrime(4 * (m_currentSize - m_currNumDeleted));
        m_currentTable = new File*[m_currentCap]();
        m_currentSize = 0;
        m_currNumDeleted = 0;
        m_currProbing = m_newPolicy;

        // Transfer Live Data By 25% Portions & Reset Transfer Index
        m_transferIndex = 0; // tells us incremental transfer begins
        int transferLimit = floor(m_oldCap / 4);
        bool allTransfered = false; // Boolean flag indicating whether or not all data has been transferred.
        while (!allTransfered){
            for (int i = m_transferIndex; i < min(m_transferIndex + transferLimit, m_oldCap); i++) {
                if (m_oldTable[i] != nullptr and m_oldTable[i]->getUsed()) { // If the entry is not deleted
                    insert(*m_oldTable[i]); // Rehash and insert into the updated current table
                }
            }

            // Update the transfer index after the portion is transferred.
            m_transferIndex += transferLimit;

            // If all data has been transferred, update allTransfered
            if (m_transferIndex >= m_oldCap){
                allTransfered = true;
            }
        }

        // After all data is transferred, Delete & Deallocate Old Table:
        for (int i = 0; i < m_oldCap; i++) {
            delete m_oldTable[i];  // delete each File object
            m_oldTable[i] = nullptr;
        }
        delete[] m_oldTable; // delete the array of pointers
        m_oldTable = nullptr;
        m_transferIndex = -1; // tells us there's no more incremental transfer
    }
}

bool FileSys::remove(File file) {
    // Try to remove file in current table
    if (removeFile(file, m_currentTable, m_currentCap, m_currProbing)) {
        m_currNumDeleted++;

        // Check if need to rehash
        if (deletedRatio() > 0.8) {
            rehash(m_currentTable);
        }
        return true;
    }

    // Try to remove file in old table
    if (m_oldTable != nullptr and m_transferIndex < m_oldSize and removeFile(file, m_oldTable, m_oldCap, m_oldProbing)) {
        m_oldNumDeleted++;

        // Check if need to rehash
        if (deletedRatio() > 0.8) {
            rehash(m_oldTable);
        }
        return true;
    }
    return false;
}

const File FileSys::getFile(string name, int block) const {
    File file(name, block); // File object with name and file block number to search for.

    // 1. Searches For File In Current Table
    const File* foundFile = searchForFile(file, m_currentTable, m_currentCap, m_currProbing);
    if (foundFile != nullptr) {
        return *foundFile;
    }

    // 2. Searches For File In Current Table
    if (m_oldTable != nullptr) {
        foundFile = searchForFile(file, m_oldTable, m_oldCap, m_oldProbing);
        if (foundFile != nullptr) {
            return *foundFile;
        }
    }
    return File();
}

bool FileSys::updateDiskBlock(File file, int block){
    if (updateFile(file, m_currentTable, m_currentCap, m_currProbing, block)) {
        return true;
    }
    if (m_oldTable != nullptr and updateFile(file, m_oldTable, m_oldCap, m_oldProbing, block)) {
        return true;
    }
    return false;
}

float FileSys::lambda() const {
    float loadFactor = 0.0;
    if (m_currentCap > 0) {
        loadFactor = (float)(m_currentSize - m_currNumDeleted) / m_currentCap;
    }
    return loadFactor;
}

float FileSys::deletedRatio() const {
    float ratio = 0.0;
    if (m_currentSize > 0) {
        ratio = (float)m_currNumDeleted / m_currentSize;
    }
    return ratio;
}

/*
This is a helper function that looks for the File object in the specified table.
*/
const File* FileSys::searchForFile(const File & file, File** table, int capacity, prob_t probing) const{
    // Build hash value
    int origIndex = m_hash(file.getName()) % capacity; // The inital index of file to be inserted. Determined by applying the hash function m_hash and then reducing the output of the hash function modulo the table size.
    int currIndex = origIndex; // Altered index of file based on probing policy. Initialzed to original index.
    int collisionAmt = 0; // Amount of collisions at the current index.

    while (table[origIndex] != nullptr) {
        // Find file match
        if (table[origIndex]->getName() == file.getName()
        and table[origIndex]->getDiskBlock() == file.getDiskBlock()) {
            return table[origIndex];
        }

        // Increment the probe index based on the current probing policy
        switch (probing) {
            case LINEAR:
                origIndex = (currIndex + collisionAmt) % capacity;
                break;
            case QUADRATIC:
                origIndex = (currIndex + (collisionAmt * collisionAmt)) % capacity;
                break;
            case DOUBLEHASH:
                int stepSize = m_hash(file.getName()) % (capacity - 1) + 1;
                origIndex = (currIndex + (collisionAmt * stepSize)) % capacity;
                break;
        }
        collisionAmt++;
    }
    return nullptr;    
}

/*
This is a helper function that looks for the File object in the specified table and updates its disk block.
*/
bool FileSys::updateFile(const File & file, File** table, int capacity, prob_t probing, int block){
    // Build hash value
    int origIndex = m_hash(file.getName()) % capacity; // The inital index of file to be inserted. Determined by applying the hash function m_hash and then reducing the output of the hash function modulo the table size.
    int currIndex = origIndex; // Altered index of file based on probing policy. Initialzed to original index.
    int collisionAmt = 0; // Amount of collisions at the current index.

    while (table[origIndex] != nullptr) {
        // Find file match
        if (*(table[origIndex]) == file and !table[origIndex]->getUsed()) {
            table[origIndex]->setDiskBlock(block);
            return true;
        }

        // Increment the probe index based on the current probing policy
        switch (probing) {
            case LINEAR:
                origIndex = (currIndex + collisionAmt) % capacity;
                break;
            case QUADRATIC:
                origIndex = (currIndex + (collisionAmt * collisionAmt)) % capacity;
                break;
            case DOUBLEHASH:
                int stepSize = m_hash(file.getName()) % (capacity - 1) + 1;
                origIndex = (currIndex + (collisionAmt * stepSize)) % capacity;
                break;
        }
        collisionAmt++;
    }
    return false;    
}

/*
This is a helper function that looks for the File object in the specified table and sets m_used to false.
*/
bool FileSys::removeFile(const File & file, File** table, int capacity, prob_t probing){
    // Build hash value
    int origIndex = m_hash(file.getName()) % capacity; // The inital index of file to be inserted. Determined by applying the hash function m_hash and then reducing the output of the hash function modulo the table size.
    int currIndex = origIndex; // Altered index of file based on probing policy. Initialzed to original index.
    int collisionAmt = 0; // Amount of collisions at the current index.

    while (table[origIndex] != nullptr) {
        // Find file match
        if (*(table[origIndex]) == file) {
            table[origIndex]->setUsed(false);
            return true;
        }

        // Increment the probe index based on the current probing policy
        switch (probing) {
            case LINEAR:
                origIndex = (currIndex + collisionAmt) % capacity;
                break;
            case QUADRATIC:
                origIndex = (currIndex + (collisionAmt * collisionAmt)) % capacity;
                break;
            case DOUBLEHASH:
                int stepSize = m_hash(file.getName()) % (capacity - 1) + 1;
                origIndex = (currIndex + (collisionAmt * stepSize)) % capacity;
                break;
        }
        collisionAmt++;
    }
    return false;    
}

/*
This is a helper function that inserts file in the specified table, if found, and sets m_used to true.
*/
bool FileSys::insertFile(const File & file, File** table, int capacity, prob_t probing){
    // Build hash value
    int origIndex = m_hash(file.getName()) % capacity; // The inital index of file to be inserted. Determined by applying the hash function m_hash and then reducing the output of the hash function modulo the table size.
    int currIndex = origIndex; // Altered index of file based on probing policy. Initialzed to original index.
    int collisionAmt = 0; // Amount of collisions at the current index.

    while (table[origIndex] != nullptr and table[origIndex]->getUsed()) {
        // Find file match
        if (*(table[origIndex]) == file) {
            return false;
        }

        // Increment the probe index based on the current probing policy
        switch (probing) {
            case LINEAR:
                origIndex = (currIndex + collisionAmt) % capacity;
                break;
            case QUADRATIC:
                origIndex = (currIndex + (collisionAmt * collisionAmt)) % capacity;
                break;
            case DOUBLEHASH:
                int stepSize = m_hash(file.getName()) % (capacity - 1) + 1;
                origIndex = (currIndex + (collisionAmt * stepSize)) % capacity;
                break;
        }
        collisionAmt++;
    }

    //insert file
    if (table[origIndex] == nullptr) {
        table[origIndex] = new File(file);
    }else {
        *table[origIndex] = file;
    }
    table[origIndex]->setUsed(true);
    return true;    
}

void FileSys::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

bool FileSys::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; i++) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int FileSys::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}
