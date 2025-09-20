## AUTHORING: 
* Leah Arfa
* UMBC, CMSC 341 (Data Structures), Fall 2024
* December 12, 2024

## PURPOSE OF SOFTWARE: 
This C++ project simulates a file system using a hash table. This project resolves collisions by probing for the next available empty slot directly within the same array.

## FILES: 
* ```filesys.h```: The header file that contains definitions for all member vars and all function prototypes for the ```File``` and ```FileSys``` classes. It also includes all necessary contansts and enums for using the hash table.
* ```filesys.cpp```: The source file that contains implementations for all functions for the ```FileSys``` class (such as inserting, removing, finding files, and private rehashing helper functions).
* ```driver.cpp```: A driver file that demonstrates the dynamic rehashing function of the ```FileSys``` class.
* ```correctOutputForDriver.cpp```: The exact output expected from the driver.cpp file. It shows the state of hash tables before and after the rehash.
* ```mytest.cpp```: A tester file that verifies the implementation of the ```FileSys``` class's functionalities (ie. file updates, probing method changes, dumping contents, load factor access, and deleted ratio access). It addresses test cases for normal conditions (like non-collisions) and edge conditions (like collisions and rehashes). Each test function is listed in the ```Tester``` class.

## CLASSES: 
* ```FileSys```: A class that stores and manages ```File``` objects within a hash table, using two probing methods: linear and double hashing. It also employs incremental hashing which allows the hash table to dynamically resize itself. This process is automatically triggered when the table's load factor or deleted entry ratio exceeds the specified limit: 0.5 for the load factor and 0.8 for the deleted entry ratio. The class also manages two hash tables simultaneously: ```m_currentTable``` and ```m_oldTable```.
* ```File```: A helper class for the ```FileSys``` data structure, providing basic getters and setters for file attributes - name, disk block, and whether the file is currently in use. A ```File``` object is represented as a single entry in the ```FileSys``` hash table.
* ```Random```: A utility class used to generate varied test data for the ```FileSys``` class, like random strings and random integers to populate the file system.
* ```Tester```: A class that verifies the correctness of the ```FileSys``` class implementation.

## BUILD INSTRUCTIONS: 
1.  **Compilation:** Compile the ```driver.cpp``` file and ```filesys.cpp``` file using the ```g++``` compiler:
    ```
    g++ -g driver.cpp filesys.cpp -o driver
    ```
    
2.  **Execution:** Run the generated executable from the terminal using the command:
    ```
    ./driver
    ```
    
3. **Output Verification:** The program will output the contents of the ```correctOutputForDriver.txt``` file.

## TESTING METHODOLOGY: 
1.  **Compilation:** Compile the ```mytest.cpp``` file and ```filesys.cpp``` file using the ```g++``` compiler:
    ```
    g++ -g mytest.cpp filesys.cpp -o test
    ```
    
2.  **Execution:** Run the generated executable from the terminal using the command:
    ```
    ./test
    ```
    
3. **Output Verification:** The program's output will display either "passed!" or "failed." for each test function in the ```Tester``` class.

## ADDITIONAL INFORMATION:
* The ```dump()``` function provides a way to visually inspect the structure of a hash table. Its output format is ```[index]: [file_name] [disk_block]```.
* The project handles file deletion by marking entries as "deleted" rather than immediately removing them. This "lazy deletion" strategy helps maintain the integrity of probing sequences until the next rehash, where the table is rebuilt and all deleted entries are finally removed.

